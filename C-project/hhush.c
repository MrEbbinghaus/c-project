//
//  hhush.c
//  C-project for the
//
//  Created by Bjoern Ebbinghaus on 12.06.14.
//  Copyright (c) 2014 Bjoern Ebbinghaus. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>

//const
const char* INVALID_ARGS = "invalid arguments";
const char* INVALID_CMD = "command not found";
const char* INVALID_DIR = "no such file or directory";

//functions
void trimString(char *);
void strsub(char *,char *,int,int);
struct cmd* assambleStruct(char *);
char* interpretCMD(const char *,const char *);
const char* interpretCMDstruct(struct cmd *);

//structs
struct cmd {
    char *cmd;
    const char *param;
    char *pattern;
    struct cmd *next;
};

int main(void)
{
    while(1){
        //init row with current directory + $ symbol
        char cwd[257]; //memory for the path
        getcwd(cwd, sizeof(cwd)-1); //get current path
        cwd[sizeof(cwd)-1]='\0'; //add 0-terminator
        printf("%s $ ",cwd); //print out the path
        
        //read input
        char *input=(char*)malloc(sizeof(char)*257); //reserve enough memory for the input
        fgets(input,256,stdin); //read input from console
        
        //handle input
        trimString(input);
        const char* out = interpretCMDstruct(assambleStruct(input));
        printf("%s\n",out);
        
        //char* out =checkForPipes(input);
        //printf("%s",out);
        
        //free
        free(input);
    }
    
    return 0;
}

const char* interpretCMDstruct(struct cmd *in){
    const char* ret;
    
    //go to the first
    if (in->next != NULL) {
        const char *t = interpretCMDstruct(in->next);
        if(
           strcmp(t, INVALID_CMD)==0 ||
           strcmp(t, INVALID_DIR)==0 ||
           strcmp(t, INVALID_ARGS)==0
           ){
            ret = t;
            goto END;
        }
        else in->param = t;
    }

    
    //handle "echo"
    if(strcmp(in->cmd,"echo")==0){
        ret = in->param;
    }
    
    //handle "exit"
    else if (strcmp(in->cmd,"exit")==0){
        if(in->param == NULL) exit(0);
        else ret = INVALID_ARGS;
    }
    
    //handle "date"
    else if (strcmp(in->cmd,"date")==0){
        if(in->param == NULL){
            time_t time_raw = time(NULL); //get seconds since 1st Jan. 1970
            struct tm *date = localtime(&time_raw); //transform time_raw into a struct to hold all informations about the date
            char *d = asctime(date); //returns "date" as a formated string
            d[strlen(d)-1]=0; //remove the trailing '\n'
            ret = d;
        }
        else ret = INVALID_ARGS;
    }
    
    //handle "cd"
    else if (strcmp(in->cmd,"cd")==0){
        if(chdir(in->param) == 0) return "";
        else ret = INVALID_DIR;
    } //change to param
    
    //else:
END:
    free(in);
    if(ret == NULL) ret = INVALID_CMD;
    return ret;
}

struct cmd* assambleStruct(char *in){
    char *currentPart;
    struct cmd *currentStruct = NULL;
    struct cmd *nextStruct;
    
    char *ptrIn;
    currentPart = strtok_r(in,"|", &ptrIn);
    
    while(currentPart != NULL){
        trimString(currentPart);
        
        nextStruct = malloc(sizeof(struct cmd));
        nextStruct->next = currentStruct;
        currentStruct = nextStruct;
        
        //extract commando
        char *subpart = strtok(currentPart," ");
        currentStruct->cmd = subpart;
        
        if(subpart!=NULL){
            if(strcmp("grep", currentStruct->cmd)==0){
                
                subpart = strtok(NULL," ");
                currentStruct->pattern = subpart;
                
                subpart = strtok(NULL,"\0");
                currentStruct->param = subpart;
            }
            else{
                subpart = strtok(NULL,"\0");
                currentStruct->param = subpart;
            }
        }
        
        currentPart = strtok_r(ptrIn,"|",&ptrIn);
    }
    
    return currentStruct;
}

//removes the white-spaces from the beginning and the end of the string
void trimString(char *in){
    int *start = malloc(sizeof(int));
    *start = 0;
    int *end = malloc(sizeof(int));
    *end=(int)strlen(in)-1; //-1 because you don't want to count \n

    for(*start;isspace(in[*start]);(*start)++){} //count up "start" to the first non-space char
    for(*end;isspace(in[*end]);(*end)--){} //count down "end" to the last non-space char
    strsub(in,in,*start,*end+1);
    
    //free
    free(start);
    free(end);
}

//returns the pointer to a substring from s to e (both inclusive) in a specific string
void strsub(char *in,char *out,const int s, const int e){
    memcpy(out, in+s, e-s); // copy everything from the input to
    out[e-s] = '\0'; //add the terminator
}


