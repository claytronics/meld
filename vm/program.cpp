
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <boost/static_assert.hpp>

#include "vm/program.hpp"
#include "db/tuple.hpp"
#include "vm/instr.hpp"
#include "db/database.hpp"
#include "utils/types.hpp"
#include "vm/state.hpp"
#include "version.hpp"
#ifdef USE_UI
#include "ui/macros.hpp"
#endif

using namespace std;
using namespace db;
using namespace vm;
using namespace vm::instr;
using namespace process;
using namespace utils;

namespace vm {

static const size_t PREDICATE_DESCRIPTOR_SIZE = sizeof(code_size_t) +
                                                PRED_DESCRIPTOR_BASE_SIZE +
                                                PRED_ARGS_MAX +
                                                PRED_NAME_SIZE_MAX +
                                                PRED_AGG_INFO_MAX;
strat_level program::MAX_STRAT_LEVEL(0);

#define READ_CODE(TO, SIZE) do { \
	fp.read((char *)(TO), SIZE);	\
	position += (SIZE);				\
} while(false)
#define SEEK_CODE(SIZE) do { \
	fp.seekg(SIZE, ios_base::cur);	\
	position += (SIZE);	\
} while(false)

// most integers in the byte-code have 4 bytes
BOOST_STATIC_ASSERT(sizeof(uint_val) == 4);

