#ifndef BDHASH_H
#define BDHASH_H

#include <stdint.h>
#include <stddef.h>
// forward references
typedef struct bdkey_t bdkey_t;
typedef struct bdval_t bdval_t;
typedef struct bdhash_t bdhash_t;

// possible error messages (negative values)
typedef enum bdhash_error_t
{
    BdhashError = -4,
    BdhashMemoryError = -3,
    BdhashKeyNotFound = -2,
    BdhashNewKey = -1,
    BdhashOk = 0,
} bdhash_error_t;

// General return value 
typedef struct bdhash_ret_t
{
    uintptr_t value; // the value or pointer to value based on len
    int32_t ret;     // Neg error or pos length 
} bdhash_ret_t;

// pick from some prime or app specific like
// 79, 101, 199, 443, 1019, ....
#define BD_HASH_SIZE 101    // first level hash size
#define BD_HASH_STORAGE (BD_HASH_SIZE)
// default behaviour of extendable memory
// is currently to allow, cmd line -D can also switch
#ifndef BD_HASH_STATIC 
// #define BD_HASH_STATIC 
#endif

/**
 * These are used to manage memory allocation and can be
 * overridden by user to provide user unique memory management
 */ 
void __attribute__((weak)) *bdhash_malloc(size_t size);
void __attribute__((weak)) bdhash_free(void *obj);

/**
 * @brief init a bdhash_t structure, if null create one on the heap
 *      and return it.
 * 
 * @param hash pointer to a hash structure or NULL to allocate a new one.
 * @return bdhash_t* pointer to the hash or NULL if an error occurred.
 */
typedef enum {BDH_AllowExtend = 0, BDH_DoNotExtend = 1} DoNotExtend_t;
bdhash_t *bdhash_init(bdhash_t *hash, DoNotExtend_t do_not_extend);

/**
 * @brief Clear the hash object, does not free any referenced values or keys
 * 
 * @param hash pointer to hash object
 * @return bdhash_ret_t where value contains current capacity 
 */
typedef enum {BDH_KeepExtended=0, BDH_FreeExtended=1} FreeExtParm_t;
bdhash_ret_t bdhash_clear(bdhash_t *hash, FreeExtParm_t free_extended);

/**
 * @brief Assign a value to a key, if key does not exist it will be created
 * 
 * @param hash - pointer to hash object
 * @param key - pointer to a key object
 * @param value - pointer to a value object
 * @return bdhash_ret_t bdhashOk, bdhashMemoryError, or bdhashError
 */
bdhash_ret_t bdhash_set(bdhash_t *hash, bdkey_t const *key, bdval_t const *value);

/**
 * @brief Update a key with a new value, returns old value, also returns
 *        error code if key does not exist.
 *
 * @param hash - pointer to hash object
 * @param key an existing key
 * @param value the new value
 * @return bdval_t the old value or bdhashKeyNotFound or 
 */
bdval_t bdhash_update(bdhash_t *hash, bdkey_t const *key, bdval_t const *value);

/**
 * @brief Get the current value of the key
 * 
 * @param hash - pointer to hash object
 * @param key an existing key
 * @return bdval_t The value or bdhashKeyNotFound
 */
bdval_t bdhash_get(bdhash_t *hash, bdkey_t const *key);

/**
 * @brief Remove a key from the hash
 * 
 * @param hash - pointer to hash object 
 * @param key an exiting key
 * @return bdval_t The existing value or bdhashKeyNotFound
 */
bdval_t bdhash_pop(bdhash_t *hash, bdkey_t const *key);

#endif // BDHASH_H
