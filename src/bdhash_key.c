#include "bdhash.h"
#include "bdhash_key.h"
#include <stdio.h>
#include <string.h>

uint32_t make_bdhash(void const *key, size_t keylen)
{
    uint32_t hashval;
    uint8_t const *s = key;
    for(hashval = 0; keylen; s++, keylen--)
    {
        hashval = *s + 31 * hashval;
    }
    return hashval % BD_HASH_SIZE;
}

// general package method, calculates the hash
bdkey_t  package_bdkey(void const *key, size_t keylen)
{
    bdkey_t key_package = 
        (bdkey_t){.len = keylen, .hash = make_bdhash(key, keylen)};
    // if we can't store copy, store pointer
    if(keylen > sizeof(uintptr_t))
    {
        key_package.key = (uintptr_t)key;
    }
    else
    {
        memcpy(&key_package.key, key, keylen);
    }
    return key_package;
}

// specific package method for strings
bdkey_t package_bdkey_str(char const *str_key)
{
    return package_bdkey(str_key, strlen(str_key));
}

// specific package method for integers
bdkey_t package_bdkey_int32(int32_t val_key)
{
    return package_bdkey(&val_key, sizeof val_key);
}

// specific package method for pointers
bdkey_t package_bdkey_ref(void *ref_key)
{
    return package_bdkey(&ref_key, sizeof ref_key);
}

// since we know purported length from key struct
// the test is just a memory compare
uint8_t test_bdkey(bdkey_t const *keyref, void const *test_key_obj)
{
    if(keyref)
    {
        // not testing the hash, just the memory
        void const *keymem = GET_KEY_MEM(*keyref);
        return !memcmp(keymem, test_key_obj, keyref->len);
    }
    return 0;
}

// here we test two key structs for equivalency 
uint8_t compare_bdkey(bdkey_t const* keyref1, bdkey_t const *keyref2)
{
    if(keyref1->hash != keyref2->hash) return 0;
    if(keyref1->len != keyref2->len) return 0;
    if (keyref1->len > sizeof keyref1->key)
    {
        // keys are pointers
        return !memcmp((void const *)(keyref1->key),(void const *)(keyref2->key), keyref1->len);
    }
    else
    {
        // keys hold instances
        return (keyref1->key == keyref2->key);
    }
}