/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <helper.h>
#include "debug.h"
#include "sfmm.h"

void *sf_malloc(size_t size) {
	if(size == 0){
		return NULL;
	}
	else{
		size_t required = roundup16(size);
		int pos = postosearch(size);
		int allc = 1;
		for(int i=pos;i<NUM_FREE_LISTS;i++){
			sf_block block = *(sf_free_list_heads+i);
			sf_header head = block.header;
			size_t len = head >> 4;
			len = len << 4;//size of the free block
			if(len > required){//got enough free space to allocate
				size_t mask = 0x2;
				//setting the prv alloc of the next block to be 1;
				block.body.links.next->header = block.body.links.next->header | mask;
				allc = 0;
				//do i need to perform spliting block
				return sf_free_list_heads+i;
			}
			else{
				;
			}
		}
		while (allc == 1){//while allocation is not done keep looping
			//current freelist do not have enought free space, do sf_mem_grow and coalescing
			char *addres = sf_mem_grow();
		}
	}

}

void sf_free(void *pp) {
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
