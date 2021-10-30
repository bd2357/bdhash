#ifndef JTREG_HASH_H
#define JTREG_HASH_H

#include <stdint.h>

// pick from some prime or app specific like
// 79, 101, 199, 443, 1019, ....
#define JT_HASH_SIZE 101    // first level hash size
#define JT_HASH_STORAGE (JT_HASH_SIZE)

typedef enum jthash_error_t
{
    JthashError = -4,
    JthashMemoryError = -3,
    JthashKeyNotFound = -2,
    JthashNewKey = -1,
    JthashOk = 0,
} jthash_error_t;

typedef struct jtkey_t
{
    uintptr_t key;  // the original key or pointer to it
    uint32_t len;   // the len of the key (test against sizeof uintptr_t)
    uint32_t hash;  // the hash of the key
} jtkey_t;
// test instance of key for type
#define GET_KEY_MEM(key_instance) \
(((key_instance).len <= sizeof (key_instance).key) ? \
    &(key_instance).key : \
    (void const *)((key_instance).key) )

// generic container holding value or reference based on len
typedef struct jtval_t
{
    uintptr_t val;  // the original val or pointer to it
    int32_t len;   // the len of the val (test against sizeof uintptr_t)
} jtval_t;


typedef struct jthash_ret_t
{
    uintptr_t value;    // the value
    jthash_error_t err; // 0 for normal 
} jthash_ret_t;

typedef struct jthash_node_t
{
    struct jthash_node_t *next; // next in chain
    jtkey_t key;              // actual key or pointer to key
    jtval_t value;            // actual value or pointer to item
} jthash_node_t;

// storage for actual key/values or their references
typedef struct jthash_extend_t
{
    struct jthash_extend_t *next_extended;
    jthash_node_t store[JT_HASH_STORAGE];
} jthash_extend_t;

typedef struct jthash_t
{
    jthash_node_t *hash_table[JT_HASH_SIZE]; // the first level table
    jthash_extend_t storage;                 // the first storage area
    jthash_extend_t *last_storage;           // pnt to last added storage
    jthash_node_t *free_list;  // pointer to next known free storage node
    uint32_t items;   // count of stored items
    uint32_t extends; // number of jthash_extend_t blocks allocated
    uint8_t do_not_extend; // prevent malloc call
} jthash_t;
// hash is full when either
#define HASH_FULL(hashP) \
  ((hashP)->free_list == NULL || \
   (((hashP)->extends+1)*JT_HASH_STORAGE)==(hashP)->items)

typedef struct jthash_32_t
{
    jthash_node_t node;
} jthash_32_t;

typedef struct jthash_str_t
{
    jthash_node_t node;
} jthash_str_t;

typedef struct jthash_obj_t
{
    jthash_node_t node;
} jthash_obj_t;




/**
 * @brief init a jthash_t structure, if null create one on the heap
 *      and return it.
 * 
 * @param hash pointer to a hash structure or NULL to allocate a new one.
 * @return jthash_t* pointer to the hash or NULL if an error occurred.
 */
typedef enum {JTH_AllowExtend = 0, JTH_DoNotExtend = 1} DoNotExtend_t;
jthash_t *jthash_init(jthash_t *hash, DoNotExtend_t do_not_extend);

/**
 * @brief Clear the hash object, does not free any referenced values or keys
 * 
 * @param hash pointer to hash object
 * @return jthash_ret_t where value contains current capacity 
 */
typedef enum {JTH_KeepExtended=0, JTH_FreeExtended=1} FreeExtParm_t;
jthash_ret_t jthash_clear(jthash_t *hash, FreeExtParm_t free_extended);

/**
 * @brief Assign a value to a key, if key does not exist it will be created
 * 
 * @param hash - pointer to hash object
 * @param key - pointer to a key object
 * @param value - pointer to a value object
 * @return jthash_ret_t JthashOk, JthashMemoryError, or JthashError
 */
jthash_ret_t jthash_set(jthash_t *hash, jtkey_t const *key, jtval_t const *value);

/**
 * @brief Update a key with a new value, returns old value, also returns
 *        error code if key does not exist.
 *
 * @param hash - pointer to hash object
 * @param key an existing key
 * @param value the new value
 * @return jtval_t the old value or JthashKeyNotFound or 
 */
jtval_t jthash_update(jthash_t *hash, jtkey_t const *key, jtval_t const *value);

/**
 * @brief Get the current value of the key
 * 
 * @param hash - pointer to hash object
 * @param key an existing key
 * @return jtval_t The value or JthashKeyNotFound
 */
jtval_t jthash_get(jthash_t *hash, jtkey_t const *key);

/**
 * @brief Remove a key from the hash
 * 
 * @param hash - pointer to hash object 
 * @param key an exiting key
 * @return jtval_t The existing value or JthashKeyNotFound
 */
jtval_t jthash_pop(jthash_t *hash, jtkey_t const *key);

#endif // JTREG_HASH_H
