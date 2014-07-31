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


//const
const char* INVALID_ARGS = "invalid arguments";
const char* INVALID_CMD = "command not found";
const char* INVALID_DIR = "no such file or directory";

//functions
char* trimString(char *);
void strsub(const char *,char *,int,int);
struct cmd* assambleStruct(char *);
char* grep(char*, FILE*);
char* interpretCMD(const char *,const char *);
char* interpretCMDstruct(struct cmd *);
char* concat(char *,char *);
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
};

struct histNode *top;
char* input;


int main(void){
    while(1){
        //init row with current directory + $ symbol
        char cwd[256]; //memory for the path
        if(getcwd(cwd,sizeof(cwd)-1) == 0) return EXIT_FAILURE; //get current path
        printf("%s $ ", cwd); //print out the path
        
        
        //read input
        input=(char*)malloc(sizeof(char)*258); //reserve enough memory for the input
        fgets(input,258,stdin); //read input from console
        
        //handle input
        trimString(input);
        //addHist(input);echo abc |
        char* out = interpretCMDstruct( assambleStruct( input ) );
        if(out != '\0') printf("%s\n",out);
        
        //free
        free(out);
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

    if(in->cmd != NULL){
        //handle "echo"
        if ( strcmp( in->cmd, "echo" ) ==0 ){
            ret = malloc(sizeof(in->param));
            strcpy(ret, in->param);
        }
        
        //handle "exit"
        else if (strcmp(in->cmd,"exit")==0){
            if(in->param == 0) {
                free(in);
                free(input);
                exit(0);
            }
            else {
                ret = malloc(sizeof(INVALID_ARGS));
                strcpy(ret, INVALID_ARGS);
            }
        }
        
        //handle "date"
        else if (strcmp(in->cmd,"date")==0){
            if(in->param == NULL){
                time_t time_raw = time(NULL); //get seconds since 1st Jan. 1970
                struct tm *date = localtime(&time_raw); //transform time_raw into a struct to hold all informations about the date
                char *d = asctime(date); //returns "date" as a formated string
                d[strlen(d)-1]=0; //remove the trailing '\n'
                ret = malloc(sizeof(d));
                ret = strcpy(ret,d);
            }
            else {
                ret = malloc(sizeof(INVALID_ARGS));
                strcpy(ret,INVALID_ARGS);
            }
        }
        
        //handle "cd"
        else if (strcmp(in->cmd,"cd")==0){
            if(chdir(in->param) == 0){
                ret = malloc(1);
                ret = '\0';
            }
            else {
                ret = malloc(sizeof(INVALID_DIR));
                strcpy(ret,INVALID_DIR);
            }
        } //change to param
        
        //handle "ls"
        else if (strcmp(in->cmd,"ls")==0) {
            DIR *dir = opendir ("."); //opens current directory in a dir-stream
            char *t = "";
            while (dir) {
                struct dirent *d = readdir(dir); //get the next dir
                if (d) {
                    if (*d->d_name != '.') { //if(d_name doesn't begin with '.'){...}
                        t = realloc(t,sizeof(t)+sizeof(d->d_name)+1);
                        strcat(t, d->d_name);
                        strcat(t, "\n");
                    }
                }
                else break;
            }
            t[strlen(t)-1]=0; //remove the last \n
            ret = malloc(sizeof(t));
            strcpy(ret, t);
            free(t); //free t
        }
        
        //handle "grep"
        else if (strcmp(in->cmd,"grep") == 0){
            FILE *file = fopen(in->param, "r");
            grep(in->pattern, file);
        }
        
        //handle "history"
        else if ( strcmp(in->cmd, "history" ) == 0) {
            if (in->param == NULL) {
                ret = getHistory();
            }
            else {
                if ( strcmp( in->param ,"-c" ) == 0 ) {
                    clearHist();
                    ret = malloc(1);
                    ret = '\0';
                }
                else {
                    ret = getLastXNodes( atol(in->param) );
                }
                //strtol for parsing String -> Int
            }
        }
        else {
            ret = malloc(sizeof(INVALID_CMD));
            strcpy(ret, INVALID_CMD);
        }
    }
    
    //else:
END:
    free(in->param);
    free(in);
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
        nextStruct->next = currentStruct;
        currentStruct = nextStruct;
        
        //extract commando
        char *subpart = strtok(currentPart," ");
        currentStruct->cmd = subpart;
        
        //extract params
        if (subpart!=NULL) {
            if (strcmp( "grep", currentStruct->cmd)==0) {
                
                subpart = strtok(NULL," ");
                trimString(subpart);
                currentStruct->pattern = subpart;
                
            }
            subpart = strtok(NULL,"\0");
            trimString(subpart);
            
            char* tmp = NULL;
            if(subpart) {
                tmp = malloc(sizeof(subpart));
                strcpy(tmp,subpart); //subpart --/--> NULL
            }
            currentStruct->param = tmp;
        }
        
        currentPart = strtok_r(ptrIn,"|",&ptrIn); //currentPart = next part of the input string
    } while (currentPart != NULL);
    
    return currentStruct;
}


char* grep(char* pattern, FILE* file){
    char* ret = malloc(1);
    char* line = malloc(128);
    size_t line_size = sizeof(line);
    while( getline(&line, &line_size, file) > 0 ) {
        if(strstr(pattern, line) ){
            ret = realloc(ret, sizeof(ret)+sizeof(line));
            strcat(ret, line);
        }
    }
    return ret;
}

//removes the white-spaces from the beginning and the end of the string
char* trimString(char *in){
    char* ret;
    if(in != NULL){
        int start = 0;
        int end;
        end=(int)strlen(in)-1; //-1 because you don't want to count \n

        for (; isspace(in[start]); start++) {} //count up "start" to the first non-space char
        for (; isspace(in[end]); end--) {} //count down "end" to the last non-space char
        
        ret = malloc(end+1-start);
        strsub(in,ret,start,end+1);
        strcpy(in, ret);
        free(ret);
    }
    return NULL;
}

//returns the pointer to a substring from s to e (both inclusive) in a specific string
void strsub(const char *in,char *out,const int s, const int e){
    if (s <= e) {
        memcpy(out, in+s, e-s); // copy everything from the input to out
        out[e-s] = '\0'; //add the terminator
    }
}

char* concat(char *a, char *b) {
    char *r = malloc(strlen(a)+strlen(b)+1);//add 1 for \0
    sprintf(r, "%s%s", a, b);
    return r;
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
            out = concat(out, tmp_head->cmd);
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
        out = concat(out, buffer);
        out = concat(out, tmp_head->cmd);
        out = concat(out, "\n");
        tmp_head = tmp_head->next;
        c++;
    }
    out[strlen(out)-1] = 0;
    return out;
}

