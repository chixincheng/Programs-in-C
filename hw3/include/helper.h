#ifndef helper_h
#define helper_h
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sfmm.h>

size_t roundup16(size_t toberound);
int postosearch(size_t size);
void initfreelisthead(void *ptr);
sf_block *coalesce(sf_block *ptr);
size_t numfreeblock(size_t num);


#endif