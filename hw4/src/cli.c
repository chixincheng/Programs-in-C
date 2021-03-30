/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"


int first = 1;
char *filetype;
char *printername;


const char s[2] = "-";//what should this token be

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    if(out != stdout){
    	;
    }
    if(in == NULL || in == stdin){
    	return -1;
    }

    //cmd =


    if(first){//first time enter
    	first = -1;
    }

}
