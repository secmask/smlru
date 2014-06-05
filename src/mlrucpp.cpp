/*
 * mlrucpp.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: secmask
 */

#include "mlrucpp.h"
#include <stdexcept>
#include <string.h>
namespace org {
namespace sm {

mlrucpp::mlrucpp(uint32_t max_item, uint64_t max_mem) {
	cache = mlru_create(max_item,max_mem);
	if(cache==NULL){
		throw std::runtime_error("cannot alloc cache");
	}
}

void mlrucpp::set(const byte_type* key, uint32_t klen, const byte_type* value,
		uint32_t vlen) {
	mlru_set(cache,key,klen,value,vlen);
}

void mlrucpp::set(const byte_type* key, const byte_type* value) {
	mlru_set(cache,key,strlen(key)+1,value,strlen(value)+1);
}

const byte_type* mlrucpp::get(const byte_type* key, uint32_t klen,uint32_t *olen) {
	return mlru_get(cache,key,klen,olen);
}

const byte_type* mlrucpp::get(const byte_type* key,uint32_t *olen) {
	return mlru_get(cache,key,strlen(key)+1,olen);
}

void mlrucpp::remove(const byte_type* key, uint32_t klen) {
	mlru_remove(cache,key,klen);
}

void mlrucpp::remove(const byte_type* key) {
	mlru_remove(cache,key,strlen(key)+1);
}

void mlrucpp::dump() {
	mlru_dump(cache);
}

uint32_t mlrucpp::size() {
	return mlru_size(cache);
}

mlrucpp::~mlrucpp() {
	if(cache){
		mlru_free(cache);
		cache = NULL;
	}
}

} /* namespace sm */
} /* namespace org */
