/**
 * @file bdhash_value.c
 * @brief support function for bdval_t objects
 * @date 2021-10-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "bdhash.h"
#include "bdhash_value.h"
#include <string.h>

void bdval_copy(bdval_t const *val, void *dest)
{
    memcpy(dest, GET_VAL_MEM(*val), val->len);
    return;
}

bdval_t package_bdval(void const *val_obj, size_t objlen)
{
    bdval_t val_package = (bdval_t){.len = objlen};
    // if we can't store copy, store pointer
    if(objlen > sizeof val_package.val)
    {
        val_package.val = (uintptr_t)val_obj;
    }
    else
    {
        memcpy(&val_package.val, val_obj, objlen);
    }
    return val_package;

}

