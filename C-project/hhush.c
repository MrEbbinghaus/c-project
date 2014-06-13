//
//  hhush.c
//  C-project
//
//  Created by Björn Ebbinghaus on 12.06.14.
//  Copyright (c) 2014 Björn Ebbinghaus. All rights reserved.
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

char* trimString(char[]);


int interpretCMD(char cmd[],char rest[]){
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

int interpretIn(char input[]){
    input[strlen(input)-1]=0;
    int cmdEnd = strcspn(input," ");
    if(cmdEnd==strlen(input)){}
    char cmd[cmdEnd+1];
    int paramL = strlen(input)-(cmdEnd+1); //-1 to eleminate \n , +1 to eleminate the ' '
    char params[paramL+1];
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

char* trimString(char in[]){
    printf("Input: %s",in);
    int start=0;
    int end=strlen(in)-1;
    for(start;isspace(in[start]);start++){}
    for(end;isspace(in[end]);end--){}
    char o[end-start+3];

    
    strncpy(o, in+start, end-start+3);
    o[end-start+1] = '\n';
    o[end-start+2] = '\0';
    printf("getrimmter String: %s",o);
    printf("Länge: %lu\n",strlen(o));
    return o;
}


