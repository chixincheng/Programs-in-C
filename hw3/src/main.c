#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {


	void *x = sf_malloc(20);
	void *y = sf_malloc(20);
	void *z = sf_realloc(x, 2040);
	void *w = sf_realloc(z,1);

	sf_show_heap();
	if(x || y || z ||w){
		;
	}

	//sf_show_heap();
    return EXIT_SUCCESS;
}
