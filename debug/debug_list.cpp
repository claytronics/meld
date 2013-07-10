
/*doubly linked lists to store breakpoints*/

#include <string>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "debug/debug_list.hpp"

using namespace std;


/*returns an empty breakpoint list - an empty breakpoint list has 
  one node but NULL data*/
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

/*frees entire list including the incduding the 
  type and name objects*/
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
    
    /*insert the data*/
    L->back->type = type;
    L->back->name = name;
    L->back->nodeID = nodeID;
    
    /*instantiate a tailing blank node*/
    L->back->next = (struct debugnode*)malloc(sizeof(struct debugnode));
    L->back->next->prev = L->back;
    L->back->next->next = NULL;
    L->back->next->type = NULL;
    L->back->next->nodeID = -1;
    L->back->next->name = NULL;
    L->back = L->back->next;
  
}

/*returns whether the debugList is empty*/
bool isListEmpty(debugList L){
  return L->front == L->back;
}

/*checks to see whether the the parameters inputed for a breakpoint
  are a hit in the list*/
bool isInBreakPointList(debugList L, char* type, char* name, int nodeID){

  if(isListEmpty(L))
    return false;

  for (struct debugnode* ptr = L->front; ptr->next!=NULL; ptr=ptr->next){
    if (!strcmp(ptr->type,type)&&
	//if "" the name or nodeId doesn't matter
	(!strcmp(ptr->name,name)||!strcmp(ptr->name,""))&&
	(ptr->nodeID == nodeID ||ptr->nodeID == -1))
      return true;
  }
  return false;
}




