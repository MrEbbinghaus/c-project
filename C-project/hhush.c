//
//  hhush.c
//  C-project for the module "Computer Science 2"
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


//constants
const char* INVALID_ARGS = "invalid arguments";
const char* INVALID_CMD = "command not found";
const char* INVALID_DIR = "no such file or directory";

//functions
void trimString(char *);
void strsub(const char *,char *,int,int);
struct cmd* assambleStruct(char *);
char* grep(char*, FILE*);
char* interpretCMDstruct(struct cmd *);
void addHist(const char *);
char* getLastXNodes(long);
char* getHistory();
void clearHist();

//structs
struct cmd {
    char *cmd;
    char *param;
    char *pattern;
    struct cmd *next;
};

struct histNode {
    char* cmd;
    struct histNode *next;
    int id;
};

//
struct histNode *top;
char* input;


int main(void){
    
    while(1){
        
        //init row with current directory + $ symbol
        char cwd[257]; //memory for the path
        if( getcwd( cwd,sizeof(cwd)-1 ) == 0 ) return EXIT_FAILURE; //get current path
        printf("%s $ ", cwd); //print out the path
        
        
        //read input
        input = (char*)malloc(259); //reserve enough memory for the input
        fgets(input,258,stdin); //read input from console
        
        //handle input
        trimString(input);
        //addHist(input);echo abc |
        char* out = interpretCMDstruct( assambleStruct( input ) );
        
        if( (out != NULL)) {
            if( out[strlen(out)-1] != '\n' && (*out != '\0') ) {
                printf("%s\n",out);
            }
            else {
                printf("%s",out);
            }
        }
        
        
        //free
        if(out) free(out);
        free(input);
    }
    
    return 0;
}

char* interpretCMDstruct(struct cmd *in){
    char* ret;
    
    // go recursive to the last element
    if (in->next != NULL) {
        char *t = interpretCMDstruct(in->next);
        if(
           strcmp(t, INVALID_CMD)==0 ||
           strcmp(t, INVALID_DIR)==0 ||
           strcmp(t, INVALID_ARGS)==0
           
           ){
            ret = (char*)t;
            goto END;
        }
        else in->param = t;
    }
    
    
    //switch / case
    if(in->cmd != NULL){
        
        //handle "echo"
        if ( strcmp( in->cmd, "echo" ) == 0 ){
            if(in->param != NULL){
                ret = malloc( strlen(in->param) +1);
                strcpy(ret, in->param);
            }
            else{
                ret = malloc(1);
                strcpy(ret, "");
            }
        }
        
        //handle "exit"
        else if ( strcmp(in->cmd,"exit") == 0 ) {
            if(in->param == 0) {
                free(in);
                free(input);
                exit(0);
            }
            else {
                ret = (char*) malloc( sizeof(char*) * ( strlen(INVALID_ARGS) +1 ) );
                strcpy(ret, INVALID_ARGS);
            }
        }
        
        //handle "date"
        else if ( strcmp(in->cmd,"date") == 0 ) {
            if(in->param == NULL) {
                time_t time_raw = time( NULL );           //get seconds since 1st Jan. 1970
                struct tm *date = localtime( &time_raw ); //transform time_raw into a struct to hold all informations about the date
                char *d = asctime( date );                //returns "date" as a formated string
                d[strlen(d)-1] = 0;                       //remove the trailing '\n'
                ret = malloc( strlen(d) +1);
                ret = strcpy( ret, d );
            }
            else {
                ret =(char*) malloc( sizeof(char*) * (strlen(INVALID_ARGS) +1 ) );
                strcpy(ret, INVALID_ARGS);
            }
        }
        
        //handle "cd"
        else if (strcmp(in->cmd,"cd")==0) {
            if(in->param != NULL){
                if(chdir(in->param) == 0) {
                    ret = malloc(1);
                    strcpy(ret,"");
                }
                else {
                    ret = (char*) malloc( sizeof(char*) * (strlen(INVALID_DIR) +1 ) );
                    strcpy(ret,INVALID_DIR);
                }
            }
            else{
                ret =(char*) malloc( sizeof(char*) * (strlen(INVALID_ARGS) +1 ) );
                strcpy(ret, INVALID_ARGS);
            }
        } //change to param
        
        //handle "ls"
        else if ( strcmp(in->cmd,"ls")==0 ) {
            if(in->param == NULL){
                DIR *dir = opendir ("."); //opens current directory in a dir-stream
                struct dirent *d = readdir(dir);
                char *t = malloc(1);
                strcpy(t,"");
                
                while(d) {
                    if (*d->d_name != '.') { //if(d_name doesn't begin with '.'){...}
                        t = realloc(t, strlen(t) + strlen(d->d_name) + 1);
                        strcat(t, d->d_name);
                        strcat(t, "\n");
                    }
                    d = readdir(dir); //get the next dir
                }
                closedir(dir);
                t[strlen(t)-1] = 0; //remove the last \n
                ret = (char*) malloc( strlen(t) + 1);
                strcpy(ret, t);
                free(t);
            }
            else{
                ret = (char*) malloc( sizeof(char*) * (strlen(INVALID_ARGS) +1 ) );
                ret = strcpy(ret, INVALID_ARGS);
            }
        }
        
        //handle "grep"
        else if (strcmp(in->cmd,"grep") == 0 ) {
            FILE *file = fopen(in->param, "r");
            if(file == NULL) {
                ret = (char*) malloc( sizeof(char*) * (strlen(INVALID_DIR)+1) );
                strcpy(ret, INVALID_DIR);
            }
            else{
                ret = grep(in->pattern, file);
            }
            fclose(file);
        }
        
        //handle "history"
        else if ( strcmp(in->cmd, "history") == 0 ) {
            if (in->param == NULL) {
                ret = getHistory();
            }
            else {
                if ( strcmp(in->param ,"-c") == 0 ) {
                    clearHist();
                    ret = (char*) malloc( 1 * sizeof(char) );
                    ret = '\0';
                }
                else {
                    ret = getLastXNodes( atol(in->param) );
                }
                //strtol for parsing String -> Int
            }
        }
        
        else {
            ret = (char*) malloc( sizeof(char*) * (strlen(INVALID_CMD) +1 ) );
            strcpy(ret, INVALID_CMD);
        }
    }
    
    //else:
END:
    if(in->param)   free(in->param);
    if(in->pattern) free(in->pattern);
    if(in)          free(in);
    return ret;
}


