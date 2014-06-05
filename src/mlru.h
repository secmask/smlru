/*
 * mlru.h
 *
 *  Created on: Jun 3, 2014
 *      Author: secmask
 */

#ifndef MLRU_H_
#define MLRU_H_
#include <stdint.h>
typedef enum {
	MLRU_NO_ERROR = 0,
	MLRU_MISSING_CACHE,
	MLRU_MISSING_KEY,
	MLRU_MISSING_VALUE
} mlru_error;
typedef struct cache_item_ cache_item;
typedef struct mlru_ mlru;
typedef struct blob_ blob;
typedef char byte_type;
struct blob_ {
	byte_type *buffer;
	uint32_t length;
};
struct cache_item_ {
	blob key;
	blob value;
	cache_item *slot_next;
	cache_item *link_prev;
	cache_item *link_next;
};

struct mlru_ {
	time_t seed;
	uint32_t capacity;
	uint32_t item_count;
	uint64_t used_mem;
	uint64_t max_mem;
	cache_item* free_cached;
	cache_item** table; //the hashtable
	cache_item* head;
	cache_item* tail;
};
#ifdef __cplusplus
extern "C" {
#endif

void mlru_free(mlru* cache);
mlru *mlru_create(uint32_t num_slots, uint64_t max_mem);
const byte_type* mlru_get(mlru* cache, const byte_type* key, uint32_t klen,	uint32_t *outlen);
mlru_error mlru_set(mlru* cache, const byte_type* key, uint32_t klen, const byte_type* value, uint32_t vlen);
mlru_error mlru_remove(mlru *cache, const byte_type* key, uint32_t keylen);
uint32_t mlru_size(mlru* cache);
mlru_error mlru_dump(mlru* cache);

#ifdef __cplusplus
}
#endif
#endif /* MLRU_H_ */
