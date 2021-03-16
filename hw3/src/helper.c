#include <helper.h>

size_t roundup16(size_t toberound){
	while(toberound % 16 != 0){
		toberound = toberound+1;
	}
	return toberound;
}
int postosearch(size_t size){
	if(size <= 32){
		return 0;
	}
	size_t comp = 64;
	int pos = 2;
	while(size > comp){
		comp = comp*2;
		pos = pos+1;
	}
	return pos;
}