/*
 *  cuts down the input string and assambles an "cmd"-struct for further use
 */
struct cmd* assambleStruct(char *in){
    char *currentPart;
    struct cmd *currentStruct = NULL;
    struct cmd *nextStruct;
    
    char *ptrIn;
    currentPart = strtok_r(in,"|", &ptrIn);
    
    do {
        trimString(currentPart);
        
        nextStruct = malloc(sizeof(struct cmd));
        nextStruct->pattern = NULL;
        nextStruct->next = currentStruct;
        currentStruct = nextStruct;
        
        //extract commando
        char *subpart = strtok(currentPart," ");
        currentStruct->cmd = subpart;
        
        //extract params
        if (subpart!=NULL) {
            if (strcmp( "grep", currentStruct->cmd)==0) {
                
                subpart = strtok(NULL," ");
                
                char* tmp = NULL;
                if(subpart) {
                    tmp = malloc( strlen(subpart) +1 );
                    strcpy(tmp,subpart); //subpart --/--> NULL
                }
                currentStruct->pattern = tmp;
            }
            
            
            subpart = strtok(NULL,"\0");
            trimString(subpart);
            
            char* tmp = NULL;
            if(subpart) {
                tmp = malloc( strlen(subpart) +1);
                strcpy(tmp,subpart); //subpart --/--> NULL
            }
            currentStruct->param = tmp;
        }
        
        currentPart = strtok_r(ptrIn,"|",&ptrIn); //currentPart = next part of the input string
    } while (currentPart != NULL);
    
    return currentStruct;
}

char* grep(char* pattern, FILE* file){
    const int LINE_SIZE = 4096;
    char* line = (char*)malloc( LINE_SIZE );
    char* ret = (char*)malloc( 1 * sizeof(char) );
    ret[0] = 0;
    
    while( fgets( line, LINE_SIZE, file ) ){
        if( strstr(line, pattern) ) {
            ret = realloc( ret, strlen(ret) + strlen(line) );
            strcat(ret,line);
        }
    }
    
    free(line);
    return ret;
}

//removes the white-spaces from the beginning and the end of the string
void trimString(char *in){
    char* ret;
    int start;
    int end;
    // |0123456789|
    // |  123456 \|
    
    if(in != NULL){
        start = 0;
        end = (int)strlen(in)-1; //-1 or off-by-one
        
        while ( isspace( in[start]  ) && start  <strlen(in) ) start++;  //start++;   //count up "start" to the first non-space char
        while ( isspace( in[end]    ) && end    >0          ) end--;    //end--;     //count down "end" to the last non-space char
        
        ret = (char*) malloc(sizeof(char*) * (end+1 - start +1) ); //+1 -> \0
        strsub(in,ret,start,end+1);
        
        strcpy(in, ret);
        free(ret);
    }
    
}

//returns the pointer to a substring from s to e (both inclusive) in a specific string
void strsub(const char *in,char *out,const int s, const int e){
    if (s <= e) {
        strncpy(out, in+s, e-s); // copy everything from the input to out
        out[e-s] = '\0'; //add the terminator
    }
}

void addHist(const char *in) {
    struct histNode *new = malloc(sizeof(struct histNode)); //alloc new struct
    new->cmd = malloc(strlen(in)+1);
    strcpy(new->cmd, in);
    new->next = top;
    top = new;
}

char* getLastXNodes(long x){
    char *out = "";
    if(x > 0) {
        struct histNode *tmp_head = top;
        for (int i=0 ; i<x && tmp_head != NULL ; i++) {
            //out = concat(out, tmp_head->cmd);
            tmp_head = tmp_head->next;
        }
        out[strlen(out)-1] = 0;
    }
    else out = (char*)INVALID_ARGS;
    return out;
}

void clearHist(){
    while (top != NULL) {
        free(top);
        top = top->next;
    }
}

char* getHistory(){
    char buffer[15];
    int c;
    char *out = "";
    struct histNode *tmp_head = top;
    
    c = 0;
    while(tmp_head != NULL){
        sprintf(buffer, "%d: ", c);
        //out = concat(out, buffer);
        //out = concat(out, tmp_head->cmd);
        //out = concat(out, "\n");
        tmp_head = tmp_head->next;
        c++;
    }
    out[strlen(out)-1] = 0;
    return out;
}
