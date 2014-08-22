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
const int   MAX_DIR_SIZE = 1024;

//functions
void trimString(char *);
void strsub(const char *,char *,int,int);
int contSpace(const char *);
struct cmd* assambleStruct(char *);
char* grep(char*, FILE*);
char* interpretCMDstruct(struct cmd *);
void addHist(const char *);
char* getLastXNodes(int);
char* getHistory();
void clearHist();
void saveHist();

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
    struct histNode *prev;
    int id;
};

//misc
struct histNode *hist_top = NULL;
struct histNode *hist_last = NULL;
int id_counter = 0;
char* input;

char start_dir[ 1024 ];

int main(void){
    
    //save current directory;
    getcwd( start_dir,sizeof(start_dir)-1 );
    
    while(1){
        
        //init row with current directory + $ symbol
        char cwd[ MAX_DIR_SIZE ]; //memory for the path
        if( getcwd( cwd,sizeof(cwd)-1 ) == 0 ) return EXIT_FAILURE; //get current path
        printf("%s $ ", cwd); //print out the path
        
        
        //read input
        input = (char*)malloc(258); //reserve enough memory for the input
        fgets(input,258,stdin); //read input from console
        
        //handle input
        trimString(input);
        addHist(input);
        char* out = NULL;
        out = interpretCMDstruct( assambleStruct( input ) );
        
        if( (out != NULL)) {
            if( (*out != '\0') && (out[strlen(out)-1] != '\n') ) {
                printf("%s\n",out);
            }
            else {
                printf("%s",out);
            }
        }
        
        
        //free
        if(out) free(out);
        free(input);
        input = NULL;
    }
    
    return 0;
}

