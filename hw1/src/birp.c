/*
 * BIRP: Binary decision diagram Image RePresentation
 */

#include "image.h"
#include "bdd.h"
#include "const.h"
#include "debug.h"
#include "helper.h"

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

int pgm_to_ascii(FILE *in, FILE *out) {
    // TO BE IMPLEMENTED
    return -1;
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
    char *informat;
    informat = "birp";
    int inp = 00000002;
    char *outformat;
    outformat = "birp";
    int outp = 00000020;
    int n= -1;
    int r = -1;
    int t= -1;
    int tt=-1;
    int z = -1;
    int zz=-1;
    int Z = -1;
    int ZZ =-1;
    if(pos == 0 && argc >= 1){
        *argv = *(argv+1);
        argc--;
        pos++;
    }
    else{
        return -1; //no argument
    }
    while(stringcmp(*argv,"\0") != 0){ //while not end of argv
        if(pos == 1 && argc >= 1){
            if(stringcmp(*argv,"-h") == 0){// if -h is first argument
                global_options = 0x80000000;
                return 0;
            }
            else if(stringcmp(*argv,"-i") == 0){ // if -i is first argument
                *argv = *(argv+1);
                argc--;
                if(stringcmp(*argv,"pgm") == 0 && argc >= 1)//if input format = pgm
                {
                    informat = "pgm";
                    inp = 00000001; //input exist
                }
                else if(stringcmp(*argv,"birp") == 0 && argc >= 1)//if input format = birp
                {
                    informat = "birp";
                    inp = 00000002; //input exist
                }
                else{
                    return -1;//-i not followed by prg or birp
                }
            }
            else if(stringcmp(*argv,"-o") ==0){//if -o is first argument, input format = birp by default
                *argv = *(argv+1);
                argc--;
                if(stringcmp(*argv,"pgm") == 0 && argc >= 1){//if output format = pgm
                    outformat = "pgm";
                    outp = 00000010;
                }
                else if(stringcmp(*argv,"birp") == 0 && argc >= 1)//if output format = birp
                {
                    outformat = "birp";
                    outp = 00000020; //input exist
                }
                else if(stringcmp(*argv,"ascii") == 0 && argc >= 1)//if output format = ascii
                {
                    outformat = "ascii";
                    outp = 00000030; //input exist
                }
                else{
                    return -1; //-o not followed by prg or birp or ascii
                }
            }
            else if(stringcmp(*argv,"-n") == 0 && argc >= 1){ //if -n is first argument
                n = 00000100;
            }
            else if(stringcmp(*argv,"-r") == 0 && argc >= 1){ // if -r is first argument
                r = 00000400;
            }
            else if(stringcmp(*argv,"-t") == 0 && argc >= 1){ // if -t is first argument
                argc--;
                *argv = *(argv+1);
                if(argc >= 1){
                    int temp = stringtoint(*argv);
                    if(temp >= 0 && temp <= 255){
                        t= 00000200;
                        tt = temp;
                    }
                    else{
                        return -1;
                    }
                }
            }
            else if(stringcmp(*argv,"z") == 0 && argc >= 1){// if -z is first argument
                argc--;
                *argv = *(argv+1);
                if(argc >= 1){
                    int temp = stringtoint(*argv);
                    if(temp >= 0 && temp <= 16){
                        z = 00000300;
                        zz = -temp; //negative
                    }
                    else{
                        return -1;
                    }
                }
            }
            else if(stringcmp(*argv,"Z") == 0 && argc >= 1){ // if -Z is first argument
                argc--;
                *argv = *(argv+1);
                if(argc >= 1){
                    int temp = stringtoint(*argv);
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
            if(stringcmp(*argv,"-i") == 0){ // if -i is second argument
                *argv = *(argv+1);
                argc--;
                if(stringcmp(*argv,"pgm") == 0 && argc >= 1)//if input format = pgm
                {
                    informat = "pgm";
                    inp = 00000001; //input exist
                }
                else if(stringcmp(*argv,"birp") == 0 && argc >= 1)//if input format = birp
                {
                    informat = "birp";
                    inp = 00000002; //input exist
                }
                else{
                    return -1;//-i not followed by prg or birp
                }
            }
            else if(stringcmp(*argv,"-o") ==0){//if -o is second argument, input format = birp by default
                *argv = *(argv+1);
                argc--;
                if(stringcmp(*argv,"pgm") == 0 && argc >= 1){//if output format = pgm
                    outformat = "pgm";
                    outp = 00000010;
                }
                else if(stringcmp(*argv,"birp") == 0 && argc >= 1)//if output format = birp
                {
                    outformat = "birp";
                    outp = 00000020; //input exist
                }
                else if(stringcmp(*argv,"ascii") == 0 && argc >= 1)//if output format = ascii
                {
                    outformat = "ascii";
                    outp = 00000030; //input exist
                }
                else{
                    return -1; //-o not followed by prg or birp or ascii
                }
            }
            else if(stringcmp(*argv,"-n") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && n<0){
                    n = 00000100;
                }
                else
                    return -1;
            }
            else if(stringcmp(*argv,"-r") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && r<0){
                    r = 00000400;
                }
                else
                    return -1;
            }
            else if(stringcmp(*argv,"-t") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && t<0){
                    argc--;
                    *argv = *(argv+1);
                    if(argc >= 1){
                        int temp = stringtoint(*argv);
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
            else if(stringcmp(*argv,"-z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && z<0){
                    argc--;
                    *argv = *(argv+1);
                    if(argc >= 1){
                        int temp = stringtoint(*argv);
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
            else if(stringcmp(*argv,"-Z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && Z<0){
                    argc--;
                    *argv = *(argv+1);
                    if(argc >= 1){
                        int temp = stringtoint(*argv);
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
            if(stringcmp(*argv,"-n") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && n<0){
                    n = 00000100;
                }
                else
                    return -1;
            }
            else if(stringcmp(*argv,"-r") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && r<0){
                    r = 00000400;
                }
                else
                    return -1;
            }
            else if(stringcmp(*argv,"-t") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && t<0){
                    argc--;
                    *argv = *(argv+1);
                    if(argc >= 1){
                        int temp = stringtoint(*argv);
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
            else if(stringcmp(*argv,"-z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && z<0){
                    argc--;
                    *argv = *(argv+1);
                    if(argc >= 1){
                        int temp = stringtoint(*argv);
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
            else if(stringcmp(*argv,"-Z") == 0 && argc >= 1){
                if(stringcmp(informat,"birp") == 0 && stringcmp(outformat,"birp") == 0 && Z<0){
                    argc--;
                    *argv = *(argv+1);
                    if(argc >= 1){
                        int temp = stringtoint(*argv);
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
        *argv = *(argv+1);
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