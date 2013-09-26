
#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <map>
#include <fstream>
#include <ostream>
#include <tr1/unordered_map>
#include <stdexcept>
#include <boost/function.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpi.hpp>

#include "conf.hpp"
#include "db/node.hpp"
#include "utils/spinlock.hpp"
#include "vm/program.hpp"

// Store all nodes in VM
namespace db
{

  class database
  {
  public:
    // used to show ids in source code, not in VM
    typedef std::tr1::unordered_map<node::node_id, node::node_id,
				    std::tr1::hash<node::node_id>,
				    std::equal_to<node::node_id>,
				    mem::allocator<
				      std::pair<const node::node_id,
						node::node_id> > > map_translate;
  // node id->node data structure
  typedef std::map<node::node_id, node*,
		   std::less<node::node_id>,
		   mem::allocator< std::pair<const node::node_id,
					     node*> > > map_nodes;
  // used to create a new node
  typedef boost::function3<node*, node::node_id, node::node_id,
			   vm::all *> create_node_fn;

  private:
    vm::all *all;		// used so nodes can access stuff.  Should not be here
    create_node_fn create_fn;	// used to create nodes.

    map_nodes nodes;		// nodeid->node data structure

    // used for debugging to get src code name
    map_translate translation;
    map_translate reverse_translation;

    // used to keep node ids uniq and track max number of nodes
    node::node_id original_max_node_id;
    node::node_id max_node_id;
    node::node_id max_translated_id;

    // used when creating a new node
    utils::spinlock mtx;

  public:
    // node id is 32 bits.  Should be a parameter in the byte code to
    // determine what nodeid is for this program.
    BOOST_STATIC_ASSERT(sizeof(node::node_id) == 4);

    // node id in byte code is twice size of node_id since it includes translation of node id
    static const size_t node_size = sizeof(node::node_id) * 2;

    // number of nodes in system at this time
    size_t nodes_total;

    // used to iterate through all the nodes starting at 'id' specified in get_node_iterator
    map_nodes::const_iterator nodes_begin(void) const { return nodes.begin(); }
    map_nodes::const_iterator nodes_end(void) const { return nodes.end(); }
    map_nodes::iterator get_node_iterator(const node::node_id id) { return nodes.find(id); }

    size_t num_nodes(void) const { return nodes.size(); }
    // max id currently
    node::node_id max_id(void) const { return max_node_id; }
    // max id in byte code
    node::node_id static_max_id(void) const { return original_max_node_id; }

    // get the node structure from an id
    node* find_node(const node::node_id) const;
    // create nodes
    node* create_node(void);
    node* create_node_id(const node::node_id);

    node::node_id translate_real_to_fake_id(const node::node_id real_id);
    node::node_id translate_fake_to_real_id(const node::node_id fake_id);

    void print_db(std::ostream&) const;
    void print_db_debug(std::ostream&, unsigned int nodeNumber);
    void print_entire_db_debug(std::ostream&cout);
    void dump_db(std::ostream&) const;

    void print(std::ostream&) const;

    explicit database(const std::string&, create_node_fn, vm::all *);

    ~database(void);
  };

  std::ostream& operator<<(std::ostream&, const database&);

  class database_error : public std::runtime_error {
  public:
    explicit database_error(const std::string& msg) :
      std::runtime_error(msg)
    {}
  };

}

#endif
