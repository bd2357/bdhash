#ifndef BDHASH_KEY_H
#define BDHASH_KEY_H
#include <stdint.h>
#include <stddef.h>

typedef struct bdkey_t
{
    uintptr_t key;  // the original key or pointer to it
    uint32_t len;   // the len of the key (test against sizeof uintptr_t)
    uint32_t hash;  // the hash of the key
} bdkey_t;

// basically, if it fits use it.  It is up to the user to
// correctly manage this. If key is a long string or structure
// it must be const
#define GET_KEY_MEM(key_instance) \
(((key_instance).len <= sizeof (key_instance).key) ? \
    &(key_instance).key : \
    (void const *)((key_instance).key) )

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
bdkey_t package_bdkey(void const *key_obj, size_t objlen);

/**
 * @brief internal function to hash a string into a key
 * 
 * @param key_str pointer to string to hash
 * @return bdkey_t - the resulting key object
 */
bdkey_t package_bdkey_str(char const *key_str); 

/**
 * @brief internal function to hash a integer into a key
 * 
 * @param key_val - the signed value to hash 
 * @return bdkey_t - the resulting key object
 */
bdkey_t package_bdkey_val(intptr_t key_val);


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
