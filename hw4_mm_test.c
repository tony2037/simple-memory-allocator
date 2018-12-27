#include "lib/hw_malloc.h"
#include "hw4_mm_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int shell(){
    char **commands;
    char c;

    // commands mallocate & initilization
    commands = malloc(20* sizeof(char*));
    for(size_t i = 0; i < 20; ++i){
        commands[i] = malloc(64* sizeof(char));
	memset(commands[i], 0, 64);
    }

    // std input
    size_t commands_i = 0;
    while(!feof(stdin) && scanf("%c", &c)){
        if(c == *"\n"){
	    //printf("%s\n", commands[commands_i]);
	    ++commands_i;
	}
	else{
	    strcat(commands[commands_i], &c);
	}
    }
    --commands_i;

    // commands execution
    for(size_t i = 0; i < commands_i; ++i){
        char *command;
	char *param;
	command = strtok(commands[i], " ");
	param = strtok(NULL, " ");
	printf("[%d][%s][%s]\n", (int)i, command, param);

	if(!strcmp(command, "alloc")){
	    //printf("a\n");
	    void *ptr;
            ptr = hw_malloc(atoi(param));
	}
	else if(!strcmp(command, "free")){
	    //printf("f\n");
	}
	else if(!strcmp(command, "print")){
	    //printf("p\n");
	}
	else{
	    printf("No such command\n");
	}
    }

    return 0;
}

int main(int argc, char *argv[])
{
    printf("size of chunk_info_t: %d", (int)sizeof(struct chunk_info_t));
    printf("size of void*: %d", (int)sizeof(void*));
    printf("size of Header: %d\n", (int)sizeof(struct Header));
    shell();
    return 0;
}
