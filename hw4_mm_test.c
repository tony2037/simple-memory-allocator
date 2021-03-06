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
            mllocAddr.addrList[mllocAddr.i++] = ptr;

	    printf("Get the allocated address: %p\n", ptr);
	}
	else if(!strcmp(command, "free")){
	    //printf("f\n");
	    long int address = (long int)strtol(param, NULL, 0);
	    void *mem = (void *)address;
	    if(hw_free(mem)){
	        printf("free successfully\n");
	    }
	}
	else if(!strcmp(command, "print")){
	    //printf("p\n");
	    if(!strcmp(param, "mmap_alloc_list")){
                printfAllocAddr();
	    }
	    else{
	        printfBin(param);
	    }
	    
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
    
    brkInit();
    printf("start brk: %p\nheap  top: %p\n", start_brk, heap_top);

    // mllocAddr init
    MallocAddrInit();
    // Bin init
    BinInit();
    // shell 
    shell();

    // printf mallocated address list
    printfAllocAddr();
    return 0;
}
