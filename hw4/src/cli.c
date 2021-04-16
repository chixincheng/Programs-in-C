/*
 * Imprimer: Command-line interface
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

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
	int gid;
	char *filename; //file name
	char *eligi[];//eligible printer list pos
}JOB;


int jobcount=0;//from 0 to 63
int id=0; //from 0 to 31
int mpidcount =0;

JOB jobarray[MAX_JOBS];
PRINTER parray[MAX_PRINTERS];
int mpid[MAX_JOBS];

//define the new printer and put it into printer array
PRINTER define_printer(char *name, FILE_TYPE type){
	PRINTER newp = {name,id,type,PRINTER_DISABLED};
	parray[id] = newp;
	id++;
	return parray[id-1];
}

int processprint(CONVERSION **path,PRINTER p,JOB j,FILE *out);

volatile sig_atomic_t flag = -5;//process id will be stored
volatile JOB_STATUS schan = -5;//job status will be stored

void sighandler(){
	int chils;
	flag = waitpid(-1,&chils,0);
	if(WIFEXITED(chils)){//exit normally
		schan = JOB_FINISHED;
	}
	else{//crash
		schan =JOB_ABORTED;
	}
	if(WIFCONTINUED(chils)){//process continued
		schan = JOB_RUNNING;
	}
	if(WIFSTOPPED(chils)){//process stopped
		schan = JOB_PAUSED;
	}
}

int run_cli(FILE *in, FILE *out)
{
    // TO BE IMPLEMENTED
    if(out != stdout){
    	;
    }
    signal(SIGCHLD,sighandler);
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
    	if(red <= 0){
    		return -1;//invalid file
    	}
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
    		fprintf(out,"%s\n","The available commands are: \nhelp, quit, type(file_type), printer(printer_name, file_type)\nconversion(file_type1,file_type2,conversion_program[arg1 arg2...]), printers\njobs, print(file_name,[printer1,printer2,...]), cancel(job_number)\npause(job_number), resume(job_number), disable(job_number), enable(job_number)");
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
    			fprintf(out,"%s%i%s%s%s%s%s\n", "Printer: id=",p.id," name=",name," type=",cmd," Status=disable");
    			sf_cmd_ok();
    		}
    		else{//type do not exist
    			fprintf(out,"%s%s\n", "Unknown file type :",cmd);
    			sf_cmd_error("printer");
    		}
    	}
    	else if(strcmp(cmd,"conversion") == 0){
    		cmd = strtok(NULL," ");//type from
    		char *fname = cmd;
    		FILE_TYPE *f = find_type(fname);
    		if(f != NULL){
    			cmd = strtok(NULL," ");//type to
    			char *tname = cmd;
    			FILE_TYPE *t = find_type(cmd);
    			if(t != NULL){//both type exist
    				char *cd[99];
    				*cd = strtok(NULL," ");//args for conversion program and program name
    				int count =1;
    				char *temp = strtok(NULL," ");
    				while(temp != NULL){
    					*(cd+count) = temp;
    					temp = strtok(NULL," ");
    					count++;
    				}
    				cd[count] = '\0';
    				if(cd != NULL){
    					define_conversion(fname,tname,cd);
    				//how to parse this cmd to only show conversion program name
    				}
    				else{
    					sf_cmd_error("NOT ENOUGH ARGUMENT");
    				}
    			}
    			else{//type dont exist
    				fprintf(out,"%s%s\n", "Undeclared type:",cmd);
    				sf_cmd_error("type do not exist error");
    			}
    		}
    		else{//type dont exist
    			fprintf(out,"%s%s\n", "Undeclared type:",cmd);
    			sf_cmd_error("type do not exist error");
    		}
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
    			fprintf(out,"%s%i%s%s%s%s%s%s\n", "Printer: id=",parray[count].id," name=",parray[count].name," type=", parray[count].type.name ," Status=", sta);
    			count++;
    		}
    		sf_cmd_ok();
    	}
    	else if(strcmp(cmd,"jobs") == 0){
    		for(int i=0;i<MAX_JOBS;i++){
    			if(jobarray[i].filename != NULL){
    				char *status;
    				if(jobarray[i].status == JOB_CREATED){
    					status = "JOB_CREATED";
    				}
    				else if(jobarray[i].status == JOB_RUNNING){
    					status = "JOB_RUNNING";
    				}
    				else if(jobarray[i].status == JOB_PAUSED){
    					status = "JOB_PAUSED";
    				}
    				else if(jobarray[i].status == JOB_FINISHED){
    					status = "JOB_FINISHED";
    				}
    				else if(jobarray[i].status == JOB_ABORTED){
    					status = "JOB_ABORTED";
    				}
    				else{
    					status = "JOB_DELETED";
    				}
	    			fprintf(out,"%s%i%s%s%s%s%s%s\n", "JOB[",jobarray[i].id,"] :type=",jobarray[i].type.name," filename=",jobarray[i].filename," status=",status);
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
	    	if(*creaj.eligi == 0x0){//all printer can be used
	    		for(int i=0;i<MAX_PRINTERS;i++){
	    			if(parray[i].status == PRINTER_IDLE){
	    				CONVERSION **path = find_conversion_path(creaj.type.name,parray[i].type.name);
	    				//if a path exist or two type is the same and no conversion needed
						if(path != NULL || creaj.type.name == parray[i].type.name){
							processprint(path,parray[i],creaj,out);
							i = MAX_PRINTERS;//exit the loop
						}
	    			}
	    		}
	    	}
	    	else{//have a set of eligible printer
	    		for(int i=0;i<MAX_PRINTERS;i++){
	    			if(parray[i].status == PRINTER_IDLE){//if a idle printer is ready to be used
	    				int elic = 0;
						int notfound=0;
						//try to match the current printer with the eliglible printer of jobs
						while(*((creaj.eligi)+elic) != NULL && notfound == 0){
							if(*((creaj.eligi)+elic) == parray[i].name){
								notfound = -1;//found the eligible printer
							}
						}
						//printer match
						if(notfound == -1){
							CONVERSION **path = find_conversion_path(creaj.type.name,parray[i].type.name);
    						//if a path exist or two type is the same and no conversion needed
    						if(path != NULL || creaj.type.name == parray[i].type.name){
    							processprint(path,parray[i],creaj,out);
    							i = MAX_PRINTERS;//exit the loop
    						}
						}
	    			}
	    		}
	    	}
    	}
    	else if(strcmp(cmd,"cancel") == 0){
    		cmd = strtok(NULL," ");//job number
    		int pos = strtol(cmd,NULL,10);
    		JOB j = jobarray[pos];
    		if(pos < MAX_JOBS && pos >=0){
	    		if(j.filename != NULL){
	    			int s;
	    			if(j.status == JOB_RUNNING){//currently being processed
	    				s = killpg(j.gid,SIGTERM);
		    			if(s == 0){
	    					j.status = JOB_DELETED;
		    				sf_job_status(j.id,j.status);
		    				sf_cmd_ok();
		    				jobarray[pos].filename = NULL;
		    				jobcount--;
		    			}
		    		}
		    		else{//job is paused
		    			s = killpg(j.gid,SIGTERM);
		    			int r = killpg(j.gid,SIGCONT);//allow process to continue and respond to SIGTERM
		    			if(s == 0 && r == 0){
		    				j.status = JOB_DELETED;
		    				sf_job_status(j.id,j.status);
		    				sf_cmd_ok();
		    				jobarray[pos].filename = NULL;
		    				jobcount--;
		    			}
		    		}
	    		}
	    		else{
	    			sf_cmd_error("job not found");
	    		}
	    	}
	    	else{
	    		sf_cmd_error("invalid job number");
	    	}
    	}
    	else if(strcmp(cmd,"pause") == 0){
    		cmd = strtok(NULL," ");//job number
    		int pos = strtol(cmd,NULL,10);
    		JOB j = jobarray[pos];
    		if(pos < MAX_JOBS && pos >=0){
	    		if(j.filename != NULL){
	    			int s =killpg(j.gid,SIGSTOP);//pause job
	    			if(s == 0){
	    				j.status = JOB_PAUSED;
	    				sf_job_status(j.id,j.status);
	    				sf_cmd_ok();
	    			}
	    		}
	    		else{
	    			sf_cmd_error("job not found");
	    		}
	    	}
	    	else{
	    		sf_cmd_error("invalid job number");
	    	}
    	}
    	else if(strcmp(cmd,"resume") == 0){
    		cmd = strtok(NULL," ");//job number
    		int pos = strtol(cmd,NULL,10);
    		JOB j = jobarray[pos];
    		if(pos < MAX_JOBS && pos >=0){
	    		if(j.filename != NULL){
	    			int s =killpg(j.gid,SIGCONT);//resume job
	    			if(s == 0){
		    			j.status = JOB_RUNNING;
	    				sf_job_status(j.id,j.status);
	    				sf_cmd_ok();
	    			}
	    		}
	    		else{
	    			sf_cmd_error("job not found");
	    		}
	    	}
	    	else{
	    		sf_cmd_error("invalid job number");
	    	}
    	}
    	else if(strcmp(cmd,"disable") == 0){
    		cmd = strtok(NULL," ");
    		char *dname = cmd;//name

    		for(int i=0;i<MAX_PRINTERS;i++){
    			if(parray[i].name != NULL && strcmp(parray[i].name,dname) == 0){
    				parray[i].status = PRINTER_DISABLED;//disable printer
    				sf_printer_status(parray[i].name,parray[i].status);
    			}
    		}
    	}
    	else if(strcmp(cmd,"enable") == 0){
    		cmd = strtok(NULL," ");
    		char *ename = cmd;//name
    		int enter = 0;
    		int pos =0;
    		for(int i=0;i<MAX_PRINTERS;i++){
    			if(parray[i].name != NULL && strcmp(parray[i].name,ename) == 0){
    				parray[i].status = PRINTER_IDLE;//enable printer
    				sf_printer_status(parray[i].name,parray[i].status);
    				pos = i;
    				//scan through jobarray,find any job waiting for this printer
    				for(int j =0;j<MAX_JOBS;j++){
    					if(jobarray[j].filename != 0x0){
    						if(*jobarray[j].eligi == 0x0){//no eliglible printer, any printer can be use
	    						CONVERSION **path = find_conversion_path(jobarray[j].type.name,parray[i].type.name);
	    						//if a path exist or two type is the same and no conversion needed
	    						if(path != NULL || jobarray[j].type.name == parray[i].type.name){
	    							processprint(path,parray[i],jobarray[j],out);
	    							enter = -1;
	    							j = MAX_JOBS;//exit loop
	    							i = MAX_PRINTERS;//exit loop
	    						}
	    					}
	    					else{//eliglible printer exist
	    						int elic = 0;
	    						int notfound=0;
	    						//try to match the current printer with the eliglible printer of jobs
	    						while(*((jobarray[j].eligi)+elic) != NULL && notfound == 0){
	    							if(*((jobarray[j].eligi)+elic) == ename){
	    								notfound = -1;//found the eligible printer
	    							}
	    						}
	    						//printer match
	    						if(notfound == -1){
	    							CONVERSION **path = find_conversion_path(jobarray[j].type.name,parray[i].type.name);
		    						//if a path exist or two type is the same and no conversion needed
		    						if(path != NULL || jobarray[j].type.name == parray[i].type.name){
		    							processprint(path,parray[i],jobarray[j],out);
		    							enter = -1;
		    							j = MAX_JOBS;//exit loop
	    								i = MAX_PRINTERS;//exit loop

		    						}
	    						}
	    					}
    					}
    				}
    			}
    		}
    		if(enter == 0){//if did not enter pipeline
    			fprintf(out,"%s%i%s%s%s%s%s\n", "Printer: id=",parray[pos].id," name=",parray[pos].name," type=", parray[pos].type.name ," Status=idle");
    			sf_cmd_ok();
    		}
    	}
    	if(flag != -5){//flag got changed
    		for(int i =0;i<MAX_JOBS;i++){
    			int sid =jobarray[i].gid;
    			if(mpid[sid] == flag){//job found
    				jobarray[i].status = schan;
    				char *status;
					if(jobarray[i].status == JOB_RUNNING){
    					status = "JOB_RUNNING";
    				}
    				else if(jobarray[i].status == JOB_PAUSED){
    					status = "JOB_PAUSED";
    				}
    				else if(jobarray[i].status == JOB_FINISHED){
    					status = "JOB_FINISHED";
    					sf_job_finished(jobarray[i].id,jobarray[i].status);
    				}
    				else if(jobarray[i].status == JOB_ABORTED){
    					status = "JOB_ABORTED";
    					sf_job_aborted(jobarray[i].id,jobarray[i].status);
    				}
	    			fprintf(out,"%s%i%s%s%s%s%s%s\n", "JOB[",jobarray[i].id,"] :type=",jobarray[i].type.name," filename=",jobarray[i].filename," status=",status);
    				if(jobarray[i].status == JOB_ABORTED || jobarray[i].status == JOB_FINISHED){
    					sf_job_deleted(jobarray[i].id);
    					jobarray[i].filename = NULL;
    				}
					sf_cmd_ok();
    			}
    		}
    		flag = -5;
    		schan = -5;
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
int processprint(CONVERSION **path,PRINTER p,JOB j,FILE *out){
	int exitnum = 0;//change to -1 with failed operation
	char *filen = j.filename;
	int initfild = open(filen, O_RDONLY);
	int empfiled = open("empty.txt", O_RDWR);
	dup2(initfild,0);//change stdin of first process to be the file to be printed


	pid_t pid = fork();//master process
	if(path == NULL && pid == 0){//no conversion needed,master process enter
		j.status = JOB_RUNNING;
		sf_job_status(j.id,j.status);
		int filed = imp_connect_to_printer(p.name,p.type.name,PRINTER_NORMAL);
		p.status = PRINTER_BUSY;
		sf_printer_status(p.name,p.status);
		sf_job_started(j.id,p.name,pid,(char**)path);
		if(pid == 0){
			setpgid(pid,pid);//set pgid to be pid
			mpid[mpidcount] = pid;//store the pipeline process id into array
			j.gid = mpidcount;
			mpidcount++;

			pid_t cp = fork();//child of master process
			if(cp == 0){
				char *cmd = "/bin/cat";
				char *argv[2] ={"/bin/cat",NULL};
				dup2(filed,1);//change stdout of last process
				execvp(cmd,argv);
				fprintf(out,"%s%i%s%s%s%s%s\n", "JOB[",j.id,"] :type=",j.type.name," filename=",j.filename," status=running");
				sf_cmd_ok();
			}
			else{//waitpid to kill child of master process
				int chils;
				waitpid(cp,&chils,0);
				if(WIFSIGNALED(chils)){//terminated by signal
					exitnum = -1;
				}
				if(WIFEXITED(chils)){
					int s = WEXITSTATUS(chils);
					if(s != 0){//Exit status is nonzero
						exitnum = -1;
					}
				}
			}
		}
	}
	else if(pid == 0){//conversion pipeline happens here, master process enter
		j.status = JOB_RUNNING;
		sf_job_status(j.id,j.status);
		p.status = PRINTER_BUSY;
		sf_printer_status(p.name,p.status);
		sf_job_started(j.id,p.name,pid,(char**)path);
		if(pid == 0){//master process enter
			setpgid(pid,pid);//set pgid to be pid
			mpid[mpidcount] = pid;//store the pipeline process id into array
			j.gid = mpidcount;
			mpidcount++;

			void *cpidadr = malloc(99*sizeof(pid_t));
			pid_t cpid[99];
			int cc=0;

			int pathcount = 0;
			while(*(*(path+pathcount))->cmd_and_args != 0x0){//while exist more conversion
				cpid[cc] = fork();//child of master process

				if(cpid[cc] == 0){
					pathcount++;
					if(*(*(path+pathcount))->cmd_and_args != 0x0){//not last process
						char *scmd = *(*(path+pathcount))->cmd_and_args;
						char *temp = malloc(999);
						strcpy(temp,scmd);
						char *argm = strtok(temp," ");//first arg for execvp
						char *argv[3] = {argm,(*(path+pathcount))->from->name,(*(path+pathcount))->to->name};
						execvp(argm,argv);
						free(temp);
						if(STDOUT_FILENO == 1){
							dup2(STDOUT_FILENO,STDIN_FILENO);//stdout become next stdin
							dup2(empfiled,STDOUT_FILENO);//file descriptor 3 become next stdout
						}
						else{
							dup2(STDOUT_FILENO,STDIN_FILENO);//stdout become next stdin
							dup2(1,STDOUT_FILENO);//file descriptor 1 become next stdout
						}
					}
					else{//last process
						char *scmd = *(*(path+pathcount))->cmd_and_args;
						char *temp = malloc(999);
						strcpy(temp,scmd);
						char *argm = strtok(temp," ");//first arg for execvp
						int fild = imp_connect_to_printer(p.name,p.type.name,PRINTER_NORMAL);
						dup2(fild,1);//change stdout of last process
						char *argv[3] = {argm,(*(path+pathcount))->from->name,(*(path+pathcount))->to->name};
						execvp(argm,argv);
						free(temp);
					}
					cc++;
				}
				else{//master process use waitpid here to reap child process
					for(int i=0;i<cc;i++){
						int chils;
						waitpid(cpid[i],&chils,0);
						if(WIFSIGNALED(chils)){//terminated by signal
							exitnum = -1;
						}
						if(WIFEXITED(chils)){
							int s = WEXITSTATUS(chils);
							if(s != 0){//Exit status is nonzero
								exitnum = -1;
							}
						}
					}
				}
			}
			free(cpidadr);
		}
	}
	p.status = PRINTER_IDLE;
	sf_printer_status(p.name,p.status);
	return exitnum;
}