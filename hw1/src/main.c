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
	extern char *informat;
	extern char *outformat;
	int validcheck = validargs(argc, argv);
    if(validcheck == -1){
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    else if(validcheck == 0){ //when valiargs return 0, input valid
    	if(global_options == 0x80000000){// -h flag is specified
    		USAGE(*argv, EXIT_SUCCESS);
    		return EXIT_SUCCESS;
    	}
    	if(stringcmp(informat,"pgm") == 0)//input is pgm format
    	{
    		if(stringcmp(outformat,"birp") == 0)//outformat = birp
    		{
    			int ret =pgm_to_birp(stdin,stdout);
    			if(ret ==0){
    				return EXIT_SUCCESS;
    			}
    		}
    		else if(stringcmp(outformat,"ascii") == 0){
    			int ret =pgm_to_ascii(stdin,stdout);
    			if(ret ==0){
    				return EXIT_SUCCESS;
    			}
    		}
    		return EXIT_FAILURE;//invalid output
    	}
    	else if(stringcmp(informat,"birp") == 0)//input is birp format
    	{
    		if(stringcmp(outformat,"pgm") == 0)//output pgm
    		{
    			int ret =birp_to_pgm(stdin,stdout);
    			if(ret ==0){
    				return EXIT_SUCCESS;
    			}
    		}
    		else if(stringcmp(outformat,"ascii") == 0)//output ascii
    		{
    			int ret =birp_to_ascii(stdin,stdout);
    			if(ret ==0){
    				return EXIT_SUCCESS;
    			}
    		}
    		else if(stringcmp(outformat,"birp") ==0){//birp to birp perform transformation
    			int ret =birp_to_birp(stdin,stdout);
    			if(ret ==0){
    				return EXIT_SUCCESS;
    			}
    		}
    		return EXIT_FAILURE;//invalid output
    	}
    	return EXIT_FAILURE; // <- not sure here
    }
    if(global_options & HELP_OPTION){
        USAGE(*argv, EXIT_SUCCESS);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */