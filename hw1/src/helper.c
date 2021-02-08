#include "helper.h"

//return 0 if strings are equal, 1 otherwise.
int stringcmp(char *a, char *b){
	int ret = 0;
	while(*a != '\n' || *b != '\n'){
		if(*a != *b){
			ret =1;
			break;
		}
		a++;
		b++;
	}
	return ret;
}