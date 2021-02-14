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
/*struct HashMap{
	int lev;
	int lef;
	int right;
};*/

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
//implement hash table function below
int hashKey(int key){
	return key % BDD_HASH_SIZE;
}
int searchNode(int key, BDD_NODE tocompare){
	int startindex = hashKey(key);

	while(bdd_hash_map[startindex] != NULL){
		if(nodecompare(*bdd_hash_map[startindex],tocompare) == 0){
			return startindex;
		}
		startindex++;
		if(startindex > BDD_HASH_SIZE-1){
			startindex = startindex % BDD_HASH_SIZE;
		}
	}
	return -1;
}
unsigned char *rasterind(int width, int orgh, int height, unsigned char *raster,int half){
	if(half == 1){ //first half
		return raster;
	}
	else{ // second half
		int add;
		if(width != height){
			add = width * orgh;
		}
		else{
			add = width;
		}
		return raster+add;
	}
}