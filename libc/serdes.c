#include <serdes.h>
#include <stdlib.h>
#include <string.h>

void serialize_int(int num,serdes_state* state) {
  state->buf=realloc(state->buf,state->sizeorpos+sizeof(int));
  *((int*)(state->buf+state->sizeorpos))=num;
  state->sizeorpos+=sizeof(int);
}

void serialize_ptr(void* ptr,serdes_state* state) {
  state->buf=realloc(state->buf,state->sizeorpos+sizeof(void*));
  *((void**)(state->buf+state->sizeorpos))=ptr;
  state->sizeorpos+=sizeof(void*);
}

void serialize_ary(void* ary,size_t len,serdes_state* state) {
  state->buf=realloc(state->buf,state->sizeorpos+len);
  memcpy(state->buf+state->sizeorpos,ary,len);
  state->sizeorpos+=len;
}


void start_deserialize(char* buf,serdes_state* state) {
  state->buf=buf;
  state->sizeorpos=0;
}

int deserialize_int(serdes_state* state) {
  int num=*((int*)(state->buf+state->sizeorpos));
  state->sizeorpos+=sizeof(int);
  return num;
}

void* deserialize_ptr(serdes_state* state) {
  void* ptr=*((void**)(state->buf+state->sizeorpos));
  state->sizeorpos+=sizeof(void*);
  return ptr;
}

void* deserialize_ary(size_t len,serdes_state* state) {
  void* ary=((void*)(state->buf+state->sizeorpos));
  state->sizeorpos+=len;
  return ary;
}

