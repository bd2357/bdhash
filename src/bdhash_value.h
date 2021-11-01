/**
 * @file bdhash_value.h
 * @author Bryce Deary
 * @brief simple structure for holding a value
 * @date 2021-10-31
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef BDHASH_VALUE_H
#define BDHASH_VALUE_H
#include <stdint.h>
#include <stddef.h>

typedef struct bdval_t
{
    uintptr_t val;  // the original value or pointer to it
    int32_t len;    // the len of the value (test against sizeof uintptr_t)
} bdval_t;

// basically, if it fits use it.  It is up to the user to
// correctly manage this. (like fixing strings or ensuring
// referenced storage is const if it needs to be)
#define GET_VAL_MEM(val_instance) \
(((val_instance).len <= sizeof (val_instance).val) ? \
    &(val_instance).val : \
    (void const *)((val_instance).val) )


/**
 * @brief copy the content of bdval_t object
 * 
 * @param val 
 * @param dest 
 */
void bdval_copy(bdval_t const *val, void *dest);

/**
 * @brief put value in a container
 * 
 * @param val_obj - pointer to the data
 * @param objlen - the size of the data
 * @return bdval_t container holding either a pointer or instance
 */
bdval_t package_bdval(void const *val_obj, size_t objlen);


#endif // BDHASH_VALUE_H
