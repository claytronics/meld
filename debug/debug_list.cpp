
/*doubly linked lists to store breakpoints*/

#include <string>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

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


bool isInBreakPointList(debugList L, char* type, 
			char* name, int nodeID);
void insertBreak(debugList L, char* type, 
		 char* name, int nodeID);
void listFree(debugList L);
debugList newBreakpointList();


//inserts a new list
debugList newBreakpointList(){
  debugList newList = (debugList)malloc(sizeof(debugList));
  newList->back = (struct debugnode *)malloc(sizeof(struct debugnode));
  newList->front = newList->back;
  newList->front->next = NULL;
  newList->front->prev = NULL;
  newList->front->type = NULL;
  newList->front->name = NULL;
  newList->front->nodeID = -1;
  return newList;
}

//frees entire list including the object that 
//a node points to
void listFree(debugList L){
  
  if (L == NULL) return;
  
  struct debugnode* tmp;
  struct debugnode* ptr = L->front;

  while(ptr!=NULL){
    tmp = ptr;
    ptr = ptr->next;
    if (tmp->type!=NULL)
      free(tmp->type);
    if(tmp->name!=NULL)
      free(tmp->name);
    free(tmp);
  }
  free(L);
}
    
//insert object at end of list
//object must be dynamically allocated
void insertBreak(debugList L, char* type, char* name, int  nodeID){
    L->back->type = type;
    L->back->name = name;
    L->back->nodeID = nodeID;
    L->back->next = (struct debugnode*)malloc(sizeof(struct debugnode));
    L->back->next->prev = L->back;
    L->back->next->next = NULL;
    L->back->next->type = NULL;
    L->back->next->nodeID = -1;
    L->back->next->name = NULL;
    L->back = L->back->next;
  
}

bool isListEmpty(debugList L){
  return L->front == L->back;
}

bool isInBreakPointList(debugList L, char* type, char* name, int nodeID){
  if(isListEmpty(L))
    return false;
  for (struct debugnode* ptr = L->front; ptr->next!=NULL; ptr=ptr->next){
    if (!strcmp(ptr->type,type)&&
	(!strcmp(ptr->name,name)||!strcmp(ptr->name,""))&&
	(ptr->nodeID == nodeID ||ptr->nodeID == -1))
      return true;
  }
  return false;
}

  
 




