#ifndef SERDES_H
#define SERDES_H

#include <stddef.h>

/**
 * Represents the state of a (de)serialization
*/
typedef struct {
  char* buf; //!< The buffer getting (de)serialized
  size_t sizeorpos; //!< When serializing, the size of the buffer. When deserializing. the position in the buffer
} serdes_state;


/**
 * Serialize an integer
 * \param num The integer to serialize
 * \param state The state of the serialization
*/
void serialize_int(int num,serdes_state* state);

/**
 * Serialize a pointer
 * \param ptr The pointer to serialize
 * \param state The state of the serialization
*/
void serialize_ptr(void* ptr,serdes_state* state);



/**
 * Serialize a byte array of given length
 * \param ary The byte array to serialize
 * \param len The length of the byte array to serialize
 * \param state The state of the serilaization
*/
void serialize_ary(void* ary,size_t len,serdes_state* state);

/**
 * Serialize a string
 * \param str The string to serialize
 * \param state The state of the serilaization
*/
#define serialize_str(str,state) (serialize_int(strlen(str)+1,state),serialize_ary(str,strlen(str)+1,state))



/**
 * Start a deserialization
 * \param buf The buffer to deserialize
 * \param state A pointer to put the state of the deserialization in
*/
void start_deserialize(char* buf,serdes_state* state);

/**
 * Deserialize an integer
 * \param state The state of the deserialization
 * \return the deserialized integer
*/
int deserialize_int(serdes_state* state);

/**
 * Deserialize a pointer
 * \param state The state of the deserialization
 * \return the deserialized pointer
*/
void* deserialize_ptr(serdes_state* state);

/**
 * Deserialize a byte array of given length
 * \param len The length of the byte array to deserialize
 * \param state The state of the deserialization
 * \return the deserialized byte array
*/
void* deserialize_ary(size_t len,serdes_state* state);

/**
 * Deserialize a string
 * \param state The state of the deserialization
 * \return the deserialized string
*/
#define deserialize_str(state) (deserialize_ary(deserialize_int(state),state))


#endif

