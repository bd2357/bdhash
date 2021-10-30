#ifndef BDHASH_H
#define BDHASH_H

#include <stdint.h>
#include <stddef.h>

// pick from some prime or app specific like
// 79, 101, 199, 443, 1019, ....
#define BD_HASH_SIZE 101    // first level hash size
#define BD_HASH_STORAGE (BD_HASH_SIZE)

typedef enum bdhash_error_t
{
    BdhashError = -4,
    BdhashMemoryError = -3,
    BdhashKeyNotFound = -2,
    BdhashNewKey = -1,
    BdhashOk = 0,
} bdhash_error_t;

typedef struct bdkey_t
{
    uintptr_t key;  // the original key or pointer to it
    uint32_t len;   // the len of the key (test against sizeof uintptr_t)
    uint32_t hash;  // the hash of the key
} bdkey_t;
// test instance of key for type
#define GET_KEY_MEM(key_instance) \
(((key_instance).len <= sizeof (key_instance).key) ? \
    &(key_instance).key : \
    (void const *)((key_instance).key) )

// generic container holding value or reference based on len
typedef struct bdval_t
{
    uintptr_t val;  // the original val or pointer to it
    int32_t len;   // the len of the val (test against sizeof uintptr_t)
} bdval_t;


typedef struct bdhash_ret_t
{
    uintptr_t value;    // the value
    bdhash_error_t err; // 0 for normal 
} bdhash_ret_t;

typedef struct bdhash_node_t
{
    struct bdhash_node_t *next; // next in chain
    bdkey_t key;              // actual key or pointer to key
    bdval_t value;            // actual value or pointer to item
} bdhash_node_t;

// storage for actual key/values or their references
typedef struct bdhash_extend_t
{
    struct bdhash_extend_t *next_extended;
    bdhash_node_t store[BD_HASH_STORAGE];
} bdhash_extend_t;

typedef struct bdhash_t
{
    bdhash_node_t *hash_table[BD_HASH_SIZE]; // the first level table
    bdhash_extend_t storage;                 // the first storage area
    bdhash_extend_t *last_storage;           // pnt to last added storage
    bdhash_node_t *free_list;  // pointer to next known free storage node
    uint32_t items;   // count of stored items
    uint32_t extends; // number of bdhash_extend_t blocks allocated
    uint8_t do_not_extend; // prevent malloc call
} bdhash_t;
// hash is full when either
#define HASH_FULL(hashP) \
  ((hashP)->free_list == NULL || \
   (((hashP)->extends+1)*BD_HASH_STORAGE)==(hashP)->items)

typedef struct bdhash_32_t
{
    bdhash_node_t node;
} bdhash_32_t;

typedef struct bdhash_str_t
{
    bdhash_node_t node;
} bdhash_str_t;

typedef struct bdhash_obj_t
{
    bdhash_node_t node;
} bdhash_obj_t;




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
