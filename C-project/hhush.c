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
void trimString(char *);
void strsub(char *,int,int);
int interpretCMD(char *,char *);
int interpretIn(char *);

//interpret the commando with the parameters
int interpretCMD(char *cmd,char *param){
    //
    if(strcmp(cmd,"echo")==0){
        printf("%s\n",param);
        return EXIT_SUCCESS;
    }
    else if (strcmp(cmd,"exit")==0){
        exit(0);
    }
    //return the actuall date
    else if (strcmp(cmd,"date")==0){
        time_t time_raw = time(NULL); //get seconds since 1st Jan. 1970
        struct tm *date = localtime(&time_raw); //transform time_raw into a struct to hold all informations about the date
        printf("%s",asctime(date)); //prints "date" as a formated string
        return EXIT_SUCCESS;
    }
    //change directory
    else if (strcmp(cmd,"cd")==0){
        chdir(param); //change to param
    }
    else{printf("command not found\n");}

    return EXIT_FAILURE;
}

//split
int interpretIn(char *input){
    int cmdEnd = (int)strcspn(input," "); //search for the first space //-1 to miss the ' '
    
    char *cmd = malloc(cmdEnd);   //
    int paramL = (int)strlen(input)-cmdEnd; //-1 to eleminate \n
    char *params = malloc(paramL);
    
    strcpy(params, input+cmdEnd+1);
    params[paramL]='\0';
    
    memcpy(cmd, input, cmdEnd);
    
    interpretCMD(cmd,params);
    
    //free
    free(cmd);
    free(params);
    return 1;
}

int main(void)
{
    while(1){
        char cwd[257];
        getcwd(cwd, sizeof(cwd)-1); //get current directory
        cwd[sizeof(cwd)-1]='\0';
        printf("%s $ ",cwd); //print out the directory
        char *input=(char*)malloc(sizeof(char)*257);
        //int x = strlen(input);
        fgets(input,256,stdin);
        
        trimString(input);
        interpretIn(input);
        
        //free
        free(input);
    }
    
    return 0;
}

//removes the spaces from the beginning and the end of the string
void trimString(char *in){
    int *start = malloc(sizeof(int));
    *start = 0;
    int *end = malloc(sizeof(int));
    *end=(int)strlen(in)-1; //-1 because you don't want to count \n
    
    for(*start;isspace(in[*start]);(*start)++){} //count up "start" to the first non-space char
    for(*end;isspace(in[*end]);(*end)--){} //count down "end" to the last non-space char
    strsub(in,*start,*end);
    
    //free
    free(start);
    free(end);
}

//returns the pointer to a substring from s to e (both inclusive) in a specific string
void strsub(char *in,const int s, const int e){
    memcpy(in, in+s, e-s+1); // copy everything from the input to
    in[e-s+1] = '\0'; //add the terminator
}


