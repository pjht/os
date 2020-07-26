/**
 * \file 
*/

#ifndef STRING_H
#define STRING_H
#include <stddef.h>

/**
 * Copies a block of memory from one address to another
 * \param dest The destination address
 * \param src The source address
 * \param len The length of the block
 * \return the destination address
*/
void* memcpy(void* dest,const void* src,size_t len);
/**
 * Sets a block of memory to a byte
 * \param dest The block to set
 * \param val The byte to set it to
 * \param len The length of the block
 * \return the destination address
*/
void* memset(void* dest,int val,size_t len);
/**
 * Compares two strings
 * \param s1 String 1 to compare
 * \param s2 String 2 to compare
 * \return 0 if the strings are the same, otherwise the difference between the values of the first non-identical character
*/
int strcmp(const char* s1,const char* s2);
/**
 * Gets the length of a string
 * \param str The string to get the length of
 * \return the length of the string
*/
size_t strlen(const char* str);
/**
 * Copies a string from one address to another
 * \param dest The destination string
 * \param src The source string
 * \return the destination string
*/
char* strcpy(char* dest,const char* src);
/**
 * Reads characters from a string until it hits a delimeter character
 * \param str The string to read from (or NULL if you are continuing tokenization)
 * \param delim A stribg containing possible delimeter characters.
 * \return the read characters, not including the delmimeter. This string will be overwritten by sucessive calls to strtok.
*/
char* strtok(const char* str, const char* delim);
/**
 * Reverses a string in place
 * \param str The string to reverse
 * \return the string
*/
char* strrev(char *str);
/**
 * Converts a number to a base 10 string
 * \param n The number to convert
 * \param str A buffer to hold the converted number
*/
void int_to_ascii(int n,char* str);
/**
 * Converts a number to a base 16 string
 * \param n The number to convert
 * \param str A buffer to hold the converted number
*/
void hex_to_ascii(unsigned int n, char* str);
/**
 * Appends a character to a string
 * \param s The string
 * \param n The character to append
*/
void append(char* s, char n);
/**
 * Deletes the last character of a string
 * \param s The string
*/
void backspace(char* s);

#endif
