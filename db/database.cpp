#include "vm/defs.hpp"
#include "db/database.hpp"
#include "vm/state.hpp"

using namespace db;
using namespace std;
using namespace vm;
using namespace process;
using namespace utils;

namespace db
{

database::database(const string& filename, create_node_fn _create_fn, vm::all *_all):
   all(_all), create_fn(_create_fn), nodes_total(0)
{
   int_val num_nodes;
   node::node_id fake_id;
   node::node_id real_id;

   ifstream fp(filename.c_str(), ios::in | ios::binary);

   fp.seekg(vm::MAGIC_SIZE, ios_base::cur); // skip magic
   fp.seekg(2*sizeof(uint32_t), ios_base::cur); // skip version

   fp.seekg(sizeof(byte), ios_base::cur); // skip number of definitions

   fp.read((char*)&num_nodes, sizeof(int_val));

   nodes_total = num_nodes;

   max_node_id = 0;
   max_translated_id = 0;

   // Create all of the nodes
   for(size_t i(0); i < nodes_total; ++i) {
      fp.read((char*)&fake_id, sizeof(node::node_id));
      fp.read((char*)&real_id, sizeof(node::node_id));

      cerr << "Fake_id: " << fake_id << endl;
      cerr << "Real_id: " << real_id << endl;

      // Implementation specific, create node
      node *node(create_fn(fake_id, real_id, all));

      translation[fake_id] = real_id;
      nodes[fake_id] = node;

      if(fake_id > max_node_id)
         max_node_id = fake_id;
      if(real_id > max_translated_id)
         max_translated_id = real_id;
   }

   original_max_node_id = max_node_id;
}
}

database::~database(void)
{
   for(map_nodes::iterator it(nodes.begin()); it != nodes.end(); ++it)
      delete it->second;
}

node*
database::find_node(const node::node_id id) const
{
   map_nodes::const_iterator it(nodes.begin());

   if(it == nodes.end()) {
      cerr << "Could not find node with id " << id << endl;
      abort();
   }
   
   return it->second;
}

node*
database::create_node_id(const db::node::node_id id)
{
   utils::spinlock::scoped_lock l(mtx);
   if(max_node_id > 0) {
      assert(max_node_id < id);
      assert(max_translated_id < id);
   }

   max_node_id = id;
   max_translated_id = id;

   node *ret(create_fn(max_node_id, max_translated_id, all));
   
   translation[max_node_id] = max_translated_id;
   nodes[max_node_id] = ret;

   return ret;
}

node*
database::create_node(void)
{
   utils::spinlock::scoped_lock l(mtx);

	if(nodes.empty()) {
		max_node_id = 0;
		max_translated_id = 0;
	} else {
   	++max_node_id;
   	++max_translated_id;
	}
	
   node *ret(create_fn(max_node_id, max_translated_id, all));
   
   translation[max_node_id] = max_translated_id;
   nodes[max_node_id] = ret;

   return ret;
}

void
database::print_db(ostream& cout) const
{
   for(map_nodes::const_iterator it(nodes.begin());
      it != nodes.end();
      ++it)
   {
      cout << *(it->second) << endl;
   }
}

void
database::dump_db(ostream& cout) const
{
   for(map_nodes::const_iterator it(nodes.begin());
      it != nodes.end();
      ++it)
   {
      it->second->dump(cout);
   }
}

#ifdef USE_UI
using namespace json_spirit;

Value
database::dump_json(void) const
{
	Object root;
	
	UI_ADD_FIELD(root, "num_nodes", (int)num_nodes());
	
	Array nodes_data;
	
	for(map_nodes::const_iterator it(nodes.begin()), end(nodes.end()); it != end; it++) {
		Object node_data;
		const node *n(it->second);
		
		UI_ADD_FIELD(node_data, "id", (int)n->get_id());
		UI_ADD_FIELD(node_data, "translated_id", (int)n->get_translated_id());

		UI_ADD_ELEM(nodes_data, node_data);
	}
	
	UI_ADD_FIELD(root, "nodes", nodes_data);
	
	return root;
}
#endif

void
database::print(ostream& cout) const
{
   cout << "{";
   for(map_nodes::const_iterator it(nodes.begin());
      it != nodes.end();
      ++it)
   {
      if(it != nodes.begin())
         cout << ", ";
      cout << it->first;
   }
   cout << "}";
}

ostream& operator<<(ostream& cout, const database& db)
{
   db.print(cout);
   return cout;
}
