/**
 * \file 
*/

#ifndef RPC_H
#define RPC_H

/**
 * Represents an RPC fumctiom 
*/
typedef struct RPCFuncInfo {
  char name[32]; //!< THe name of the function
  void* (*code)(void*); //!< A pointer to the code that implements the funtcion
} RPCFuncInfo;

#endif
