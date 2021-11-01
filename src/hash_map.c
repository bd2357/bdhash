#include "hash_map.h"
#include "bdhash_node.h"
#include <assert.h>

typedef int fooValueType;
typedef int fooKeyType;

static bdhash_t *foo_map;
typedef struct foo_ret_t
{
    fooValueType value;
    bdhash_error_t err;        
} foo_ret_t;

void foo_init(void)
{
    static bdhash_t foo_store;
    foo_map = bdhash_init(&foo_store, BDH_AllowExtend);
    assert(foo_map == &foo_store);
}

void foo_clear(int free_memory)
{
    assert(foo_map != NULL);
    bdhash_ret_t ret = bdhash_clear(foo_map, free_memory);
    assert(ret.ret == BdhashOk);
}

int foo_set(fooKeyType key, fooValueType val)
{
    bdkey_t _key = package_bdkey(&key, sizeof key);
    bdval_t _val = package_bdval(&val, sizeof val);
    bdhash_ret_t ret = bdhash_set(foo_map, &_key, &_val);
    return ret.ret;
}

foo_ret_t foo_process_return_val(bdval_t _val)
{
    if(_val.len > 0)
    {
        return (foo_ret_t){.err=0, 
            .value=(sizeof(fooValueType) > sizeof _val.val)?
                    *(fooValueType*)(_val.val) :
                    *(fooValueType*)(&_val.val) };
    }
    else
    {
        return (foo_ret_t){.err = _val.len};
    }    
}

foo_ret_t foo_update(fooKeyType key, fooValueType val)
{
    bdkey_t _key = package_bdkey(&key, sizeof key);
    bdval_t _val = package_bdval(&val, sizeof val);
    return foo_process_return_val(
        bdhash_update(foo_map, &_key, &_val) );
}

foo_ret_t foo_get(fooKeyType key)
{
    bdkey_t _key = package_bdkey(&key, sizeof key);
    return foo_process_return_val(
        bdhash_get(foo_map, &_key) );
}

foo_ret_t foo_pop(fooKeyType key)
{
    bdkey_t _key = package_bdkey(&key, sizeof key);
    return foo_process_return_val(
        bdhash_pop(foo_map, &_key) );
}






