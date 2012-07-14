
#ifndef QUEUE_HEAP_IMPLEMENTATION_HPP
#define QUEUE_HEAP_IMPLEMENTATION_HPP

#define HEAP_DEFINE_DATA \
	typedef std::vector<heap_object> heap_vector;	\
	heap_vector heap
	
#define HEAP_DEFINE_EMPTY				\
	inline bool empty(void) const		\
	{											\
		return heap.empty();				\
	}
	
#define HEAP_DEFINE_INVALID_PRIORITY	\
	static const int INVALID_PRIORITY = -1
	
#define HEAP_DEFINE_UTILS								\
	inline int left(const int parent) const		\
	{															\
		int i = (parent << 1) + 1;						\
		return (i < (int)heap.size()) ? i : -1;	\
	}															\
																\
	inline int right(const int parent) const		\
	{															\
		int i = (parent << 1) + 2;						\
		return (i < (int)heap.size()) ? i : -1;	\
	}															\
																\
	inline int parent(const int child) const		\
	{															\
		if(child != 0)										\
			return (child - 1) >> 1;					\
		return -1;											\
	}
	
#define HEAP_DEFINE_PRINT												\
	void print(std::ostream& out)										\
	{																			\
		for(typename heap_vector::iterator it(heap.begin()), 	\
											end(heap.end());				\
			it != end;														\
			++it)																\
		{																		\
			out << it->data << " (" << it->val << ") ";			\
		}																		\
	}
	
#define HEAP_DEFINE_HEAPIFYUP											\
	bool heapifyup(int index)											\
	{																			\
		bool moved(false);												\
		while((index > 0) && (parent(index) >= 0) &&				\
			(HEAP_GET_PRIORITY(heap[parent(index)]) > HEAP_GET_PRIORITY(heap[index])))			\
		{																		\
			heap_object obj(heap[parent(index)]);					\
																				\
			HEAP_SET_INDEX(parent(index), heap[index]);			\
			HEAP_SET_INDEX(index, obj);								\
			index = parent(index);										\
			moved = true;													\
		}																		\
		return moved;														\
	}
	
#define HEAP_DEFINE_MIN_VALUE											\
	int min_value(void) const											\
	{																			\
		if(empty())															\
			return INVALID_PRIORITY;									\
																				\
		return heap.at(0).val;											\
	}
	
#define HEAP_SET_INDEX(IDX, OBJ)	do {								\
	heap[IDX] = OBJ;														\
	HEAP_GET_POS(OBJ) = IDX;											\
} while(false)
	
#define HEAP_DEFINE_HEAPIFYDOWN										\
	void heapifydown(const int index)								\
	{																			\
		int l = left(index);												\
		int r = right(index);											\
		const bool hasleft = (l >= 0);								\
		const bool hasright = (r >= 0);								\
																				\
		if(!hasleft && !hasright)										\
			return;															\
																				\
		if(hasleft &&														\
HEAP_GET_PRIORITY(heap[index]) <= HEAP_GET_PRIORITY(heap[l])\
		&& ((hasright && HEAP_GET_PRIORITY(heap[index]) <=		\
			HEAP_GET_PRIORITY(heap[r]))								\
					|| !hasright))											\
			return;															\
																				\
		int smaller;														\
		if(hasright && HEAP_GET_PRIORITY(heap[r]) <= 			\
					HEAP_GET_PRIORITY(heap[l]))						\
			smaller = r;													\
		else																	\
			smaller = l;													\
																				\
		heap_object obj(heap[index]);									\
																				\
		HEAP_SET_INDEX(index, heap[smaller]);						\
		HEAP_SET_INDEX(smaller, obj);									\
		heapifydown(smaller);											\
	}

#endif
