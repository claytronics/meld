#include "vm/defs.hpp"
#include "db/database.hpp"
#include "vm/state.hpp"
#include "api/api.hpp"

#define SYNC_MPI
#ifdef SYNC_MPI
#include "boost/serialization/string.hpp"
#endif

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

#ifdef SYNC_MPI
    api::world->barrier();

    const int TOKEN = 0;
    const int DONE = 1;

    int source = (api::world->rank() - 1) % api::world->size();
    int dest = (api::world->rank() + 1) % api::world->size();

    if (api::world->rank() == 0) {
        for (map_nodes::const_iterator it(nodes.begin()); it != nodes.end(); ++it) {
            node::node_id id = it->first;

            if (api::on_current_process(id)) {
                cout << "[PID " << api::world->rank() << "] " << *(it->second) << endl;
                cout.flush();
            } else {
                api::world->send(api::get_process_id(id), TOKEN, id);

                string result;
                api::world->recv(api::get_process_id(id), TOKEN, result);

                cout << result << endl;
                cout.flush();
            }
        }
        // Finish printing, signal done
        api::world->isend(dest, DONE);
    } else {
        while(true) {
            boost::mpi::status status = api::world->probe(boost::mpi::any_source,
                                                          boost::mpi::any_tag);

            if (status.tag() == DONE) {
                // Done tag received, terminated
                api::world->irecv(source, DONE);
                api::world->isend(dest, DONE);
                break;
            }

            node::node_id id;

            api::world->recv(0, TOKEN, id);

            assert(api::on_current_process(id));

            ostringstream stream;
            string output;

            stream << "[PID " << api::world->rank() << "] " << *(nodes.at(id));

            output = stream.str();

            api::world->send(0, TOKEN, output);
        }
    }
#endif
}

void
database::print_db_debug(ostream& cout, unsigned int nodeNumber) const
{
   for(map_nodes::const_iterator it(nodes.begin());
      it != nodes.end();
      ++it)
   {
     if ((nodeNumber == it->second->get_translated_id())){
      cout << *(it->second) << endl;
      return;
     }
   }
   cout << "NODE SPECIFIED NOT IN DATABASE" << endl;
}



void
database::dump_db(ostream& cout) const
{
#ifdef SYNC_MPI
    api::world->barrier();

    const int TOKEN = 0;
    const int DONE = 1;

    int source = (api::world->rank() - 1) % api::world->size();
    int dest = (api::world->rank() + 1) % api::world->size();

    if (api::world->rank() == 0) {
        for (map_nodes::const_iterator it(nodes.begin()); it != nodes.end(); ++it) {
            node::node_id id = it->first;

            if (api::on_current_process(id)) {
                it->second->dump(cout);
            } else {
                api::world->send(api::get_process_id(id), TOKEN, id);

                string result;
                api::world->recv(api::get_process_id(id), TOKEN, result);

                cout << result;
                cout.flush();
            }
        }
        // Finish printing, signal done
        api::world->isend(dest, DONE);
    } else {
        while(true) {
            boost::mpi::status status = api::world->probe(boost::mpi::any_source,
                                                          boost::mpi::any_tag);

            if (status.tag() == DONE) {
                // Done tag received, terminated
                api::world->irecv(source, DONE);
                api::world->isend(dest, DONE);
                break;
            }

            node::node_id id;

            api::world->recv(0, TOKEN, id);

            assert(api::on_current_process(id));

            ostringstream stream;
            string output;

            nodes.at(id)->dump(stream);

            output = stream.str();

            api::world->send(0, TOKEN, output);
        }
    }
#endif
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
