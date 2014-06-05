#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/unordered_map.hpp>
#include <boost/date_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <list>
#include <algorithm>
#include "lruc.h"
#include "lrucpp.h"
#include "lrucache.h"
#include "mlrucpp.h"

//#include <jemalloc/jemalloc.h>

class lock{
public:
	lock(){
		std::cout << "ctor" << std::endl;
	}
	~lock(){
		std::cout << "destor" << std::endl;
	}
};
void malloc_free_test(){
	int N=1000000;
	void* ptr[N];
	for(int i=0;i<N;i++){
		ptr[i]=malloc(1024);
	}
}
void test_lru(){
	using namespace boost::posix_time;
	com::adtech::LRUCache<std::string,std::string> cache(100000);
	printf("set=====\n");
	int N=10000000;
	ptime start = microsec_clock::local_time();
	for(int i=0;i<N;i++){
		char key[32]={0};
		char value[32]={0};
		sprintf(key,"key-%d",i);
		sprintf(value,"value-%d",i);
		cache.insert(key,value);
	}
	std::cout << time_duration(microsec_clock::local_time()-start).total_milliseconds() << std::endl;
	printf("get=====\n");
	start = microsec_clock::local_time();
	for(int i=0;i<N;i++){
		char key[32]={0};
		//char value[32]={0};
		sprintf(key,"key-%d",i);
		//sprintf(value,"value-%d",i);
		//cache.set(key,value);
		cache.fetch_ptr(key);

		//printf("%s=%s\n",key,v);
	}
	std::cout << time_duration(microsec_clock::local_time()-start).total_milliseconds() << std::endl;
}
void test_mlru(){
	using namespace boost::posix_time;
	org::sm::mlrucpp cache(1000000,200*1024*1024);
	printf("set=====\n");
	int N=20000000;
	char key[64];
	char value[128];
	char *keys = (char*)malloc(N*10);
	ptime start = microsec_clock::local_time();
	for(int i=0;i<N;i++){
		sprintf(key,"key-%d",i);
		//sprintf(value,"valueeeeeeeeeee-%d",i);
		cache.set(&keys[i*10],10,"vvvvvvvvvv",11);
	}
	std::cout << time_duration(microsec_clock::local_time()-start).total_milliseconds() << std::endl;
	printf("get=====\n");
	start = microsec_clock::local_time();
	for(int i=0;i<N;i++){
		char key[32]={0};
		//char value[32]={0};
		sprintf(key,"key-%d",i);
		//sprintf(value,"value-%d",i);
		//cache.set(key,value);
		const char* v = cache.get(key);
		v = v?v:"NULL";
		//printf("%s=%s\n",key,v);
	}
	std::cout << time_duration(microsec_clock::local_time()-start).total_milliseconds() << std::endl;
}
void test_read(mlru* cache){
	uint32_t sz=0;
	char buff[128]={0};
	for(int i=0;i<10000000;i++){
		sprintf(buff,"keyyyyyiiiiiiiii%d",i);
		//mlru_set(cache,buff,strlen(buff),buff,strlen(buff));
		const char* v = (const char*)mlru_get(cache,buff,strlen(buff),&sz);
		v = v?v:"NULL";
		printf("%s=>%s\n",buff,v);
		//mlru_dump(cache);
	}
}
uint32_t hash1(const char* key, uint32_t key_length) {
	uint32_t hash = 5381; //use jdb2 hash function.
	int c;
	while (c = *key++){
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		key_length--;
	}
	while(key_length>0){
		c = *key++;
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
		key_length--;
	}
	return hash;
}
void test_hash(){
	char key[]="hellohello";
	for(int i=0;i<100000000;i++){
		hash1(key,9);
	}
}
int main(int argc, char **argv) {
	if(true){
		org::sm::mlrucpp cache(10,100000000);
		cache.set("1","11");
		cache.dump();
		cache.set("2","22");
		cache.set("3","33");
		cache.dump();
		cache.get("1");
		cache.dump();
		cache.remove("3");
		cache.dump();
	}
	//test_lru();
	//test_mlru();
	test_hash();
}
