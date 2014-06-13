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

//functions
char* trimString(char *);
char* strsub(char *,int,int);
int interpretCMD(char *,char *);
int interpretIn(char *);


int interpretCMD(char *cmd,char *rest){
    if(strcmp(cmd,"echo")==0){
        printf("%s\n",rest);
        return EXIT_SUCCESS;
    }
    else if (strcmp(cmd,"date")==0){
        time_t time_raw = time(NULL);
        struct tm * date =localtime(&time_raw);
        printf("%s",asctime(date));
        return EXIT_SUCCESS;
    }
    else{printf("command not found\n");}

    return EXIT_FAILURE;
}

int interpretIn(char *input){
    long cmdEnd = strcspn(input," ");
    
    if(cmdEnd==strlen(input)){}
    char *cmd = malloc(cmdEnd+1);
    long paramL = strlen(input)-(cmdEnd+1); //-1 to eleminate \n , +1 to eleminate the ' '
    char *params = malloc(paramL+1);
    strcpy(params, input+cmdEnd+1);
    params[paramL]='\0';
    strncpy(cmd, input, cmdEnd);
    cmd[cmdEnd]='\0';
    interpretCMD(cmd,params);
    return 1;
}

int main(void)
{
    while(1){
        char cwd[256];
        getcwd(cwd, sizeof(cwd));
        fprintf(stdout,"%s $ ",cwd);
        char i[256];
        fgets(i,sizeof(i),stdin);
        
        interpretIn(trimString(i));
    }
    
    return 0;
}

char* trimString(char *in){
    int start=0;
    int end=(int)strlen(in)-1; //-1 because you don't want to count \n
    
    for(start;isspace(in[start]);start++){} //count up "start" to the first non-space char
    for(end;isspace(in[end]);end--){} //count down "end" to the last non-space char
    return strsub(in,start,(int)end);
    }

//returns the pointer to a substring from s to e (both inclusive) in a specific string
char* strsub(char *in,const int s, const int e){
    char *o = malloc(sizeof(char)*(e-s+2));//+1 off by one / +1 space for \0
    strncpy(o, in+s, e-s+1); // copy everything from the input to
    o[e-s+1] = '\0'; //add the terminator
    return o;
}


