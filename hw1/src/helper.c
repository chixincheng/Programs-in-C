#include "helper.h"
#include "bdd.h"
//return 0 if strings are equal, 1 otherwise.
int stringcmp(char *a, char *b){
	int ret = 0;
	if(a == 0x0 && b == 0x0) //if both null then equal, if only one null then not equal
		return ret;
	else{
		if(a == 0x0)
		{
			ret = 1;
			return ret;
		}
		else if(b == 0x0){
			ret = 1;
			return ret;
		}
	}
	while(*a != '\0' || *b != '\0'){ //while both string not null
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
//compare two bdd_node, 0 means equal, -1 means unequal
int nodecompare(BDD_NODE a, BDD_NODE b){
	if(a.level == b.level){
		if(a.left == b.left){
			if(a.right == b.right){
				return 0;
			}
		}
	}
	return -1;
}