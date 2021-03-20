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
	sf_footer *currfooter = 0;//initilize currfooter variable
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
	sf_header prevheader = (*prevblock).header;//prev header
	///////////////////////////////
	if(prevalloc && nextalloc){//case1, no coalesce, just set header and footer
		*currheader = (*currheader>>1)<<1;//set alloc to 0
		*currfooter = *currheader;//set footer equal to header
		return ptr;
	}
	else if(prevalloc && !nextalloc){//next block is not alloc, coalesce with next block
		currsize = (currsize + nextsize);//combine size
		currfooter = (void*)(ptr)+currsize-8;//new current footer pointer
		*currheader = currsize | 2;//set prev alloc
		*currfooter = *currheader;//set footer = header
/*		ptr->body.links.prev->body.links.next = ptr->body.links.next;
		ptr->body.links.next->body.links.prev = ptr->body.links.prev;*/
		return ptr;
	}
	else if(!prevalloc && nextalloc){//prev block is not alloc, coalesce with prev block
		prevsize = (prevsize + currsize);//combine size
		prevfoot = (void *)(prevblock)+prevsize-sizeof(sf_footer);//new prev footer pointer
		prevheader = prevsize;//dont know if the prevblock of prev is alloc
		*prevfoot = prevheader;//set footer = header
		return prevblock;
	}
	else{//both prev and next block is not alloc, coalesce with both block
		prevsize = prevsize + currsize + nextsize;//combine all size
		prevfoot = (void *)(prevblock+prevsize-sizeof(sf_footer));//new prev footer pointer
		prevheader = prevsize;//dont know if the prevblock of prev is alloc
		*prevfoot = prevheader;//set footer = header
		return prevblock;
	}
}