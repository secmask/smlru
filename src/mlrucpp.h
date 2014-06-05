/*
 * mlrucpp.h
 *
 *  Created on: Jun 3, 2014
 *      Author: secmask
 */

#ifndef MLRUCPP_H_
#define MLRUCPP_H_
#include "mlru.h"
#include <limits>
namespace org {
namespace sm {

class mlrucpp {
public:
	mlrucpp(uint32_t max_item,uint64_t max_mem=std::numeric_limits<uint64_t>::max());
	void set(const byte_type* key,uint32_t klen,const byte_type* value,uint32_t vlen);
	void set(const byte_type* key,const byte_type* value); // assume work on string, klen = strlen(key) +1, vlen = strlen(value)+1
	const byte_type* get(const byte_type* key,uint32_t klen,uint32_t *olen=0);
	const byte_type* get(const byte_type* key,uint32_t* olen=0);
	void remove(const byte_type* key,uint32_t klen);
	void remove(const byte_type* key);
	uint32_t size();
	void dump();
	virtual ~mlrucpp();
private:
	mlru *cache;
};

} /* namespace sm */
} /* namespace org */

#endif /* MLRUCPP_H_ */
