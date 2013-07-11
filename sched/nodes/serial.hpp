#ifndef SCHED_NODES_SERIAL_HPP
#define SCHED_NODES_SERIAL_HPP

#include "mem/base.hpp"
#include "db/tuple.hpp"
#include "utils/spinlock.hpp"
#include "sched/base.hpp"
#include "queue/intrusive.hpp"
#include "sched/nodes/in_queue.hpp"
#include "queue/bounded_pqueue.hpp"


// Node type for sequential scheduler
namespace sched
{

enum face_t {
   INVALID_FACE = -1,
   BOTTOM = 0,
   NORTH = 1,
   EAST = 2,
   WEST = 3,
   SOUTH = 4,
   TOP = 5
};

inline face_t& operator++(face_t &f)
{
   f = static_cast<face_t>(f + 1);
   return f;
}

inline face_t operator++(face_t& f, int) {
   ++f;
   return f;
}

class serial_node: public in_queue_node
{
/*Making it compatible with simulator*/
public:
   DECLARE_DOUBLE_QUEUE_NODE(serial_node);
   typedef queue::unsafe_bounded_pqueue<db::simple_tuple*>::type queue_type;
   queue_type queue;


private:
   vm::node_val top;
   vm::node_val bottom;
   vm::node_val east;
   vm::node_val west;
   vm::node_val north;
   vm::node_val south;

   bool instantiated_flag;
   size_t neighbor_count;



public:

typedef queue_type::const_iterator queue_iterator;

inline queue_iterator begin(void) const { return queue.begin(); }
inline queue_iterator end(void) const { return queue.end(); }

   inline void add_work(db::simple_tuple *stpl)
   {
      queue.push(stpl, stpl->get_strat_level());
   }

   inline bool has_work(void) const { return !queue.empty(); }

   virtual void assert_end(void) const
   {
      in_queue_node::assert_end();
      assert(!has_work());
   }

   virtual void assert_end_iteration(void) const
   {
      in_queue_node::assert_end_iteration();
      assert(!has_work());
   }

/*Making compatible with simulator*/
static const vm::node_val NO_NEIGHBOR = (vm::node_val)-1;

   static const face_t INITIAL_FACE = BOTTOM;
   static const face_t FINAL_FACE = TOP;
// returns a pointer to a certain face, allowing modification

   vm::node_val *get_node_at_face(const face_t face) {
      switch(face) {
         case BOTTOM: return &bottom;
         case NORTH: return &north;
         case EAST: return &east;
         case WEST: return &west;
         case SOUTH: return &south;
         case TOP: return &top;
         default: assert(false);
      }
   }

   face_t get_face(const vm::node_val node) {
      if(node == bottom) return BOTTOM;
      if(node == north) return NORTH;
      if(node == east) return EAST;
      if(node == west) return WEST;
      if(node == south) return SOUTH;
      if(node == top) return TOP;
      return INVALID_FACE;
   }

   inline bool has_been_instantiated(void) const
   {
      return instantiated_flag;
   }

   inline void set_instantiated(const bool flag)
   {
      instantiated_flag = flag;
   }

   inline void inc_neighbor_count(void)
   {
      ++neighbor_count;
   }

   inline void dec_neighbor_count(void)
   {
      --neighbor_count;
   }

   inline size_t get_neighbor_count(void) const
   {
      return neighbor_count;
   }

/*Changed constructor to conform to new member variables*/
   explicit serial_node(const db::node::node_id _id, const db::node::node_id _trans, vm::all *all):
      in_queue_node(_id, _trans, all),
      INIT_DOUBLE_QUEUE_NODE(),
      queue(all->PROGRAM->MAX_STRAT_LEVEL),
	  top(NO_NEIGHBOR), bottom(NO_NEIGHBOR), east(NO_NEIGHBOR),
      west(NO_NEIGHBOR), north(NO_NEIGHBOR), south(NO_NEIGHBOR),
      instantiated_flag(false),
      neighbor_count(0)
    {
	top = bottom = west = east = north = south = -1;
	}

   virtual ~serial_node(void) { }
};

}

#endif
