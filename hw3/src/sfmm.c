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
	size_t adjsize = size+16;//16 byte for header and footer
	adjsize = roundup16(adjsize);//round to nearest multiple of 16
	char *str = sf_mem_start();//start of heap
	char *end = sf_mem_end();//end of heap
	if(str == end){ //first time allocating, setting up prologue and epilogue
		initfreelisthead((void *)(sf_free_list_heads+0));
		void *newmem = sf_mem_grow();//pointer to new allocated block
		sf_header prolohead= 32|1;//header of prologue
		sf_block *prolo = (sf_block *)(newmem + 8);//skip the first 8 byte of garbage data
		prolo->header = prolohead;//set the header of prologue
		*(sf_footer *)((void *) (prolo+24)) = prolohead;//set the footer of prologue

		sf_header epilohead= 1;
		sf_header *epilo = (sf_header *) (newmem+PAGE_SZ-8);
		*epilo = epilohead;

		//prologue = 32,8byte garbage
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = (sf_block *)(newmem + 40);
		//set header of new block |2 to set the prv alloca to 1
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next->header = ((PAGE_SZ-48) | 2);
		//prologue = 32 byte,8 byte garbage
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = (sf_block *)(newmem + 40);
		//set the footer of new free block
		*(sf_footer *)((void *)(sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next)
			-40+PAGE_SZ-8-sizeof(sf_footer))
		= ((PAGE_SZ-48) | 2);//(PAGE_SZ | 1<<1) to set the prv alloca to 1

		sf_block *currhead = (sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next);

		//currently the wilderness block only have one block, what to set next and prev?
		currhead->body.links.next = &sf_free_list_heads[NUM_FREE_LISTS-1];
		currhead->body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS-1];
		//sf_show_heap();
	}
	sf_show_blocks();
	while(sf_errno != ENOMEM)
	{
		for(int i=0;i<NUM_FREE_LISTS;i++){ //search for free blocks in free_list_heads
			if(sf_free_list_heads[i].body.links.next != &sf_free_list_heads[i]
				|| sf_free_list_heads[i].body.links.prev != &sf_free_list_heads[i])
			{	//when the block is not empty
				size_t fbsz = ((sf_free_list_heads[i].body.links.next->header) >> 4) << 4;//free block size
				if(fbsz >= adjsize){//if have enough free block
					//set header of returned block
					sf_free_list_heads[i].body.links.next->header = adjsize | 1;
					//to be returned,free block position + header size
					void *ret = ((void *)(sf_free_list_heads[i].body.links.next)+8);
					fbsz = fbsz-adjsize;//remainder free block size

					if(fbsz >= 32){//enough for 32 bytes
						//new address of block
						sf_free_list_heads[i].body.links.next = (sf_block *)(ret-8+adjsize);
						//set header for new free block, set prev alloc
						sf_free_list_heads[i].body.links.next->header = (fbsz | 2);
						*((sf_footer *)((void *)(sf_free_list_heads[i].body.links.next)
							+fbsz-sizeof(sf_footer))) = (fbsz | 2);

						sf_block *currhead = (sf_free_list_heads[i].body.links.next);
						currhead->body.links.next = &sf_free_list_heads[i];
						currhead->body.links.prev = &sf_free_list_heads[i];
					}
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
		void *pos =sf_mem_grow();//allocate more space to heap
		void *newend = sf_mem_end();//the new end

		sf_header currheader = sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next->header;
		size_t newheadersize = (((currheader)>>4)<<4) + 8 +PAGE_SZ;//not sure about this math <-
		sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next->header = (newheadersize);//increase wilderness block size,set prev alloc
		*((sf_footer *)(newend-8-sizeof(sf_footer))) = (newheadersize);//set the new footer same as header

		sf_header epilohead= 1;//new epiloheader
		sf_header *epilo = (sf_header *) (pos+PAGE_SZ-8);
		*epilo = epilohead;//setting new epiloheader
	}
	return NULL;
}

void sf_free(void *pp) {
	sf_block *ptr = (sf_block *)(pp-8);
	if(pp == NULL){//pointer is null
		abort();
	}
	if((size_t)(pp) % 16 != 0){//pointer is not 16-byte aligned
		abort();
	}
	size_t sze = ((*ptr).header>>4)<<4;
	if(sze % 16 != 0){//size of block is not multiple of 16
		abort();
	}
	if(sze < 32){//size less than minimum block
		abort();
	}
	if(((*ptr).header & 1) == 0){//allocated bit in header is 0
		abort();
	}
	if((void *)(ptr+sze) > sf_mem_end()){//block ends after heap
		abort();
	}
	sf_block *next = (*ptr).body.links.next;//next block
	if((void *)(next) > sf_mem_end()){//header of next block lies outside of current heap
		abort();
	}
	sf_block *prevblock = (*ptr).body.links.prev;//previous block
	//prev_alloc bit do not match alloc bit of previous block
	if(((*ptr).header & 2) == 0){//prev block is free
		if((((*prevblock).header) & 1) == 0){//not allocate
			size_t s = ((*prevblock).header>>4)<<4;
			sf_footer *foot = (void *)(ptr)+s-sizeof(sf_footer);//pointer + size of block - size of footer
			if(((*foot) & 1) != 0){//footer do not match header
				abort();
			}
		}
		else{//do not match
			abort();
		}
	}
	//pass all condition, valid pointer, start to free
	sf_block *rptr= coalesce(ptr);//coalesce the free block
	size_t list = ((*rptr).header>>4)<<4;
	list = postosearch(list);//list to insert
	//insert to the list
	sf_block *currhead = sf_free_list_heads[list].body.links.next;//current head of the freelist
	sf_free_list_heads[list].body.links.next = rptr;//insert free block to head of the freelist
	rptr->body.links.next = currhead;//link the old head with new head
	rptr->body.links.prev = currhead->body.links.prev;//new head->prev = old head->prev
	currhead->body.links.prev->body.links.next = rptr;//old tail's next = new head
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
