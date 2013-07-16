#include "vm/defs.hpp"
#include "db/database.hpp"
#include "vm/state.hpp"
#include "api/api.hpp"

using namespace db;
using namespace std;
using namespace vm;
using namespace process;
using namespace utils;
using namespace api;
namespace mpi = boost::mpi;

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

      // Implementation specific, create node
      node *node(create_fn(fake_id, real_id, all));

      translation[fake_id] = real_id;
      reverse_translation[real_id] = fake_id;

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
    /* MPI NOTE: find_node only finds the node in the current database.
     * Since the current implementation of the MPI has each process
     * containing all of the nodes, this is a non-issue.  However if the
     * nodes are partitioned throughout the process, then find node needs to
     * take into consideration of the id translation
     * -- Xing
     */
   map_nodes::const_iterator it(nodes.find(id));

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

   max_node_id = id;
   max_translated_id = id;

   node *ret(create_fn(max_node_id, max_translated_id, all));
   translation[max_node_id] = max_translated_id;
   reverse_translation[max_translated_id] = max_node_id;
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
   reverse_translation[max_translated_id] = max_node_id;
   nodes[max_node_id] = ret;

   return ret;
}

node::node_id
database::translate_real_to_fake_id(const node::node_id real_id) {
    return reverse_translation[real_id];
}

node::node_id
database::translate_fake_to_real_id(const node::node_id fake_id) {
    return translation[fake_id];
}

void
database::print_db(ostream& cout) const
{
    api::printDB(cout, nodes);
}


void
database::print_db_debug(ostream& cout, const node::node_id real_id)
{
    cout << *(nodes.at(translate_real_to_fake_id(real_id))) << endl;
}

void
database::dump_db(ostream& cout) const
{
    api::dumpDB(cout, nodes);
}


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
