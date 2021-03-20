#include <helper.h>
#include <stdio.h>
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
	if(size > 1024){
		return 6;
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
sf_block *coalesce(sf_block *ptr){//return size to determine appropriate size class
	sf_header *currheader = (void *)(ptr);//current header
	size_t currsize = (*currheader>>4)<<4;//current block size
	sf_footer *currfooter = (void *)(ptr)+currsize-8;//initilize currfooter variable
	///////////////////////////////
	sf_footer *prevfoot = (void*)(ptr)-8;//previous block footer
	size_t prevsize = (*prevfoot>>4)<<4;
	sf_block *prevblock = (void*)(ptr)-prevsize;//previous block
	///////////////////////////////
	sf_block *nextblock = (void*)(ptr)+currsize;//next block
	sf_header nextheader = (*nextblock).header;//next header
	size_t nextsize = (nextheader>>4)<<4;//next block size
	///////////////////////////////
	size_t prevalloc = (((*prevblock).header) & 1);//prev alloc
	size_t nextalloc = (((*nextblock).header) & 1);//next alloc
	///////////////////////////////
	sf_header *prevheader = (void*)(prevblock);//prev header
	sf_block *remove;
	sf_block *remove2 = NULL;
	///////////////////////////////
	if(prevalloc && nextalloc){//case1, no coalesce, just set header and footer
		*currheader = (*currheader>>1)<<1;//set alloc to 0
		*currfooter = *currheader;//set footer equal to header
		ptr = ptr;
		remove = NULL;
	}
	else if(prevalloc && !nextalloc){//next block is not alloc, coalesce with next block
		currsize = (currsize + nextsize);//combine size
		currfooter = (void*)(ptr)+currsize-8;//new current footer pointer
		*currheader = currsize | 2;//set prev alloc
		*currfooter = *currheader;//set footer = header
		ptr = ptr;
		remove = nextblock;
	}
	else if(!prevalloc && nextalloc){//prev block is not alloc, coalesce with prev block
		prevsize = (prevsize + currsize);//combine size
		prevfoot = (void *)(prevblock)+prevsize-sizeof(sf_footer);//new prev footer pointer
		*prevheader = prevsize;//dont know if the prevblock of prev is alloc
		*prevfoot = *prevheader;//set footer = header
		remove = prevblock;
		ptr = prevblock;
	}
	else{//both prev and next block is not alloc, coalesce with both block
		prevsize = prevsize + currsize + nextsize;//combine all size
		prevfoot = (void *)(prevblock)+prevsize-sizeof(sf_footer);//new prev footer pointer
		*prevheader = prevsize;//dont know if the prevblock of prev is alloc
		*prevfoot = *prevheader;//set footer = header
		remove = prevblock;
		remove2 = nextblock;
		ptr = prevblock;
	}
	int rev = 0;//remove
	int rev2 =0;//remove2
	for(int i=0;i<NUM_FREE_LISTS;i++){
		if(remove != NULL){
			sf_block * first = sf_free_list_heads[i].body.links.next;
			if(first == remove){//if the remove block is the first inthe free list
				sf_free_list_heads[i].body.links.next = first->body.links.next;
				rev =1;
			}
		}
		if(remove2 != NULL){
			sf_block * first = sf_free_list_heads[i].body.links.next;
			if(first == remove){//if the remove block is the first inthe free list
				sf_free_list_heads[i].body.links.next = first->body.links.next;
				rev2 =1;
			}
		}
	}
	if(rev == 0 && remove != NULL){//not yet remove, not first in any list
		remove->body.links.prev->body.links.next = remove->body.links.next;
		remove->body.links.next->body.links.prev = remove->body.links.prev;
	}
	if(rev2 == 0 && remove2 != NULL){
		remove2->body.links.prev->body.links.next = remove2->body.links.next;
		remove2->body.links.next->body.links.prev = remove2->body.links.prev;
	}

	return ptr;
}