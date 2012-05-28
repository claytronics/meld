
#ifndef QUEUE_UNSAFE_DOUBLE_QUEUE_HPP
#define QUEUE_UNSAFE_DOUBLE_QUEUE_HPP

#include "queue/intrusive_implementation.hpp"

namespace queue
{

template <class T>
class intrusive_unsafe_double_queue
{
private:

	DEFINE_INTRUSIVE_DOUBLE_DATA();

	QUEUE_DEFINE_TOTAL();

	DEFINE_INTRUSIVE_DOUBLE_OPS();
   
public:
	
	DEFINE_INTRUSIVE_CONST_ITERATOR();
   
	QUEUE_DEFINE_TOTAL_SIZE(); // size()
	DEFINE_INTRUSIVE_DOUBLE_EMPTY(); // empty()
	DEFINE_INTRUSIVE_IN_QUEUE(); // static in_queue()
   
   inline bool pop_if_not_empty(node_type& data)
   {
		DEFINE_INTRUSIVE_DOUBLE_POP_IF_NOT_EMPTY();
   }
   
   inline bool pop(node_type& data)
   {
		DEFINE_INTRUSIVE_DOUBLE_POP();
   }
   
   inline void push(node_type data)
   {
      push_tail_node(data);
   }

	inline void push_head(node_type data)
	{	
		push_head_node(data);
	}
   
   inline void move_up(node_type node)
   {
		DEFINE_INTRUSIVE_MOVE_UP();
   }

	inline void remove(node_type node)
	{
		DEFINE_INTRUSIVE_REMOVE();
	}

	DEFINE_INTRUSIVE_CONSTRUCTOR(intrusive_unsafe_double_queue);
   
   ~intrusive_unsafe_double_queue(void)
   {
      assert(empty());
   }
};

}

#endif
