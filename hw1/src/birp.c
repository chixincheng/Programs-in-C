/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"
#include "helper.h"

char *informat; //input format
char *outformat; //output format
int n= -1;
int r = -1;
int t= -1;
int tt=-1;
int z = -1;
int zz=-1;
int Z = -1;
int ZZ =-1;
int width;
int height;


int pgm_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_pgm(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int birp_to_birp(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

int pgm_to_ascii(FILE *in, FILE *out) { //this works
    int sucread = -1;
    int pos = 0;
    sucread = img_read_pgm(in,&width,&height,raster_data,sizeof(raster_data));
    if(sucread == 0){ //read pgm successfully
        for(int i=0;i<height;i++){
            for(int j=0;j<width;j++){
                if(raster_data[pos] > 191 && raster_data[pos] < 256){
                    fputc('@',out);
                }
                else if(raster_data[pos] > 127 && raster_data[pos] < 192){
                    fputc('*',out);
                }
                else if(raster_data[pos] > 63 && raster_data[pos] < 128){
                    fputc('.',out);
                }
                else if(raster_data[pos] > -1 && raster_data[pos] < 64){
                    fputc(' ',out);
                }
                pos++;
            }
            fputc('\n',out);
        }
        return 0;
    }
    else{
        return -1;
    }
}

int birp_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specifed will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere int the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int validargs(int argc, char **argv) {
    int pos = 0;
    informat = "birp";
    int inp = 00000002;
    outformat = "birp";
    int outp = 00000020;
    char *ptr = *(argv);
    int ofset = 1;
    if(pos == 0 && argc >= 1){
        ptr = *(argv+ofset);
        ofset++;
        argc--;
        pos++;
    }
    else{
        return -1; //no argument
    }
    while(stringcmp(ptr,'\0') != 0){ //while not end of argv
        if(pos == 1 && argc >= 1){
            if(stringcmp(ptr,"-h") == 0){// if -h is first argument
                global_options = 0x80000000;
                return 0;
            }
            else if(stringcmp(ptr,"-i") == 0){ // if -i is first argument
                ptr = *(argv+ofset);
                ofset++;
                argc--;
                if(stringcmp(ptr,"pgm") == 0 && argc >= 1)//if input format = pgm
                {
                    informat = "pgm";
                    inp = 00000001; //input exist
                }
                else if(stringcmp(ptr,"birp") == 0 && argc >= 1)//if input format = birp
                {
                    informat = "birp";
                    inp = 00000002; //input exist
                }
                else{
                    return -1;//-i not followed by prg or birp
                }
            }
            else if(stringcmp(ptr,"-o") ==0){//if -o is first argument, input format = birp by default
                ptr = *(argv+ofset);
                ofset++;
                argc--;
                if(stringcmp(ptr,"pgm") == 0 && argc >= 1){//if output format = pgm
                    outformat = "pgm";
                    outp = 00000010;
                }
                else if(stringcmp(ptr,"birp") == 0 && argc >= 1)//if output format = birp
                {
                    outformat = "birp";
                    outp = 00000020; //input exist
                }
                else if(stringcmp(ptr,"ascii") == 0 && argc >= 1)//if output format = ascii
                {
                    outformat = "ascii";
                    outp = 00000030; //input exist
                }
                else{
                    return -1; //-o not followed by prg or birp or ascii
                }
            }
            else if(stringcmp(ptr,"-n") == 0 && argc >= 1){ //if -n is first argument
                n = 00000100;
            }
            else if(stringcmp(ptr,"-r") == 0 && argc >= 1){ // if -r is first argument
                r = 00000400;
            }
            else if(stringcmp(ptr,"-t") == 0 && argc >= 1){ // if -t is first argument
                argc--;
                ptr = *(argv+ofset);
                ofset++;
                if(argc >= 1){
                    int temp = stringtoint(ptr);
                    if(temp >= 0 && temp <= 255){
                        t= 00000200;
                        tt = temp;
                    }
                    else{
                        return -1;
                    }
                }
            }
            else if(stringcmp(ptr,"z") == 0 && argc >= 1){// if -z is first argument
                argc--;
                ptr = *(argv+ofset);
                ofset++;
                if(argc >= 1){
                    int temp = stringtoint(ptr);
                    if(temp >= 0 && temp <= 16){
                        z = 00000300;
                        zz = -temp; //negative
                    }
                    else{
                        return -1;
                    }
                }
            }
            else if(stringcmp(ptr,"Z") == 0 && argc >= 1){ // if -Z is first argument
                argc--;
                ptr = *(argv+ofset);
                ofset++;
                if(argc >= 1){
                    int temp = stringtoint(ptr);
                    if(temp >= 0 && temp <= 16){
                        Z = 00000300;
                        ZZ = temp; // positve
                    }
                    else{
                        return -1;
                    }
                }
            }
            else{ //invalid first argument
                return -1;
            }
        }
        else if(pos == 2 && argc >= 1){ //check for second argument
            if(stringcmp(ptr,"-i") == 0){ // if -i is second argument
                ptr = *(argv+ofset);
                ofset++;
                argc--;
                if(stringcmp(ptr,"pgm") == 0 && argc >= 1)//if input format = pgm
                {
                    informat = "pgm";
                    inp = 00000001; //input exist
                }
                else if(stringcmp(ptr,"birp") == 0 && argc >= 1)//if input format = birp
                {
                    informat = "birp";
                    inp = 00000002; //input exist
                }
                else{
                    return -1;//-i not followed by prg or birp
                }
            }
            else if(stringcmp(ptr,"-o") ==0){//if -o is second argument, input format = birp by default
                ptr = *(argv+ofset);
                ofset++;
                argc--;
                if(stringcmp(ptr,"pgm") == 0 && argc >= 1){//if output format = pgm
                    outformat = "pgm";
                    outp = 00000010;
                }
                else if(stringcmp(ptr,"birp") == 0 && argc >= 1)//if output format = birp
                {
                    outformat = "birp";
                    outp = 00000020; //input exist
                }
                else if(stringcmp(ptr,"ascii") == 0 && argc >= 1)//if output format = ascii
                {
                    outformat = "ascii";
                    outp = 00000030; //input exist
                }
                else{
                    return -1; //-o not followed by prg or birp or ascii
                }
            }
            else if(stringcmp(ptr,"-n") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && n<0){
                    n = 00000100;
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-r") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && r<0){
                    r = 00000400;
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-t") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && t<0){
                    argc--;
                    ptr = *(argv+ofset);
                    ofset++;
                    if(argc >= 1){
                        int temp = stringtoint(ptr);
                        if(temp >= 0 && temp <= 16){
                            t = 00000200;
                            tt = temp;
                        }
                        else{
                            return -1;
                        }
                    }
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && z<0){
                    argc--;
                    ptr = *(argv+ofset);
                    ofset++;
                    if(argc >= 1){
                        int temp = stringtoint(ptr);
                        if(temp >= 0 && temp <= 16){
                            z = 00000300;
                            zz = temp;
                        }
                        else{
                            return -1;
                        }
                    }
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-Z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && Z<0){
                    argc--;
                    ptr = *(argv+ofset);
                    ofset++;
                    if(argc >= 1){
                        int temp = stringtoint(ptr);
                        if(temp >= 0 && temp <= 16){
                            Z = 00000300;
                            ZZ = temp;
                        }
                        else{
                            return -1;
                        }
                    }
                }
                else
                    return -1;
            }
            else
                return -1;
        }
        else{
            if(stringcmp(ptr,"-n") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && n<0){
                    n = 00000100;
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-r") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && r<0){
                    r = 00000400;
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-t") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && t<0){
                    argc--;
                    ptr = *(argv+ofset);
                    ofset++;
                    if(argc >= 1){
                        int temp = stringtoint(ptr);
                        if(temp >= 0 && temp <= 16){
                            t = 00000200;
                            tt = temp;
                        }
                        else{
                            return -1;
                        }
                    }
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && z<0){
                    argc--;
                    ptr = *(argv+ofset);
                    ofset++;
                    if(argc >= 1){
                        int temp = stringtoint(ptr);
                        if(temp >= 0 && temp <= 16){
                            z = 00000300;
                            zz = temp;
                        }
                        else{
                            return -1;
                        }
                    }
                }
                else
                    return -1;
            }
            else if(stringcmp(ptr,"-Z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && Z<0){
                    argc--;
                    ptr = *(argv+ofset);
                    ofset++;
                    if(argc >= 1){
                        int temp = stringtoint(ptr);
                        if(temp >= 0 && temp <= 16){
                            Z = 00000300;
                            ZZ = temp;
                        }
                        else{
                            return -1;
                        }
                    }
                }
                else
                    return -1;
            }
            else
                return -1;
        }
        pos++;
        argc--;
        ptr = *(argv+ofset);
        ofset++;
    }
    if(t > 0){
        global_options = global_options + tt;
        global_options = global_options >> 8;//shift right
        global_options = global_options + inp + outp + t;
    }
    else if(z > 0){
        global_options = global_options + zz;
        global_options = global_options >> 8;
        global_options = global_options + inp + outp + z;
    }
    else if(Z > 0){
        global_options = global_options + ZZ;
        global_options = global_options >> 8;
        global_options = global_options + inp + outp + Z;
    }
    else if(n > 0){
        global_options = global_options + inp + outp + n;
    }
    else if(r > 0){
        global_options = global_options + inp + outp + r;
    }
    return 0;
}