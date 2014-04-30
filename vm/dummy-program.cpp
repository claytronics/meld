
#include <fstream>
#include <sys/stat.h>
#include <iostream>
#include <boost/static_assert.hpp>
#include<dlfcn.h>

#include "vm/program.hpp"
#include "db/tuple.hpp"
#include "vm/instr.hpp"
#include "db/database.hpp"
#include "utils/types.hpp"
#include "vm/state.hpp"
#include "vm/reader.hpp"
#include "version.hpp"


using namespace std;
using namespace db;
using namespace vm;
using namespace vm::instr;
using namespace process;
using namespace utils;

namespace vm {

  all* All;		 // global variable that holds pointer to vm
			 // all structure.  Set by process/machine.cpp
			 // in constructor.
  program* theProgram;
#if 0
  // most integers in the byte-code have 4 bytes
  BOOST_STATIC_ASSERT(sizeof(uint_val) == 4);

  program::program(const string& _filename):
    filename(_filename),
    init(NULL)
  {
    code_reader read(filename);

    // read magic
    uint32_t magic1, magic2;
    read.read_type<uint32_t>(&magic1);
    read.read_type<uint32_t>(&magic2);
    if(magic1 != MAGIC1 || magic2 != MAGIC2)
      throw load_file_error(filename, "not a meld byte code file");

    // read version
    read.read_type<uint32_t>(&major_version);
    read.read_type<uint32_t>(&minor_version);
    if(!VERSION_AT_LEAST(0, 5))
      throw load_file_error(filename, string("unsupported byte code version"));

    if(VERSION_AT_LEAST(0, 11))
      throw load_file_error(filename, string("unsupported byte code version"));

    // read number of predicates
    byte num_preds;
    read.read_type<byte>(&num_preds);
   
    const size_t num_predicates = (size_t)num_preds;
   
    predicates.resize(num_predicates);
    code_size.resize(num_predicates);
    code.resize(num_predicates);

    // skip nodes
    uint_val num_nodes;
    read.read_type<uint_val>(&num_nodes);

    read.seek(num_nodes * database::node_size);

    if(VERSION_AT_LEAST(0, 10)) {
      // read number of types
      byte ntypes;
      read.read_type<byte>(&ntypes);
      types.resize((size_t)ntypes);

      for(size_t i(0); i < num_types(); ++i) {
	types[i] = read_type_from_reader(read);
      }
    }

    // read imported/exported predicates
    if(VERSION_AT_LEAST(0, 9)) {
      uint32_t number_imported_predicates;

      read.read_type<uint32_t>(&number_imported_predicates);

      for(uint32_t i(0); i < number_imported_predicates; ++i) {
	uint32_t size;
	read.read_type<uint32_t>(&size);

	char buf_imp[size + 1];
	read.read_any(buf_imp, size);
	buf_imp[size] = '\0';

	read.read_type<uint32_t>(&size);
	char buf_as[size + 1];
	read.read_any(buf_as, size);
	buf_as[size] = '\0';

	read.read_type<uint32_t>(&size);
	char buf_file[size + 1];
	read.read_any(buf_file, size);
	buf_file[size] = '\0';

	cout << "import " << buf_imp << " as " << buf_as << " from " << buf_file << endl;

	imported_predicates.push_back(new import(buf_imp, buf_as, buf_file));
      }
      assert(imported_predicates.size() == number_imported_predicates);

      uint32_t number_exported_predicates;

      read.read_type<uint32_t>(&number_exported_predicates);

      for(uint32_t i(0); i < number_exported_predicates; ++i) {
	uint32_t str_size;
	read.read_type<uint32_t>(&str_size);
	char buf[str_size + 1];
	read.read_any(buf, str_size);
	buf[str_size] = '\0';
	exported_predicates.push_back(string(buf));
      }
      assert(exported_predicates.size() == number_exported_predicates);
    }

    // get number of args needed
    byte n_args;

    read.read_type<byte>(&n_args);
    num_args = (size_t)n_args;

    // get rule information
    uint32_t n_rules;

    read.read_type<uint32_t>(&n_rules);

    number_rules = n_rules;

    for(size_t i(0); i < n_rules; ++i) {
      // read rule string length
      uint32_t rule_len;

      read.read_type<uint32_t>(&rule_len);

      assert(rule_len > 0);

      char str[rule_len + 1];

      read.read_any(str, rule_len);

      str[rule_len] = '\0';

      rules.push_back(new rule((rule_id)i, string(str)));
    }

    // read string constants
    uint32_t num_strings;
    read.read_type<uint32_t>(&num_strings);
	
    default_strings.reserve(num_strings);
	
    for(uint32_t i(0); i < num_strings; ++i) {
      uint32_t length;
		
      read.read_type<uint32_t>(&length);
		
      char str[length + 1];
      read.read_any(str, length);
      str[length] = '\0';
      default_strings.push_back(runtime::rstring::make_default_string(str));
    }
	
    // read constants code
    uint32_t num_constants;
    read.read_type<uint32_t>(&num_constants);
	
    // read constant types
    const_types.resize(num_constants);
	
    for(uint_val i(0); i < num_constants; ++i) {
      const_types[i] = read_type_id_from_reader(read, types);
    }
	
    // read constants code
    read.read_type<code_size_t>(&const_code_size);
	
    const_code = new byte_code_el[const_code_size];
	
    read.read_any(const_code, const_code_size);

    MAX_STRAT_LEVEL = 0;

    if(VERSION_AT_LEAST(0, 6)) {
      // get function code
      uint32_t n_functions;

      read.read_type<uint32_t>(&n_functions);

      functions.resize(n_functions);

      for(uint32_t i(0); i < n_functions; ++i) {
	code_size_t fun_size;

	read.read_type<code_size_t>(&fun_size);
	byte_code fun_code(new byte_code_el[fun_size]);
	read.read_any(fun_code, fun_size);

	functions[i] = new vm::function(fun_code, fun_size);
      }

      //init functions defined in external namespace
      init_external_functions();

      if(major_version > 0 || minor_version >= 7) {
	// get external functions definitions
	uint32_t n_externs;

	read.read_type<uint32_t>(&n_externs);

	for(uint32_t i(0); i < n_externs; ++i) {
	  uint32_t extern_id;

	  read.read_type<uint32_t>(&extern_id);
	  char extern_name[256];

	  read.read_any(extern_name, sizeof(extern_name));

	  char skip_filename[1024];

	  read.read_any(skip_filename, sizeof(skip_filename));

	  ptr_val skip_ptr;

	  read.read_type<ptr_val>(&skip_ptr);

	  //dlopen call
	  //dlsym call
	  skip_ptr = get_function_pointer(skip_filename,extern_name);
	  uint32_t num_args;

	  read.read_type<uint32_t>(&num_args);

	  type *ret_type = read_type_id_from_reader(read, types);

	  cout << "Id " << extern_id << " " << extern_name << " ";
	  cout <<"Num_args "<<num_args<<endl;

	  type *arg_type[num_args];
	  if(num_args){
	    for(uint32_t j(0); j != num_args; ++j) {
	      arg_type[j] = read_type_id_from_reader(read, types);
	      cout << arg_type[j]->string() << " ";
	    }

	    add_external_function((external_function_ptr)skip_ptr,num_args,ret_type,arg_type);             
	  }else
	    add_external_function((external_function_ptr)skip_ptr,0,ret_type,NULL);
	  cout << endl;
	}
      }
    }

    // read predicate information
   
    for(size_t i(0); i < num_predicates; ++i) {
      code_size_t size;

      predicates[i] = predicate::make_predicate_from_reader(read, &size, (predicate_id)i, major_version, minor_version, types);
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
	
    read.read_type<byte>(&global_info);

    initial_priority.int_priority = 0;
    initial_priority.float_priority = 0.0;
    priority_static = false;

    is_data_file = false;
	
    switch(global_info) {
    case 0x01: { // priority by predicate
      cerr << "Not supported anymore" << endl;
      assert(false);
    }
      break;
    case 0x02: { // normal priority
      byte type(0x0);
      byte asc_desc;

      read.read_type<byte>(&type);
      priority_type = FIELD_FLOAT;
      assert(type == 0x01);

      read.read_type<byte>(&asc_desc);
      if(asc_desc & 0x01)
	priority_order = PRIORITY_ASC;
      else
	priority_order = PRIORITY_DESC;
      priority_static = (asc_desc & 0x02) ? true : false;

      read.read_type<float_val>(&initial_priority.float_priority);
    }
      break;
    case 0x03: { // data file
      is_data_file = true;
    }
      break;
    default:
      priority_type = FIELD_FLOAT; 
      priority_order = PRIORITY_DESC;
      break;
    }
   
    // read predicate code
    for(size_t i(0); i < num_predicates; ++i) {
      const size_t size = code_size[i];
      code[i] = new byte_code_el[size];
      
      read.read_any(code[i], size);
    }

    // read rules code
    uint32_t num_rules_code;
    read.read_type<uint32_t>(&num_rules_code);

    assert(num_rules_code == number_rules);

    for(size_t i(0); i < num_rules_code; ++i) {
      code_size_t code_size;
      byte_code code;

      read.read_type<code_size_t>(&code_size);

      code = new byte_code_el[code_size];

      read.read_any(code, code_size);

      rules[i]->set_bytecode(code_size, code);

      byte is_persistent(0x0);

      read.read_type<byte>(&is_persistent);

      if(is_persistent == 0x1)
	rules[i]->set_as_persistent();

      uint32_t num_preds;

      read.read_type<uint32_t>(&num_preds);

      assert(num_preds < 10);

      for(size_t j(0); j < num_preds; ++j) {
	predicate_id id;
	read.read_type<predicate_id>(&id);
	predicate *pred(predicates[id]);

	pred->affected_rules.push_back(i);
	rules[i]->add_predicate(pred);
      }
    }

    data_rule = NULL;
  }
#endif

  predicate*
  program::get_predicate(const predicate_id& id) const
  {
  }

  predicate*
  program::get_predicate_by_name(const string& name) const
  {
  }

  tuple*
  program::new_tuple(const predicate_id& id) const
  {
  }

  ptr_val 
  program::get_function_pointer(char *lib_path,char* func_name)
  {
  }

  void 
  program::add_external_function(external_function_ptr ptr,size_t num_args,type *ret,type **arg)
  {
  }
}
