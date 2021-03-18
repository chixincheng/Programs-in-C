#ifndef helper_h
#define helper_h
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

size_t roundup16(size_t toberound);
int postosearch(size_t size);
void initfreelisthead(void *ptr);
void *coalesce(void *ptr);

#define getsize(p) (((*p)>>4)<<4) //get the size of header


#endif