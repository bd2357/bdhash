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

uint8_t compare_bdkey(bdkey_t const* keyref1, bdkey_t const *keyref2)
{
    if(keyref1->hash != keyref2->hash) return 0;
    if(keyref1->len != keyref2->len) return 0;
    if (keyref1->len > sizeof keyref1->key)
    {
        return !memcmp((void const *)(keyref1->key),(void const *)(keyref2->key), keyref1->len);
    }
    else
    {
        return !memcmp(&(keyref1->key),&(keyref2->key), keyref1->len);
    }
}