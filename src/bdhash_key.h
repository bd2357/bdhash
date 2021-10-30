#ifndef BDHASH_KEY_H
#define BDHASH_KEY_H

#include "bdhash.h"

/**
 * @brief internal hash generator, if you need to change hash this is the 
 *        place
 * 
 * @param key_obj pointer to memory to hash
 * @param keylen length of thing to hash in bytes
 * @return uint32_t - the hash up to 32 bits, actual has limit is in
 *                    bdhash.h
 */
uint32_t make_bdhash(void const *key_obj, size_t keylen);

/**
 * @brief internal function to hash some memory segment and store the
 *        information in a structure.
 * 
 * @param key_obj pointer to memory to hash
 * @param keylen length of thing to hash in bytes
 * @return bdkey_t - the resulting key object
 */
bdkey_t  package_bdkey(void const *key_obj, size_t keylen);

/**
 * @brief internal function to test key match to existing key
 * 
 * @param keyref pointer to existing key
 * @param test_key_obj key memory to test
 * @return uint_8 1 if match or 0 if no match
 */
uint8_t test_bdkey(bdkey_t const *keyref, void const *test_key_obj);

/**
 * @brief compare two keys
 * 
 * @param keyref1 pointer to first key
 * @param keyref2 pointer to second key
 * @return uint_8 1 if match or 0 if no match
 */
uint8_t compare_bdkey(bdkey_t const *keyref1, bdkey_t const *keyref2);

#endif // BDHASH_KEY_H
