#include "unity.h"

#include "jtreg_hash.h"
#include <stdlib.h>
#include "hash_reg.h"

// helper statics
extern uint32_t count_free(jthash_t const *hash);
extern jthash_ret_t add_extended(jthash_t *hash);
uint32_t count_hash(jthash_t const *hash);

void setUp(void)
{
}

void tearDown(void)
{
}

void test_jtreg_hash_NeedToImplement(void)
{
    TEST_IGNORE_MESSAGE("Need to Implement jtreg_hash");
}

void test_jthash_init_static(void)
{
    jthash_t my_hash;

    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash, 0));

    TEST_ASSERT_EQUAL(0, my_hash.do_not_extend);
    TEST_ASSERT_EQUAL_PTR(NULL, my_hash.storage.next_extended);
    TEST_ASSERT_EQUAL(0, my_hash.items);
    TEST_ASSERT_EQUAL(0, my_hash.extends);
    // should be pointing at the last cell
    TEST_ASSERT_EQUAL_PTR(&my_hash.storage.store[JT_HASH_STORAGE-1], my_hash.free_list);
}

void test_jthash_init_malloc(void)
{
    jthash_t *another_hash = jthash_init(NULL, 1);

    TEST_ASSERT_EQUAL(1, another_hash->do_not_extend);
    TEST_ASSERT_EQUAL_PTR(&another_hash->storage.store[JT_HASH_STORAGE-1], another_hash->free_list);
    TEST_ASSERT_EQUAL_PTR(NULL, another_hash->storage.next_extended);
    TEST_ASSERT_EQUAL(0, another_hash->items);
    TEST_ASSERT_EQUAL(0, another_hash->extends);
    free(another_hash);
}



void test_jthash_clear_bad_config(void)
{
    jthash_extend_t temp;
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash, 1) );
    my_hash.storage.next_extended = &temp;
    my_hash.last_storage = &my_hash.storage;
    jthash_ret_t ret = jthash_clear(&my_hash, 0);
    TEST_ASSERT_EQUAL(JthashError, ret.err);
    TEST_ASSERT_EQUAL(3, ret.value);

    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash, 1) );
    my_hash.storage.next_extended = &temp;
    my_hash.last_storage = &temp;
    ret = jthash_clear(&my_hash, 0);
    TEST_ASSERT_EQUAL(JthashError, ret.err);
    TEST_ASSERT_EQUAL(6, ret.value);

    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash, 1) );
    my_hash.storage.next_extended = &temp;
    my_hash.last_storage = &temp;
    my_hash.extends = 3;
    ret = jthash_clear(&my_hash, 0);
    TEST_ASSERT_EQUAL(JthashError, ret.err);
    TEST_ASSERT_EQUAL(2, ret.value);

}

void test_jthash_clear_no_extended(void)
{
    jthash_t my_hash = (jthash_t){0};
    TEST_ASSERT_EQUAL(JthashOk, jthash_clear(&my_hash, JTH_KeepExtended).err);
    TEST_ASSERT_EQUAL_PTR(&my_hash.storage.store[JT_HASH_STORAGE-1], my_hash.free_list);
}

void test_jthash_clear_keep_extended(void)
{
    #define EXTENDS 3
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash,0));
    for(int i=0; i<EXTENDS; i++)
    {
        TEST_ASSERT_EQUAL(i+1, add_extended(&my_hash).value);
    }

    TEST_ASSERT_EQUAL(EXTENDS, my_hash.extends);
    TEST_ASSERT_EQUAL((EXTENDS+1)*JT_HASH_STORAGE, count_free(&my_hash));

    TEST_ASSERT_EQUAL(JthashOk, jthash_clear(&my_hash, JTH_KeepExtended).err);

    TEST_ASSERT_EQUAL(EXTENDS, my_hash.extends);
    TEST_ASSERT_EQUAL((EXTENDS+1)*JT_HASH_STORAGE, count_free(&my_hash));
    TEST_ASSERT_NOT_NULL(my_hash.last_storage);

    jthash_extend_t *ext = &my_hash.storage;
    for(int i =0; i<=EXTENDS; i++)
    {
        TEST_ASSERT_NOT_NULL_MESSAGE(ext, "cleared storage should retain links");
        ext = ext->next_extended;
    }
    TEST_ASSERT_NULL_MESSAGE(ext, "last extended should point to NULL");
    TEST_ASSERT_EQUAL(0, my_hash.do_not_extend);
    TEST_ASSERT_EQUAL(0, my_hash.items);
}

void test_jthash_clear_free_extended(void)
{
    #define EXTENDS 3
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash,0));
    for(int i=0; i<EXTENDS; i++)
    {
        TEST_ASSERT_EQUAL(i+1, add_extended(&my_hash).value);
    }

    TEST_ASSERT_EQUAL(EXTENDS, my_hash.extends);
    TEST_ASSERT_EQUAL((EXTENDS+1)*JT_HASH_STORAGE, count_free(&my_hash));

    TEST_ASSERT_EQUAL(JthashOk, jthash_clear(&my_hash, JTH_FreeExtended).err);

    TEST_ASSERT_EQUAL(0, my_hash.extends);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE, count_free(&my_hash));
    TEST_ASSERT_NULL(my_hash.storage.next_extended);
    TEST_ASSERT_EQUAL(0, my_hash.do_not_extend);
    TEST_ASSERT_EQUAL(0, my_hash.items);
    TEST_ASSERT_NULL(my_hash.last_storage);
}

