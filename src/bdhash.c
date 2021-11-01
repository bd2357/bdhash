#include "bdhash.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "bdhash_node.h"
#include <stdio.h>


void __attribute__((weak)) *bdhash_malloc(size_t size)
{
#ifdef BD_HASH_STATIC // no dynamic memory
    (void)size;
    return NULL;
#else
    return malloc(size);
#endif
}

void __attribute__((weak)) bdhash_free(void *obj)
{
#ifdef BD_HASH_STATIC // no dynamic memory
    (void)obj;
#else
    free(obj);
#endif
    return;
}

// add all unallocated to free list, does not update other meta
void add_extend_to_free(bdhash_t *hash, bdhash_extend_t *store)
{
    bdhash_node_t *next = store->store;
    for(int i = 0; i < (sizeof store->store/sizeof store->store[0]); i++)
    {
        if(!next->next) // un allocated
        {
            if(hash->free_list)
            {
                next->next = hash->free_list; // new node point to old head
            }
            else // new free list
            {
                next->next = next; // self terminate
            }
            hash->free_list = next; // update head
        }
        next++;
    }
}

// completely add a store to the hash
void add_extend_to_hash(bdhash_t *hash, bdhash_extend_t *store)
{
    if(!hash->last_storage) hash->last_storage = &hash->storage;

    hash->last_storage->next_extended = store; // add to list
    hash->last_storage = store;                // save list end
    hash->extends++;
    add_extend_to_free(hash, store);
}

// allocate and add one extended store to the hash if enabled
bdhash_ret_t add_extended(bdhash_t *hash)
{
    if(!hash->do_not_extend)
    {
        bdhash_extend_t *new_ext = bdhash_malloc(sizeof(bdhash_extend_t));
        if(!new_ext) return (bdhash_ret_t){.ret=BdhashMemoryError};
        *new_ext = (bdhash_extend_t){0};
        add_extend_to_hash(hash, new_ext); 
        return (bdhash_ret_t){.ret=BdhashOk, .value=hash->extends};
    }
    return (bdhash_ret_t){.ret=BdhashMemoryError};
}

void add_to_free(bdhash_t *hash, bdhash_node_t *node)
{
    if(hash->free_list) // link back at head
    {
        node->next = hash->free_list;
        hash->free_list = node;
    }
    else  // link back at head with termination
    {
        node->next = node;
        hash->free_list = node;
    }
}

// just walk any self terminated list counting entries
uint32_t count_list(bdhash_node_t *first)
{
    bdhash_node_t *next = first;
    uint32_t cnt=0;    
    while(next)
    {
        cnt++;
        if(next->next == next) break;
        next = next->next;
    }
    return cnt;    
}

// count the free list
uint32_t count_free(bdhash_t const *hash)
{
    return count_list(hash->free_list);
}

// count each bin list and all
uint32_t count_hash(bdhash_t const *hash)
{
    uint32_t cnt = 0;
    for(int h=0; h < sizeof hash->storage.store/sizeof hash->storage.store[0]; h++)
    {
        uint32_t hcnt = count_list(hash->hash_table[h]);
        cnt += hcnt;
        printf("h%u->%u,",h, hcnt);
    }
    printf("\n");
    return cnt;
}


// the do_not_extend prevents future malloc calls
bdhash_t *bdhash_init(bdhash_t *hash, DoNotExtend_t do_not_extend)
{
    if(!hash) // if not provided, allocate one
    {
        hash = bdhash_malloc(sizeof(bdhash_t));
        if(!hash) return NULL;
    }

    // zero out structure
    *hash = (bdhash_t){.do_not_extend = do_not_extend};
    add_extend_to_free(hash, &hash->storage);
    return hash;
}

