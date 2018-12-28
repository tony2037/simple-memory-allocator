#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "hw_malloc.h"


void *hw_malloc(size_t bytes)
{
    return NULL;
}

int hw_free(void *mem)
{
    return 0;
}

void *get_start_sbrk(void)
{
    return NULL;
}


void brkInit()
{
    start_brk = sbrk(1 << 16);
    heap_top = sbrk(0);
}


void BinInit(){
    void *ptr;
    ptr = heap_top;
    for(size_t i = 0; i < 11; ++i){
	Bin[i] = malloc(sizeof(struct Header *));
	Bin[i]->prev = (void *)Bin[i];
	Bin[i]->next = NULL;
    }

    size_t j = 0;
    do{
	size_t size = 0;
        ptr = (ptr - start_brk) / 2 + start_brk;
        size = ptr - start_brk;
        
	j = 0;
	while(!(size & 0x1)){
	    ++j;
	    size = size >> 1;
	}
	Bin[j - 5]->next = ptr;
        struct Header *temp = ptr;
        temp->prev = Bin[j - 5];
        temp->next = NULL;
        temp->chunk_info.PrevSize_AllcFlg = 0;
	unsigned int CurSize = ptr - start_brk;
        temp->chunk_info.CurSize_MFlg = (CurSize) << 1;	
    }while(j > 5);

    for(size_t i = 0; i < 11; ++i){
        struct Header *ptr = Bin[i];
	printf("Bin[%d]: \n", (int)i);
	while(ptr->next != NULL){
	    ptr = ptr->next;
	}
	    printf("PrevSize_AllcFlg: %d\n", ptr->chunk_info.PrevSize_AllcFlg);
	    printf("CurSize_MFlg: %d\n", ptr->chunk_info.CurSize_MFlg);
    }
}
