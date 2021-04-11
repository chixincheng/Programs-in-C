/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "imprimer.h"
#include "conversions.h"
#include "sf_readline.h"


int first = 1;
char *filetype;
char *printername;

typedef struct printer {
  char *name;       /* Printername extension for this type. */
  int id;        /* printer id number */
  FILE_TYPE type;   //type of this printer
  PRINTER_STATUS status;//printer status
} PRINTER;

typedef struct job{
	int id;//job id
	JOB_STATUS status;//job status
	FILE_TYPE type;   //type of this printer
	char *filename; //file name
	char *eligi[];//eligible printer list pos
}JOB;


int jobcount=0;//from 0 to 63
int id=0; //from 0 to 31


JOB jobarray[MAX_JOBS];
PRINTER parray[MAX_PRINTERS];

//define the new printer and put it into printer array
PRINTER define_printer(char *name, FILE_TYPE type){
	PRINTER newp = {name,id,type,PRINTER_DISABLED};
	parray[id] = newp;
	id++;
	return parray[id-1];
}

void processprint(CONVERSION **path,PRINTER p,JOB j);

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    if(out != stdout){
    	;
    }
    size_t size = 999;
    char *linebuf = malloc(size);
    char *red = malloc(size);
    char *cmd;
    size_t character = 0;
    if(in != stdin){//in batch mode
    	character = getline(&linebuf,&size,in);
    	strcpy(red,linebuf);
    }

    char *prom = "imp>";
    if(character == -1 || character == 0){//not in batch mode
    	red = sf_readline(prom);
	    while(strcmp(red,"") == 0){
	    		red = sf_readline(prom);
	    }

        if((in == NULL || in == stdin) && *red == EOF){
    		return -1;
    	}
	}
	if(in == stdin){
		cmd = strtok(red," ");
	}
	else{
		cmd = strtok(red,"\n");
	}

    while (strcmp(cmd,"quit") != 0){

    	if(strcmp(cmd,"help") == 0){
    		printf("%s\n","The available commands are: \nhelp, quit, type(file_type), printer(printer_name, file_type)\nconversion(file_type1,file_type2,conversion_program[arg1 arg2...]), printers\njobs, print(file_name,[printer1,printer2,...]), cancel(job_number)\npause(job_number), resume(job_number), disable(job_number), enable(job_number)");
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"type") == 0){
    		cmd = strtok(NULL," ");
    		define_type(cmd);
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"printer") == 0){
    		cmd = strtok(NULL," ");
    		char *name = cmd;//name
    		cmd = strtok(NULL," ");//printer type
    		if(find_type(cmd) != NULL){//type exist
    			PRINTER p = define_printer(name,*find_type(cmd));
    			sf_printer_defined(name,cmd);
    			printf("%s%i%s%s%s%s%s\n", "Printer: id=",p.id," name=",name," type=",cmd," Status=disable");
    			sf_cmd_ok();
    		}
    		else{//type do not exist
    			printf("%s%s\n", "Unknown file type :",cmd);
    			sf_cmd_error("printer");
    		}
    	}
    	else if(strcmp(cmd,"conversion") == 0){
    		conversions_init();
    		cmd = strtok(NULL," ");//type from
    		char *fname = cmd;
    		FILE_TYPE *f = find_type(cmd);
    		if(f != NULL){
    			cmd = strtok(NULL," ");//type to
    			char *tname = cmd;
    			FILE_TYPE *t = find_type(cmd);
    			if(t != NULL){//both type exist
    				cmd = strtok(NULL,"");//args for conversion program and program name
    				if(cmd != NULL){
    					define_conversion(fname,tname,&cmd);
    				//how to parse this cmd to only show conversion program name
    				}
    				else{
    					sf_cmd_error("NOT ENOUGH ARGUMENT");
    				}
    			}
    			else{//type dont exist
    				printf("%s%s\n", "Undeclared type:",cmd);
    				sf_cmd_error("conversion");
    			}
    		}
    		else{//type dont exist
    			printf("%s%s\n", "Undeclared type:",cmd);
    			sf_cmd_error("conversion");
    		}
    		conversions_fini();
    	}
    	else if(strcmp(cmd,"printers") == 0){
    		int count = 0;

    		while(parray[count].name != NULL){//exist a printer
    			char *sta = "notset";
    			if(parray[count].status == PRINTER_DISABLED){
    				sta="disable";
    			}
    			else if(PRINTER_DISABLED == PRINTER_IDLE){
    				sta="idle";
    			}
    			else{
    				sta="busy";
    			}
    			printf("%s%i%s%s%s%s%s%s\n", "Printer: id=",parray[count].id," name=",parray[count].name," type=", parray[count].type.name ," Status=", sta);
    			count++;
    		}
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"jobs") == 0){
    		for(int i=0;i<MAX_JOBS;i++){
    			if(jobarray[i].filename != NULL){
	    			printf("%s%i%s%s%s%s%s%i\n", "JOB[",jobarray[i].id,"] :type=",jobarray[i].type.name," filename=",jobarray[i].filename," status=",jobarray[i].status);
	    		}
    		}
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"print") == 0){
    		cmd = strtok(NULL," ");//file name
    		JOB creaj;

    		int printernum =0;//counter for eligible printer
    		creaj.filename=cmd;//file name
    		cmd = strtok(NULL," ");//printer name
    		while(cmd != NULL){
    			*((creaj.eligi)+printernum) = cmd;//store the eligible printer to a list
    			printernum++;
    			cmd = strtok(NULL," ");//printer name
    		}

    		char *token = creaj.filename;
    		strtok(token,".");
    		creaj.type.name = (strtok(NULL,""));//file type
    		creaj.status = JOB_CREATED;//status

    		if(jobcount<64){
	    		for(int i=0;i<MAX_JOBS;i++){//search for first avilable space
	    			if(jobarray[i].filename == NULL){
	    				creaj.id=i;
	    				jobarray[i] = creaj;
	    				i=64;//exit forloop
	    			}
	    		}
	    		jobcount++;
	    	}
	    	else{
	    		sf_cmd_error("Max JOB REACHED");
	    	}
	    	sf_job_created(creaj.id,creaj.filename,creaj.type.name);
	    	printf("%s%i%s%s%s%s%s\n", "JOB[",creaj.id,"] :type=",creaj.type.name," filename=",creaj.filename," status=created");
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"cancel") == 0){

    	}
    	else if(strcmp(cmd,"pause") == 0){

    	}
    	else if(strcmp(cmd,"resume") == 0){

    	}
    	else if(strcmp(cmd,"disable") == 0){
    		cmd = strtok(NULL," ");
    		char *dname = cmd;//name

    		for(int i=0;i<MAX_PRINTERS;i--){
    			if(parray[i].name != NULL && parray[i].name == dname){
    				parray[i].status = PRINTER_DISABLED;//disable printer
    				sf_printer_status(parray[i].name,parray[i].status);
    			}
    		}
    	}
    	else if(strcmp(cmd,"enable") == 0){
    		cmd = strtok(NULL," ");
    		char *ename = cmd;//name

    		for(int i=0;i<MAX_PRINTERS;i--){
    			if(parray[i].name != NULL && parray[i].name == ename){
    				parray[i].status = PRINTER_IDLE;//enable printer
    				sf_printer_status(parray[i].name,parray[i].status);
    				//scan through jobarray,find any job waiting for this printer
    				for(int j =0;j<MAX_JOBS;j--){
    					if(jobarray[j].status == JOB_CREATED){
    						int elijc = 0;
    						int notfound =0;
    						while(*jobarray[j].eligi+elijc != NULL && notfound==0){
    							if(*jobarray[j].eligi+elijc == parray[i].name){
    								notfound=-1;//found the printer, quit while
    							}
    						}
    						conversions_init();
    						CONVERSION **path = find_conversion_path(jobarray[j].type.name,parray[i].type.name);
    						//if a path exist or two type is the same and no conversion needed
    						if(path != NULL || jobarray[j].type.name == parray[i].type.name){
    							processprint(path,parray[i],jobarray[j]);
    						}
    						conversions_fini();
    					}
    				}
    			}
    		}
    	}
    	//get next command
    	if(in == stdin){
	    	red = sf_readline(prom);
	    	while(strcmp(red,"") == 0){
	    		red = sf_readline(prom);
	    	}
	    }
	    else{
	    	getline(&linebuf,&size,in);
	    	strcpy(red,linebuf);
	    }
		if(in == stdin){
			cmd = strtok(red," ");
		}
		else{
			cmd = strtok(red,"\n");
		}
    }

    if(first){//first time enter
    	first = -1;
    }
    free(linebuf);
    free(red);
    return 1;
}
void processprint(CONVERSION **path,PRINTER p,JOB j){
	if(path == NULL){//no conversion needed
		j.status = JOB_RUNNING;
		sf_job_status(j.id,j.status);
		int filed = imp_connect_to_printer(p.name,p.type.name,PRINTER_NORMAL);
		p.status = PRINTER_BUSY;
		sf_printer_status(p.name,p.status);
		if(fork() == 0){
			char *cmd = "/bin/cat";
			char *argv[0];
			dup2(filed,1);
			execvp(cmd,argv);
		}
	}
	else{//conversion pipeline happens here

	}
}