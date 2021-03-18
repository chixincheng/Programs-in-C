#include <helper.h>
#include <sfmm.h>

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
	int pos = 1;
	while(size > comp){
		comp = comp*2;
		pos = pos+1;
	}
	return pos;
}
void initfreelisthead(void* heads){
	for (int i = 0; i < NUM_FREE_LISTS; ++i)
	{
		sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
		sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
	}
}


// void *coalesce(void *ptr){

// }