char* interpretCMDstruct(struct cmd *in){
    char* ret = NULL;
    
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
                saveHist();
                clearHist();
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
        else if (strcmp(in->cmd,"cd") == 0 ) {
            if(in->param != NULL){
                if( !contSpace(in->param) ){
                    if(chdir(in->param) == 0) {
                        ret = (char*) malloc(sizeof(char)*1);
                        strcpy(ret,"");
                    }
                    else {
                        ret = (char*) malloc( sizeof(char*) * (strlen(INVALID_DIR) +1 ) );
                        strcpy(ret,INVALID_DIR);
                    }
                }
                else {
                    ret = (char*) malloc( sizeof(char*) * (strlen(INVALID_ARGS) +1 ) );
                    strcpy(ret,INVALID_ARGS);
                }
            }
            else{
                ret =(char*) malloc( sizeof(char*) * (strlen(INVALID_ARGS) +1 ) );
                strcpy(ret, INVALID_ARGS);
            }
        } //change to param
        
        //handle "ls"
        else if ( strcmp(in->cmd,"ls") == 0 ) {
            if(in->param == NULL){
                DIR *dir = opendir ("."); //opens current directory in a dir-stream
                struct dirent *d = readdir(dir);
                char *t = malloc(1);
                strcpy(t,"");
                
                while(d) {
                    if (*d->d_name != '.') { //if(d_name doesn't begin with '.'){...}
                        t = realloc(t, strlen(t) + strlen(d->d_name) + 2);
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
            
            if(in->next == NULL){
                FILE* file = fopen(in->param, "r");

                if(file == NULL) {
                    ret = (char*) malloc( sizeof(char) * ( strlen(INVALID_DIR) +1 ) );
                    ret = strcpy(ret, INVALID_DIR);
                }
                else{
                    ret = grep(in->pattern, file);
                }
                
                fclose(file);
            }
            else{
                char* line = strtok(in->param, "\n");
                ret = (char*) calloc( 1, sizeof(char) );
                while( line ) {
                    if( strstr(line, in->pattern) ) {
                        ret = (char*) realloc( ret, strlen(ret) + strlen(line) + 2);
                        if(*ret) sprintf(ret,"%s\n%s", ret, line);
                        else sprintf(ret, "%s" ,line);
                    }
                    line = strtok(NULL, "\n");
                }
            }
        }
        
        //handle "history"
        else if ( strcmp(in->cmd, "history") == 0 ) {
            if (in->param == NULL) {
                ret = getHistory();
            }
            else {
                if ( strcmp(in->param ,"-c") == 0 ) {
                    clearHist();
                    ret = (char*) calloc( 1, sizeof(char) );
                }
                else {
                    ret = getLastXNodes( atoi(in->param) );
                }
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
struct cmd* assambleStruct(char* in){
    char *currentPart = NULL;
    struct cmd *currentStruct = NULL;
    struct cmd *nextStruct;
    
    char *ptrIn = NULL; //future pointer to the rest of the tokenized string
    currentPart = strtok_r(in,"|", &ptrIn);
    
    do {
        trimString(currentPart);
        
        //create next struct
        nextStruct = malloc(sizeof(struct cmd));
        nextStruct->param = NULL;
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
                trimString(subpart);
                
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
    char* ret = (char*)calloc( 1 , sizeof(char) );
    
    while( fgets( line, LINE_SIZE, file ) ){
        if( strstr(line, pattern) ) {
            ret = realloc( ret, strlen(ret) + strlen(line) + 1);
            strcat(ret,line);
        }
    }
    
    free(line);
    return ret;
}

//removes the white-spaces from the beginning and the end of the string and multiple spaces will be compressed to a single space
void trimString(char *in){
    char* tmp = NULL;
    int start;
    int end;
    // |0123456789|
    // |  123456 \|
    
    if(in != NULL){
        start = 0;
        end = (int)strlen(in)-1; //-1 or off-by-one
        
        while ( isspace( in[start]  ) && start  <strlen(in) ) start++;  //start++;   //count up "start" to the first non-space char
        while ( isspace( in[end]    ) && end    >0          ) end--;    //end--;     //count down "end" to the last non-space char
        
        tmp = (char*) calloc( (end+1 - start +1) , sizeof(char*)); //+1 -> \0
        strsub(in,tmp,start,end+1);
        strcpy(in, tmp);

        char* tmp2 = (char*) calloc(1, sizeof(char));
        for(int p = 0; tmp[p] != '\0' ; p++){
            if( !isspace(tmp[p]) ){
                int t = p;
                while( ( !isspace(tmp[++p]) ) && ( tmp[p]!='\0' ) ){}
                
                char* tmp3 = calloc(p-t+1,sizeof(char));
                strsub(tmp, tmp3, t, p);
                
                tmp2 = realloc(tmp2, strlen(tmp2) + 1 + p - t +1); //+1: space //+1: '\0' //p-t:=substring
                if(*tmp2) sprintf(tmp2, "%s %s",tmp2,tmp3);
                else sprintf(tmp2, "%s", tmp3);
                free(tmp3);
            }
        }
        free(tmp);
        strcpy(in, tmp2);
        free(tmp2);
        
    }
    
}

//returns the pointer to a substring from s to e (both inclusive) in a specific string
void strsub(const char *in,char *out,const int s, const int e){
    if (s <= e) {
        strncpy(out, in+s, e-s); // copy everything from the input to out
        out[e-s] = '\0'; //add the terminator
    }
}

//check weather a string contains a space character. Yes? -> return 1, No? -> return 0
int contSpace(const char* in){
    for(int i=0; i<strlen(in) ; i++){
        if(isspace( in[i] ) ) return 1;
    }
    return 0;
}

//adds an string to the history
void addHist(const char *in) {
    struct histNode *new = malloc( sizeof(struct histNode) ); //alloc new struct
    new->cmd = malloc( strlen(in)+1 );
    strcpy(new->cmd, in);
    new->id = id_counter++;
    new->next = NULL;
    new->prev = NULL;
    
    if(hist_last != NULL ) {
        hist_last->next = new;
        new->prev = hist_last;
    }
    else hist_top = new;
    
    hist_last = new;
}

//returns the last x entries in the history as a string [has to be free'd!]
char* getLastXNodes(int x) {
    char *ret = calloc(1, sizeof(char));
    
    if(x > 0) {
        struct histNode *tmp_head = hist_last;
        
        while(--x > 0 && (tmp_head->prev != NULL) ) {
            tmp_head = tmp_head->prev;
        }
        for (int i=0 ; tmp_head != NULL ; i++) {
            
            char* pline = (char*) malloc(sizeof(int) + 1 + strlen(tmp_head->cmd) +1);
            
            sprintf(pline, "%d %s", tmp_head->id, tmp_head->cmd);
            
            ret = realloc(ret, strlen(ret) + strlen(pline) + 2 );
            sprintf(ret, "%s%s\n", ret, pline);
            
            free(pline);
            
            tmp_head = tmp_head->next;
        }
        
        ret[ strlen(ret) ] = 0;
    }
    
    else {
        ret = (char*) realloc(ret, strlen(INVALID_ARGS) +1 );
        strcpy(ret, INVALID_ARGS);
    }
    return ret;
}

//frees the History and resets top, last und id-counter of the history
void clearHist() {
    id_counter = 0;
    while (hist_top != NULL) {
        free(hist_top->cmd);
        struct histNode* tmp = hist_top;
        hist_top = hist_top->next;
        free(tmp);
    }
    hist_last = NULL;
}

//returns the whole history as a string [has to be free'd]
char* getHistory() {
    char* ret = (char*) calloc(1, sizeof(char) );
    struct histNode *tmp_head = hist_top;
    
    while(tmp_head != NULL){
        
        char* pline = (char*) malloc(sizeof(int) + 1 + strlen(tmp_head->cmd) +2);
        sprintf(pline, "%d %s\n", tmp_head->id, tmp_head->cmd);
        
        ret = realloc(ret, strlen(ret) + strlen(pline) +1);
        sprintf(ret, "%s%s", ret,pline );
        free(pline);
        
        tmp_head = tmp_head->next;
        
    }
    
    return ret;
}


void saveHist() {
    chdir(start_dir);
    FILE* file = fopen("hhush.histfile", "w");
    
    //fputs( , file);
    
    fclose(file);
}
