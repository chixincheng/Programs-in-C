#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"
#include "helper.h"
#include "image.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif




int main(int argc, char **argv)
{
    if(validargs(argc, argv) == -1){
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    else if(validargs(argc,argv) == 0){ //when valiargs return 0
    	pgm_to_birp(stdin,stdout);

    	//
    	if(global_options == 0x80000000){// -h flag is specified
    		USAGE(*argv, EXIT_SUCCESS);
    		return EXIT_SUCCESS;
    	}
    	return EXIT_SUCCESS; // <- not sure here
    }
    if(global_options & HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    // TO BE IMPLEMENTED
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