/* interprets predicates, argus, types, interprets the code */
program::program(const string& _filename):
   filename(_filename),
   init(NULL), priority_pred(NULL)
{
	size_t position(0);
   
   ifstream fp(filename.c_str(), ios::in | ios::binary);
   
   if(!fp.is_open())
      throw load_file_error(filename, "unable to open file");

   // read magic
   // determines meld file type
   uint32_t magic1, magic2;
   READ_CODE(&magic1, sizeof(magic1));
   READ_CODE(&magic2, sizeof(magic2));
   if(magic1 != MAGIC1 || magic2 != MAGIC2)
      throw load_file_error(filename, "not a meld byte code file");

   // read version
   // version of byte code
   uint32_t major_version, minor_version;
   READ_CODE(&major_version, sizeof(uint32_t));
   READ_CODE(&minor_version, sizeof(uint32_t));
   if(major_version != MAJOR_VERSION || minor_version != MINOR_VERSION)
      throw load_file_error(filename, string("invalid byte-code file, required ") + to_string(MAJOR_VERSION) + "." + to_string(MINOR_VERSION));

   // read number of predicates
   byte buf[PREDICATE_DESCRIPTOR_SIZE];
   
	READ_CODE(buf, sizeof(byte));
   
   const size_t num_predicates = (size_t)buf[0];
   
   predicates.resize(num_predicates);
   code_size.resize(num_predicates);
   code.resize(num_predicates);

   // skip nodes
   // node: a node in a graph
   // node: local computation, send facts
   uint_val num_nodes;
	READ_CODE(&num_nodes, sizeof(uint_val));

	SEEK_CODE(num_nodes * database::node_size);

	// get number of args needed
	byte n_args;

	READ_CODE(&n_args, sizeof(byte));
	num_args = (size_t)n_args;

   // get rule information
   uint_val n_rules;

   READ_CODE(&n_rules, sizeof(uint_val));

   number_rules = n_rules;

   for(size_t i(0); i < n_rules; ++i) {
      // read rule string length
      uint_val rule_len;

      READ_CODE(&rule_len, sizeof(uint_val));

      assert(rule_len > 0);

      char str[rule_len + 1];

      READ_CODE(str, sizeof(char) * rule_len);

      str[rule_len] = '\0';

      rules.push_back(new rule((rule_id)i, string(str)));
   }

	// read string constants
	int_val num_strings;
	READ_CODE(&num_strings, sizeof(int_val));
	
	default_strings.reserve(num_strings);
	
	for(int i(0); i < num_strings; ++i) {
		int_val length;
		
		READ_CODE(&length, sizeof(int_val));
		
		char str[length + 1];
		READ_CODE(str, sizeof(char) * length);
		str[length] = '\0';
		default_strings.push_back(runtime::rstring::make_default_string(str));
	}
	
	// read constants code
	uint_val num_constants;
	READ_CODE(&num_constants, sizeof(uint_val));
	
	// read constant types
	const_types.resize(num_constants);
	
	for(uint_val i(0); i < num_constants; ++i) {
		byte b;
		READ_CODE(&b, sizeof(byte));
		const_types[i] = (field_type)b;
	}
	
	// read constants code
	READ_CODE(&const_code_size, sizeof(code_size_t));
	
	const_code = new byte_code_el[const_code_size];
	
	READ_CODE(const_code, const_code_size);

   // read predicate information
   for(size_t i(0); i < num_predicates; ++i) {
      code_size_t size;

		READ_CODE(buf, PREDICATE_DESCRIPTOR_SIZE);
      
      predicates[i] = predicate::make_predicate_from_buf((unsigned char*)buf, &size, (predicate_id)i);
      code_size[i] = size;

      MAX_STRAT_LEVEL = max(predicates[i]->get_strat_level() + 1, MAX_STRAT_LEVEL);
		
      if(predicates[i]->is_route_pred())
         route_predicates.push_back(predicates[i]);
   }
   
   safe = true;
   for(size_t i(0); i < num_predicates; ++i) {
      predicates[i]->cache_info(this);
      if(predicates[i]->is_aggregate() && predicates[i]->is_unsafe_agg()) {
         safe = false;
		}
   }

	// get global priority information
	byte global_info;
	
	READ_CODE(&global_info, sizeof(byte));

   initial_priority.int_priority = 0;
   initial_priority.float_priority = 0.0;
	
   switch(global_info) {
      case 0x01: {
         predicate_id pred;
         byte asc_desc;
         field_num priority_argument;
         
         READ_CODE(&pred, sizeof(predicate_id));
         READ_CODE(&priority_argument, sizeof(field_num));
         READ_CODE(&asc_desc, sizeof(byte));
         
         priority_order = (asc_desc ? PRIORITY_ASC : PRIORITY_DESC);
         priority_pred = predicates[pred];
         priority_argument -= 2;
         priority_pred->set_global_priority(priority_order, priority_argument);
         priority_strat_level = priority_pred->get_strat_level();
         priority_type = priority_pred->get_field_type(get_priority_argument());
      }
      break;
      case 0x02: {
         byte type(0x0);
         byte asc_desc;

         READ_CODE(&type, sizeof(byte));
         switch(type) {
            case 0x01: priority_type = FIELD_FLOAT; break;
            case 0x02: priority_type = FIELD_INT; break;
            default: assert(false);
         }

         READ_CODE(&asc_desc, sizeof(byte));
         priority_order = (asc_desc ? PRIORITY_ASC : PRIORITY_DESC);

         if(priority_type == FIELD_FLOAT)
            READ_CODE(&initial_priority.float_priority, sizeof(float_val));
         else
            READ_CODE(&initial_priority.int_priority, sizeof(int_val));
      }
      break;
      default:
      priority_type = FIELD_FLOAT; 
      break;
   }
   
   // read predicate code
   for(size_t i(0); i < num_predicates; ++i) {
      const size_t size = code_size[i];
      code[i] = new byte_code_el[size];
      
		READ_CODE(code[i], size);
   }

   // read rules code
	uint_val num_rules_code;
	READ_CODE(&num_rules_code, sizeof(uint_val));

   assert(num_rules_code == number_rules);

   for(size_t i(0); i < num_rules_code; ++i) {
      code_size_t code_size;
      byte_code code;

      READ_CODE(&code_size, sizeof(code_size_t));

      code = new byte_code_el[code_size];

      READ_CODE(code, code_size);

      rules[i]->set_bytecode(code_size, code);

      byte is_persistent(0x0);

      READ_CODE(&is_persistent, sizeof(byte));

      if(is_persistent == 0x1)
         rules[i]->set_as_persistent();

      uint_val num_preds;

      READ_CODE(&num_preds, sizeof(uint_val));

      assert(num_preds < 10);

      for(size_t j(0); j < num_preds; ++j) {
         predicate_id id;
         READ_CODE(&id, sizeof(predicate_id));
         predicate *pred(predicates[id]);

         pred->affected_rules.push_back(i);
         rules[i]->add_predicate(pred);
      }
   }
}

program::~program(void)
{
   for(size_t i(0); i < num_predicates(); ++i) {
      delete predicates[i];
      delete []code[i];
   }
	for(size_t i(0); i < num_rules(); ++i) {
		delete rules[i];
	}
	delete []const_code;
   MAX_STRAT_LEVEL = 0;
}

