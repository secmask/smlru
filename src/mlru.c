/*
 * mlru.c
 *
 *  Created on: Jun 3, 2014
 *      Author: secmask
 */
#include "mlru.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef NULL
#define NULL 0
#endif
uint32_t mlru_hash(mlru *cache, const byte_type *key, uint32_t key_length) {
	uint32_t hash = 5381; //use jdb2 hash function.
	int c;
	while (key_length--){
		c = *key++;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	return hash%cache->capacity;
}
#define error_for(conditions, error)  if(conditions) {return error;}
#define test_for_missing_cache()      error_for(!cache, MLRU_MISSING_CACHE)
#define test_for_missing_key()        error_for(!key || klen == 0, MLRU_MISSING_KEY)
#define test_for_missing_value()      error_for(!value || vlen == 0, MLRU_MISSING_VALUE)

static void* mlru_alloc_mem(size_t length) {
	return calloc(1, length);
}
static void mlru_free_mem(void* p) {
	free(p);
}
static void* mlru_clone(const void* p, uint32_t length) {
	void* m = mlru_alloc_mem(length);
	memcpy(m, p, length);
	return m;
}
mlru_error mlru_dump(mlru* cache) {
	test_for_missing_cache();
	cache_item* it = cache->head;
	printf("dump========\n");
	while (it != NULL) {
		printf("%s=%s\n", (char*) it->key.buffer, (char*) it->value.buffer);
		it = it->link_next;
	}
	return MLRU_NO_ERROR;
}
// compare a key against an existing item's key
static int mlru_cmp_keys(cache_item *item, const byte_type *key, uint32_t key_length) {
	if (key_length != item->key.length)
		return 1;
	else
		return memcmp(key, item->key.buffer, key_length);
}
mlru *mlru_create(uint32_t num_slots, uint64_t max_mem) {
	mlru *cache = (mlru*) mlru_alloc_mem(sizeof(mlru));
	cache->free_cached = NULL;
	size_t tabsize = num_slots * sizeof(cache_item*);
	cache->table = (cache_item**) mlru_alloc_mem(tabsize);
	cache->max_mem = max_mem;
	cache->capacity = num_slots;
	cache->used_mem = tabsize;
	cache->item_count = 0;
	return cache;
}
void mlru_free_item(cache_item *item) {
	mlru_free_mem(item->key.buffer);
	mlru_free_mem(item->value.buffer);
	mlru_free_mem(item);
}
cache_item *mlru_pop_or_create_item(mlru *cache) {
	cache_item *item = NULL;
	if (cache->free_cached) {
		item = cache->free_cached;
		cache->free_cached = item->link_next;

		item->link_next = NULL;
		item->link_prev = NULL;
		item->slot_next = NULL;
	} else {
		item = (cache_item *) mlru_alloc_mem(sizeof(cache_item));
	}
	return item;
}
void mlru_free(mlru* cache) {
	cache_item *it = cache->head;
	cache_item *c = it;
	while (it) {
		c = it;
		it = it->link_next;
		mlru_free_item(c);
	}
	if(cache->free_cached){
		mlru_free_mem(cache->free_cached);
		cache->free_cached=NULL;
	}
}
cache_item* linklist_detach_item(mlru* cache, cache_item* item) {
	cache_item *pre = item->link_prev;
	cache_item *next = item->link_next;
	if (item == cache->head) {
		cache->head = item->link_next;
	}
	if (item == cache->tail) {
		cache->tail = item->link_prev;
	}
	if (pre) {
		pre->link_next = next;
	}
	if (next) {
		next->link_prev = pre;
	}
	return item;
}
/*
 * add and set 'item' as head
 * */
void linklist_set_head(mlru* cache, cache_item* item) {
	item->link_prev = NULL;
	if (cache->head) {
		cache->head->link_prev = item;
		item->link_next = cache->head;
		cache->head = item;
	} else {
		item->link_next = NULL;
		cache->head = cache->tail = item;
	}
}
/*
 * 'item' was removed from cache (hashtable and linklist), we can decide to cache it
 * for reuse later or free it immediately.
 * */
void mlru_return_item(mlru* cache, cache_item *item) { //return item to free_cached, reuse it later?
	cache->used_mem -= (item->key.length + item->value.length + sizeof(cache_item));
	cache->item_count--;
	if (cache->free_cached == NULL) {//current implementation only cache 1 entry.
		mlru_free_mem(item->key.buffer);
		mlru_free_mem(item->value.buffer);
		item->link_next = item->link_prev = item->slot_next = NULL;
		cache->free_cached = item;
	} else {
		mlru_free_item(item);
	}
}
/*
 * find item by key
 * */
cache_item* mlru_get_item(mlru* cache, const byte_type* key, uint32_t keylen) {
	uint32_t h = mlru_hash(cache, key, keylen);
	cache_item *it = cache->table[h];
	while (it && mlru_cmp_keys(it, key, keylen)) {
		it = it->slot_next;
	}
	return it;
}
/*
 * remove an item from hashtable.
 * */
void hash_remove_item(mlru* cache,cache_item *item){
	uint32_t hashindex = mlru_hash(cache, item->key.buffer, item->key.length);
	cache_item* it = cache->table[hashindex];
	cache_item* prev = NULL;
	while (it && mlru_cmp_keys(it, item->key.buffer, item->key.length)) {
		prev = it;
		it = it->slot_next;
	}
	if (prev) {
		prev->slot_next = item->slot_next;
	} else {
		cache->table[hashindex] = item->slot_next;//no prev=> item is the head of slot item list.
	}
}
/*
 * completely remove and release and item from cache.
 * it will detach the item from linklist first, remove item from hashtable then.
 * and move item into free zone.
 * */
mlru_error mlru_remove_item(mlru* cache,cache_item *item) {
	if(item){
		linklist_detach_item(cache,item);
		hash_remove_item(cache,item);
		mlru_return_item(cache,item);
	}
	return MLRU_NO_ERROR;
}
/*
 * check for item count and size limit condition.
 * */
void mlru_check_limit(mlru* cache) {
	while (cache->item_count > cache->capacity) {
		mlru_remove_item(cache,cache->tail);
	}
	while (cache->used_mem > cache->max_mem) {
		mlru_remove_item(cache,cache->tail);
	}
}
/*
 * get cache value by key, outlen is fill with length of the value if not NULL.
 * return pointer to value if there a value for 'key', NULL on otherwise.
 * */
const byte_type* mlru_get(mlru* cache, const byte_type* key, uint32_t klen, uint32_t *outlen) {
	if (cache==NULL ||key==NULL) return NULL;
	cache_item* it = mlru_get_item(cache, key, klen);
	if (it) {
		if (it != cache->head) {
			linklist_detach_item(cache, it);
			linklist_set_head(cache, it);
		}
		if(outlen)
			*outlen = it->value.length;
		return it->value.buffer;
	} else {
		if(outlen)
			*outlen = 0;
	}
	return NULL;
}
/*
 * put an item into cache.
 * */
mlru_error mlru_set(mlru* cache, const byte_type* key, uint32_t klen,
		const byte_type* value, uint32_t vlen) {
	test_for_missing_cache();
	test_for_missing_key();
	test_for_missing_value();
	uint32_t hash_index = mlru_hash(cache, key, klen);
	cache_item *it = cache->table[hash_index];
	cache_item *prev = NULL;
	while (it && mlru_cmp_keys(it, key, klen)) {//search for an exist entry
		prev = it;
		it = it->slot_next;
	}
	if (it) {//update exist entry
		cache->used_mem -= it->value.length;
		cache->used_mem += vlen;
		free(it->value.buffer);
		it->value.buffer = mlru_clone(value, vlen);
		it->value.length = vlen;
		linklist_detach_item(cache, it);
	} else {//create a new entry
		it = mlru_pop_or_create_item(cache);
		it->value.buffer = mlru_clone(value, vlen);
		it->value.length = vlen;
		it->key.buffer = mlru_clone(key, klen);
		it->key.length = klen;
		cache->used_mem += (klen + vlen + sizeof(cache_item));
		cache->item_count++;
		if (prev) {
			prev->slot_next = it;
			//printf("slot %d insert %s after %s\n",hash_index,it->key.buffer,prev->key.buffer);
		} else {
			//printf("slot %d for %s\n",hash_index,it->key.buffer);
			cache->table[hash_index] = it;
		}
	}
	linklist_set_head(cache, it);
	mlru_check_limit(cache);
	return MLRU_NO_ERROR;
}
uint32_t mlru_size(mlru* cache){
	if(cache==NULL)return UINT32_MAX;//???
	return cache->item_count;
}
mlru_error mlru_remove(mlru *cache,const byte_type* key,uint32_t klen){
	test_for_missing_cache();
	test_for_missing_key();
	return mlru_remove_item(cache,mlru_get_item(cache,key,klen));
}
