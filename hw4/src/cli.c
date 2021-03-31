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


const char s[2] = " ";//what should this token be

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    if(out != stdout){
    	;
    }
    char *prom = "imp>";
    char *red = sf_readline(prom);
    while(strcmp(red,"") == 0){
    		red = sf_readline(prom);
    }
    char *cmd = strtok(red,s);

    if((in == NULL || in == stdin) && *red == EOF){
    	return -1;
    }

    while (strcmp(cmd,"quit") != 0){

    	if(strcmp(cmd,"help") == 0){
    		printf("%s\n","The available commands are: \nhelp, quit, type(file_type), printer(printer_name, file_type)\nconversion(file_type1,file_type2,conversion_program[arg1 arg2...]), printers\njobs, print(file_name,[printer1,printer2,...]), cancel(job_number)\npause(job_number), resume(job_number), disable(job_number), enable(job_number)");
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"type") == 0){
    		cmd = strtok(NULL,s);
    		define_type(cmd);
    		sf_printer_defined("TYPE",cmd);
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"priter") == 0){

    	}
    	else if(strcmp(cmd,"conversion") == 0){

    	}
    	else if(strcmp(cmd,"printers") == 0){

    	}
    	else if(strcmp(cmd,"jobs") == 0){

    	}
    	else if(strcmp(cmd,"print") == 0){

    	}
    	else if(strcmp(cmd,"cancel") == 0){

    	}
    	else if(strcmp(cmd,"pause") == 0){

    	}
    	else if(strcmp(cmd,"resume") == 0){

    	}
    	else if(strcmp(cmd,"disable") == 0){

    	}
    	else if(strcmp(cmd,"enable") == 0){

    	}
    	red = sf_readline(prom);
    	while(strcmp(red,"") == 0){
    		red = sf_readline(prom);
    	}
    	cmd = strtok(red,s);
    }

    if(first){//first time enter
    	first = -1;
    }
    return 1;
}
