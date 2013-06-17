
#ifndef DEBUG_LIST_HPP
#define DEBUG_LIST_HPP


struct debugnode{
  struct debugnode* next;
  struct debugnode* prev;
  char* type;
  char* name;
  int nodeID;
};


struct list_header{
  struct debugnode* front;
  struct debugnode* back;
};

typedef struct list_header* debugList;

bool isInBreakPointList(debugList L, char* type, char* name, int nodeID);
void insertBreak(debugList L, char* type, char* name, int nodeID);
void listFree(debugList L);
debugList newBreakpointList();


#endif
