#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {

	char *ptr1 = sf_malloc(1);
	*(ptr1) = 'A';

	char *ptr2 = sf_malloc(1);
	*(ptr2) = 'B';

	sf_free(ptr1);
	sf_free(ptr2);

	//sf_show_blocks();
	int *ptr3 = sf_malloc(2020 * sizeof(int));
	*(ptr3+0)=1;
	*(ptr3+1)=69;
	*(ptr3+2)=80;
	*(ptr3+23)=69;

	//sf_show_heap();
	char *ptr4 = sf_malloc(8168);
	*(ptr4) = 'Y';

/*
	int *ptr5=sf_malloc(9000);
	if(ptr5!=NULL){
		*(ptr5) = 'A';
	}*/
	sf_show_heap();
	//printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
	//sf_show_blocks();


    return EXIT_SUCCESS;
}