void test_jthash_set(void)
{
    int items = 10;
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash,JTH_DoNotExtend));
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE, count_free(&my_hash));
    for(uint16_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        TEST_ASSERT_EQUAL(JthashNewKey, jthash_set(&my_hash, &key, &(jtval_t){.val=i, sizeof i}).err);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items, count_free(&my_hash));

    for(uint16_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        TEST_ASSERT_EQUAL(JthashOk, jthash_set(&my_hash, &key, &(jtval_t){.val=i*i, sizeof i}).err);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items, count_free(&my_hash));

    for(uint16_t i=0; i<items; i++)
    {
        uint8_t k = i;
        jtkey_t key = package_jtkey(&k, sizeof k);
        TEST_ASSERT_EQUAL(JthashNewKey, jthash_set(&my_hash, &key, &(jtval_t){.val=i*i, sizeof i}).err);
    }
    TEST_ASSERT_EQUAL(items*2, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items*2, count_free(&my_hash));    
}

// test we can update but not set keys
void test_jthash_update(void)
{
    int items = 10;
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash,JTH_DoNotExtend));
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE, count_free(&my_hash));
    for(uint16_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        jtval_t old_val = jthash_update(&my_hash, &key, &(jtval_t){.val=i, sizeof i});
        TEST_ASSERT_EQUAL(JthashKeyNotFound, old_val.len);
    }
    TEST_ASSERT_EQUAL(0, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE, count_free(&my_hash));

    // now set some byte size keys
    for(uint8_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        TEST_ASSERT_EQUAL(JthashNewKey, jthash_set(&my_hash, &key, &(jtval_t){.val=i, sizeof i}).err);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items, count_free(&my_hash));

    // try again with slight wrong sized keys
    for(uint16_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        jtval_t old_val = jthash_update(&my_hash, &key, &(jtval_t){.val=i, sizeof i});
        TEST_ASSERT_EQUAL(JthashKeyNotFound, old_val.len);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items, count_free(&my_hash));

    // last time with correct sized keys
    for(uint8_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        jtval_t old_val = jthash_update(&my_hash, &key, &(jtval_t){.val=i*i, sizeof i});
        TEST_ASSERT_EQUAL(sizeof i, old_val.len);
        TEST_ASSERT_EQUAL(i, old_val.val);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items, count_free(&my_hash));

    // and verify the updates happened
    for(uint8_t i=0; i<items; i++)
    {
        jtkey_t key = package_jtkey(&i, sizeof i);
        jtval_t old_val = jthash_get(&my_hash, &key);
        TEST_ASSERT_EQUAL(sizeof i, old_val.len);
        TEST_ASSERT_EQUAL(i*i, old_val.val);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE-items, count_free(&my_hash));

}

void test_jthash_get(void)
{
    int items = 300;
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash,JTH_AllowExtend));
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE, count_free(&my_hash));
    for(uint16_t i=0; i<items; i++)
    {
        uint32_t big = i*i;
        jtkey_t key = package_jtkey(&big, sizeof big);
        TEST_ASSERT_EQUAL(JthashNewKey, jthash_set(&my_hash, &key, &(jtval_t){.val=i*6, sizeof i}).err);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(3*JT_HASH_STORAGE-items, count_free(&my_hash));

    for(uint16_t i=0; i<items; i++)
    {
        uint32_t big = i*i;
        jtkey_t key = package_jtkey(&big, sizeof big);
        jtval_t val = jthash_get(&my_hash, &key); 
        TEST_ASSERT_EQUAL(sizeof i, val.len);
        TEST_ASSERT_EQUAL(i*6, val.val); 
    }

    for(uint16_t i=0; i<items; i++)
    {
        uint16_t big2 = i*i; 
        jtkey_t key = package_jtkey(&big2, sizeof big2);
        jtval_t val = jthash_get(&my_hash, &key); 
        TEST_ASSERT_EQUAL(JthashKeyNotFound, val.len);
    }
}

void test_jthash_pop(void)
{
    int items = 300;
    jthash_t my_hash;
    TEST_ASSERT_EQUAL_PTR(&my_hash, jthash_init(&my_hash,JTH_AllowExtend));
    TEST_ASSERT_EQUAL(JT_HASH_STORAGE, count_free(&my_hash));
    for(uint16_t i=0; i<items; i++)
    {
        uint32_t big = i*i;
        jtkey_t key = package_jtkey(&big, sizeof big);
        TEST_ASSERT_EQUAL(JthashNewKey, jthash_set(&my_hash, &key, &(jtval_t){.val=i*6, sizeof i}).err);
    }
    TEST_ASSERT_EQUAL(items, my_hash.items);
    TEST_ASSERT_EQUAL(3*JT_HASH_STORAGE-items, count_free(&my_hash));
    TEST_ASSERT_EQUAL(items, count_hash(&my_hash));

    for(int16_t i=items-1; i>=0; i--)
    {
        uint32_t big = i*i;
        jtkey_t key = package_jtkey(&big, sizeof big);
        jtval_t val = jthash_pop(&my_hash, &key); 
        TEST_ASSERT_EQUAL(sizeof i, val.len);
        TEST_ASSERT_EQUAL(i*6, val.val); 
    }
    TEST_ASSERT_EQUAL(0, my_hash.items);
    TEST_ASSERT_EQUAL(3*JT_HASH_STORAGE, count_free(&my_hash));
    TEST_ASSERT_EQUAL(0, count_hash(&my_hash));
}
