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


int interpretCMD(char cmd[],char rest[]){
    if(strcmp(cmd,"echo")==0){
        printf("%s\n",rest);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

int interpretIn(char input[]){
    input[strlen(input)-1]=NULL;
    int cmdEnd = strcspn(input," ");
    if(cmdEnd==strlen(input)){}
    char cmd[cmdEnd+1];
    int restL = strlen(input)-(cmdEnd+1); //-1 to eleminate \n , +1 to eleminate the ' '
    char rest[restL+1];
    strcpy(rest, input+cmdEnd+1);
    rest[restL]='\0';
    strncpy(cmd, input, cmdEnd);
    cmd[cmdEnd]='\0';
    interpretCMD(cmd,rest);
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
        
        interpretIn(i);
    }
    
    return 0;
}


