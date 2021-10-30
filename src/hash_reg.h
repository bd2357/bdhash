#ifndef HASH_REG_H
#define HASH_REG_H

#include "jtreg_hash.h"

/**
 * @brief internal hash generator, if you need to change hash this is the 
 *        place
 * 
 * @param key_obj pointer to memory to hash
 * @param keylen length of thing to hash in bytes
 * @return uint32_t - the hash up to 32 bits, actual has limit is in
 *                    jtreg_hash.h
 */
uint32_t make_jthash(void const *key_obj, size_t keylen);

/**
 * @brief internal function to hash some memory segment and store the
 *        information in a structure.
 * 
 * @param key_obj pointer to memory to hash
 * @param keylen length of thing to hash in bytes
 * @return jtkey_t - the resulting key object
 */
jtkey_t  package_jtkey(void const *key_obj, size_t keylen);

/**
 * @brief internal function to test key match to existing key
 * 
 * @param keyref pointer to existing key
 * @param test_key_obj key memory to test
 * @return uint_8 1 if match or 0 if no match
 */
uint8_t test_jtkey(jtkey_t const *keyref, void const *test_key_obj);

/**
 * @brief compare two keys
 * 
 * @param keyref1 pointer to first key
 * @param keyref2 pointer to second key
 * @return uint_8 1 if match or 0 if no match
 */
uint8_t compare_jtkey(jtkey_t const *keyref1, jtkey_t const *keyref2);

#endif // HASH_REG_H
