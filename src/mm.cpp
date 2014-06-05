#include <list>
#include <algorithm>
#include "mlrucpp.h"
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
uint64_t currentMillis(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t time_in_mill = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
	return time_in_mill;
}
void test_mlru(){
	org::sm::mlrucpp cache(1000000,200*1024*1024);
	int N=20000000;
	std::cout << "set===== on "<< N << " keys\n";
	char key[64];
	char value[128];
	char *keys = (char*)malloc(N*10);
	uint64_t start = currentMillis();
	for(int i=0;i<N;i++){
		sprintf(key,"key-%d",i);
		//sprintf(value,"valueeeeeeeeeee-%d",i);
		cache.set(&keys[i*10],10,"vvvvvvvvvv",11);
	}
	std::cout << (currentMillis()-start) << std::endl;
	std::cout << "get=====\n";
	start = currentMillis();
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
	std::cout << (currentMillis()-start) << std::endl;
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
	test_mlru();
}
