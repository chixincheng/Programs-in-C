#include "helper.h"

//return 0 if strings are equal, 1 otherwise.
int stringcmp(char *a, char *b){
	int ret = 0;
	while(*a != '\0' || *b != '\0'){
		if(*a != *b){
			ret =1;
			break;
		}
		a++;
		b++;
	}
	return ret;
}
//convert string of int to int.
int stringtoint(char *str){
	int neg;
	if(*str == '-'){
		neg = -1;
		*str = *(str+1);
	}
	int revoutput = 0;
	while(*str != '\0'){
		revoutput = revoutput * 10 + *str;
	}
	int output = 0;
	while(revoutput != 0){
		output = output * 10 + (revoutput%10);
		revoutput = revoutput/10;
	}
	if(neg == -1){
		output = -output;
	}
	return output;
}