predicate*
program::get_predicate(const predicate_id& id) const
{
   if((size_t)id >= num_predicates()) {
      return NULL;
   }
   
   return predicates[id];
}

predicate*
program::get_route_predicate(const size_t& i) const
{
   assert(i < num_route_predicates());
   
   return route_predicates[i];
}

void
program::print_predicate_code(ostream& out, predicate* p) const
{
   out << "PROCESS " << p->get_name()
      << " (" << code_size[p->get_id()] << "):" << endl;
   instrs_print(code[p->get_id()], code_size[p->get_id()], 0, this, out);
   out << "END PROCESS;" << endl;
}

void
program::print_bytecode(ostream& out) const
{
	out << "CONST CODE" << endl;
	
	instrs_print(const_code, const_code_size, 0, this, out);
	
   for(size_t i = 0; i < num_predicates(); ++i) {
      predicate_id id = (predicate_id)i;
      
      if(i != 0)
         out << endl;
         
      print_predicate_code(out, get_predicate(id));
   }

   out << "RULES CODE" << endl;

   for(size_t i(0); i < number_rules; ++i) {
      out << endl;
      out << "RULE " << i << endl;
		rules[i]->print(out, this);
   }
}

void
program::print_bytecode_by_predicate(ostream& out, const string& name) const
{
	predicate *p(get_predicate_by_name(name));
	
	if(p == NULL) {
		cerr << "Predicate " << name << " not found." << endl;
		return;
	}
   print_predicate_code(out, get_predicate_by_name(name));
}

void
program::print_predicates(ostream& cout) const
{
   if(is_safe())
      cout << ">> Safe program" << endl;
   else
      cout << ">> Unsafe program" << endl;
   for(size_t i(0); i < num_predicates(); ++i) {
      cout << predicates[i] << " " << *predicates[i] << endl;
   }
}

#ifdef USE_UI
using namespace json_spirit;

Value
program::dump_json(void) const
{
	Array preds_json;

	for(size_t i(0); i < num_predicates(); ++i) {
		Object obj;
		predicate *pred(get_predicate((predicate_id)i));

		UI_ADD_FIELD(obj, "name", pred->get_name());

		Array field_types;

		for(size_t j(0); j < pred->num_fields(); ++j) {
			switch(pred->get_field_type(j)) {
				case FIELD_INT:
					UI_ADD_ELEM(field_types, "int");
					break;
				case FIELD_FLOAT:
					UI_ADD_ELEM(field_types, "float");
					break;
				case FIELD_NODE:
					UI_ADD_ELEM(field_types, "node");
					break;
				case FIELD_STRING:
					UI_ADD_ELEM(field_types, "string");
					break;
				case FIELD_LIST_INT:
					UI_ADD_ELEM(field_types, "list int");
					break;
				default:
					throw type_error("Unrecognized field type " + field_type_string(pred->get_field_type(j)) + " (program::dump_json)");
			}
		}
		UI_ADD_FIELD(obj, "fields", field_types);

		UI_ADD_FIELD(obj, "route",
				pred->is_route_pred() ? UI_YES : UI_NIL);
		UI_ADD_FIELD(obj, "reverse_route",
				pred->is_reverse_route_pred() ? UI_YES : UI_NIL);
		UI_ADD_FIELD(obj, "linear",
			pred->is_linear_pred() ? UI_YES : UI_NIL);

		UI_ADD_ELEM(preds_json, obj);
	}

	return preds_json;
}
#endif

predicate*
program::get_predicate_by_name(const string& name) const
{
   for(size_t i(0); i < num_predicates(); ++i) {
      predicate *pred(get_predicate((predicate_id)i));
      
      if(pred->get_name() == name)
         return pred;
   }
   
   return NULL;
}

predicate*
program::get_init_predicate(void) const
{
   if(init == NULL) {
      init = get_predicate(INIT_PREDICATE_ID);
      if(init->get_name() != "_init") {
         cerr << "_init program should be predicate #" << (int)INIT_PREDICATE_ID << endl;
         init = get_predicate_by_name("_init");
      }
      assert(init->get_name() == "_init");
   }

   assert(init != NULL);
      
   return init;
}

predicate*
program::get_edge_predicate(void) const
{
   return get_predicate_by_name("edge");
}

tuple*
program::new_tuple(const predicate_id& id) const
{
   return new tuple(get_predicate(id));
}

}