bdhash_ret_t bdhash_clear(bdhash_t *hash, FreeExtParm_t free_extended)
{
    uint32_t extends = hash->extends;  // number of bdhash_extend_t blocks allocated
    bdhash_extend_t *first_extended = hash->storage.next_extended;
    bdhash_extend_t *last_storage = hash->last_storage;
    // if last storage, last link should be null
    if(last_storage && last_storage->next_extended) // internal error
    {
        return (bdhash_ret_t){.ret=BdhashError, .value=3};
    }
    // now clear all the hash bins and local store preserving the do_not_extend
    bdhash_init(hash, hash->do_not_extend);

    //now clear up extended memory if any
    bdhash_extend_t *next_extended = first_extended;
    uint32_t found = 0; // validate existing counter
    // now walk extended list to clear or free
    while(next_extended)
    {
        // grab next in chain before modifying current
        bdhash_extend_t *next = next_extended->next_extended;
    
        if(++found > extends) // internal error
        {
            return (bdhash_ret_t){.ret=BdhashError, .value=6};
        }

        if(free_extended)
        {
            next_extended->next_extended = NULL; // break link just in case
            bdhash_free(next_extended);
        }
        else // keep the block but clear it and then relink
        {
            // clear the actual store and relink
            *next_extended = (bdhash_extend_t){.next_extended=next};
            add_extend_to_hash(hash, next_extended);
        }
        next_extended = next; // move to next store
    }
    if(found != extends) // internal error
    {
        return (bdhash_ret_t){.ret=BdhashError, .value=2};
    }

    // return current capicity
    return (bdhash_ret_t){.value = BD_HASH_STORAGE * (hash->extends + 1), .ret=BdhashOk};
}

bdhash_node_t *bdfind_key(bdhash_t *hash, bdkey_t const *key, bdhash_node_t ***prev)
{
    bdhash_node_t *next = hash->hash_table[key->hash];
    if(prev) *prev = &(hash->hash_table[key->hash]);
    while(next)
    {
        if(compare_bdkey(&next->key, key))
        {
            return next;
        }
        if(prev) *prev = &next->next;
        if(next == next->next) break; // node self ref is terminator
        next = next->next;
    }
    return NULL; // not found
}


bdhash_ret_t bdhash_set(bdhash_t *hash, bdkey_t const *key, bdval_t const *value)
{
    // find existing key if it exists
    bdhash_node_t **link_prev;
    bdhash_node_t *node = bdfind_key(hash, key, &link_prev);
    if(node) // if already exists just update value
    {
        node->value = *value;
    }
    else // else store new node
    {
        if(hash->free_list) // if available
        {
            node = hash->free_list; // grab from list
            if(node->next == node) // if last in list
            {
                hash->free_list = NULL; // list is empty
            }
            else
            {
                hash->free_list = node->next;
            }
        }
        else 
        {
            if(add_extended(hash).ret)
            {
                return (bdhash_ret_t){.ret = BdhashMemoryError};
            }
            node = hash->free_list;
            hash->free_list = hash->free_list->next;
        }
        node->key = *key;
        node->value = *value;
        node->next = node;  // self ref is list terminator
        *link_prev = node;  // hook into hash list
        hash->items++;
        return (bdhash_ret_t){.value = (uintptr_t)node, .ret=BdhashNewKey};
    }
    return (bdhash_ret_t){.value = (uintptr_t)node};
}


bdval_t bdhash_update(bdhash_t *hash, bdkey_t const *key, bdval_t const *value)
{
    // find existing key if it exists
    bdhash_node_t *node = bdfind_key(hash, key, NULL);
    bdval_t old_val;
    if(node) // if already exists just update value
    {
        old_val = node->value;
        node->value = *value;
    }
    else
    {
        return (bdval_t){.len = BdhashKeyNotFound};
    }
    return (bdval_t){.val = old_val.val, .len = old_val.len};
}


bdval_t bdhash_get(bdhash_t *hash, bdkey_t const *key)
{
    // find existing key if it exists
    bdhash_node_t *node = bdfind_key(hash, key, NULL);

    if(node) // if exists return
    {
        return (bdval_t){.val = node->value.val , .len = node->value.len};
    }
    else
    {
        return (bdval_t){.len = BdhashKeyNotFound};
    }
}

bdval_t bdhash_pop(bdhash_t *hash, bdkey_t const *key)
{
    bdhash_node_t **link_prev;
    // find existing key if it exists
    bdhash_node_t *node = bdfind_key(hash, key, &link_prev);
    bdval_t old_val;
    if(node) // node exists so remove it
    {
        old_val = node->value; // save value
        if(node == node->next)  // last node in list
        {
            *link_prev = NULL;          // clear list
        }
        else
        {
            *link_prev = node->next;    // remove from active list
        }
        add_to_free(hash, node);
        hash->items--;
    }
    else
    {
        return (bdval_t){.len = BdhashKeyNotFound};
    }
    return (bdval_t){.val = old_val.val, .len = old_val.len};
}

