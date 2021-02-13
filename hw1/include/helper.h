#ifndef helper_h
#define helper_h
#include "bdd.h"

int stringcmp(char *a, char *b);
int stringtoint(char *str);
int nodecompare(BDD_NODE a,BDD_NODE b);
int searchNode(int key, BDD_NODE tocompare);
int hashKey(int key);
#endif