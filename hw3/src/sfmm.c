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
#include "errno.h"

void *sf_malloc(size_t size) {
	if(size == 0){
		return NULL;
	}
	size_t adjsize = size+16;
	adjsize = roundup16(adjsize);//round to nearest multiple of 16
	char *str = sf_mem_start();//start of heap
	char *end = sf_mem_end();//end of heap
	if(str == end){ //first time allocating, setting up prologue and epilogue
		initfreelisthead((void *)(sf_free_list_heads+0));
		void *newmem = sf_mem_grow();//pointer to new allocated block
		sf_header prolohead= 32|1;//header of prologue
		sf_block *prolo = (sf_block *)(newmem + 8);//skip the first 8 byte of garbage data
		prolo->header = prolohead;//set the header of prologue
		*(sf_header *)((void *) (prolo+24)) = prolohead;//set the footer of prologue
		//set header of new block |2 to set the prv alloca to 1,prologue = 32,8byte garbage
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = (sf_block *)(newmem + 40);
		//set header of new free block
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next->header = ((PAGE_SZ-48) | 2);
		//prologue = 32,8byte garbage
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = (sf_block *)(newmem + 40);
		//set the footer of new free block
		*(sf_footer *)((void *)(sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next)
			-40+PAGE_SZ-8-sizeof(sf_footer))
		= ((PAGE_SZ-48) | 2);//(PAGE_SZ | 1<<1) to set the prv alloca to 1

		sf_header epilohead= 3;
		sf_header *epilo = (sf_header *) (newmem+8192-8);
		*epilo = epilohead;
	}
	sf_show_heap();
	while(sf_errno != ENOMEM)
	{
		for(int i=0;i<NUM_FREE_LISTS;i++){ //search for free blocks in free_list_heads
			while(sf_free_list_heads[i].body.links.next != &sf_free_list_heads[i]
				|| sf_free_list_heads[i].body.links.prev != &sf_free_list_heads[i])
			{	//when the block is not empty
				size_t fbsz = ((sf_free_list_heads[i].header) >> 4) << 4;//free block size
				if(fbsz >= adjsize){//if have enough free block

					//to be returned,free block position + header size
					void *ret = ((void *)(&sf_free_list_heads[i].body.links.next)+8);
					fbsz = fbsz-adjsize;//remainder free block size

					//new address of block
					(sf_free_list_heads[i].body.links.next) = (sf_block *)(ret+adjsize-8);
					//new header size for the allocated block
					sf_free_list_heads[i].body.links.next->header = (fbsz | 2);

/*					//set header and footer, (PAGE_SZ | 1<<1) to set the prv alloca to 1
					sf_free_list_heads[freelistheadpos].header = (fbsz | 1<<1);//set header of new free block
					//set the footer of new free block
					*(sf_header *)((void *)(sf_free_list_heads + freelistheadpos)+sizeof(sf_block)-sizeof(sf_header))
					= (fbsz | 1<<1);//(PAGE_SZ | 1<<1) to set the prv alloca to 1
					//insert the remainder to the appropriate freelist
					sf_free_list_heads[freelistheadpos].body.links.next = ret+adjsize;*/
					return ret;//enough free space, return the location
				}
			}
		}
		//current free_list_heads do not have enough free space, allocate more space
		sf_mem_grow();//allocate more space to heap
		void *newend = sf_mem_end();//the new end
		sf_header epilohead= 1;//new epiloheader
		*((sf_header *)(newend-8)) = epilohead;//setting new epiloheader
		sf_header currheader = sf_free_list_heads[NUM_FREE_LISTS-1].header;
		size_t newheadersize = (((currheader)>>4)<<4)+(PAGE_SZ-16);//PAGE_SZ -16 for epilo and footer
		sf_free_list_heads[NUM_FREE_LISTS-1].header = (newheadersize | 1<<1);//increase wilderness block size,set prev alloc
		*((sf_header *)(newend-16)) = (newheadersize | 1<<1);//set the new footer same as header
	}
	return NULL;
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
