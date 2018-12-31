#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "hw_malloc.h"


void *hw_malloc(size_t bytes)
{
    unsigned int required_size = (unsigned int)bytes + (unsigned int)sizeof(struct Header);
    printf("hw_malloc required size: %d\n", required_size);

    int power = 0;
    while((1 << (power)) < required_size){++power;};
    printf("power required: %d\n", power);

    // Because Bin[0] = 2^4 Bin[power - 4] = 2^ power;
    power -= 4;

    // if Bin accessable give it, if not split
    while(Bin[power]->next == NULL){
        // no accessable, split
        split(power + 1);
    }
    if(Bin[power]->next != NULL){
        // accessable
	struct Header *ptr;
	ptr = Bin[power]->next;
	while(ptr->next != NULL){
	    ptr = (struct Header *)ptr->next;
	}
	// now ptr is the tail of the linked list
	((struct Header *)ptr->prev)->next = NULL;
        ptr->prev = (void *)ptr;
	ptr->next = NULL;
        ptr->chunk_info.PrevSize_AllcFlg = ptr->chunk_info.PrevSize_AllcFlg | 0x1;
	return ptr;
    }
    else
        return NULL;
}

int hw_free(void *mem)
{
    return 0;
}

void *get_start_sbrk(void)
{
    return start_brk;
}


void brkInit()
{
    start_brk = sbrk(1 << 16);
    heap_top = sbrk(0);
}


void BinInit(){
    void *ptr;
    ptr = heap_top;
    for(size_t i = 0; i < 12; ++i){
	Bin[i] = malloc(sizeof(struct Header*));
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
	Bin[j - 4]->next = ptr;
        struct Header *temp = ptr;
        temp->prev = Bin[j - 4];
        temp->next = NULL;
        temp->chunk_info.PrevSize_AllcFlg = 0;
	unsigned int CurSize = ptr - start_brk;
        temp->chunk_info.CurSize_MFlg = (CurSize) << 1;	
    }while(j > 4);
    
    for(size_t i = 0; i < 12; ++i){
        struct Header *ptr = Bin[i];
	printf("Bin[%d]: \n", (int)i);
	while(ptr->next != NULL){
	    ptr = ptr->next;
	}
	/*
	printf("PrevSize_AllcFlg: ");
	printfBinary(ptr->chunk_info.PrevSize_AllcFlg);
	printf("\n");
	printf("CurSize_MFlg    : ");
	printfBinary(ptr->chunk_info.CurSize_MFlg);
	printf("\n");
	*/
	printfHeader(ptr);
    }
}



void printfBinary(unsigned int bin)
{
    unsigned int i;
    for(i = 1 << 31; i > 0; i = i / 2){
        (bin & i)? printf("1") : printf("0");
    }
}



void split(size_t index)
{
    if((int)index == 0)
        return;

    if((int)index > 11){
        printf("Run out of memory \n");
	exit(0);
    }

    if(Bin[index]->next == NULL){
        // Need to split the next Bin
	split(index + 1);
    }
    
    if(Bin[index]->next != NULL){
        struct Header *hdr0, *hdr1;
	hdr0 = Bin[index]->next;
	Bin[index]->next = hdr0->next;
	void *hdr1_, *hdr0_;
	hdr0_ = (void *) hdr0;
        hdr1_ = hdr0_ + (1 << ((index + 4) - 1));
	hdr1 = (struct Header *)hdr1_;

        hdr0->chunk_info.PrevSize_AllcFlg = 1 << (index + 4); // (1 << ((index + 4) -1)) << 1;
        hdr0->chunk_info.CurSize_MFlg = 1 << (index + 4); // (1 << ((index + 4) -1)) << 1;
        hdr1->chunk_info.PrevSize_AllcFlg = 1 << (index + 4); // (1 << ((index + 4) -1)) << 1;
        hdr1->chunk_info.CurSize_MFlg = 1 << (index + 4); // (1 << ((index + 4) -1)) << 1;

	hdr0->next = (void *)hdr1;
        hdr1->next = NULL;
	hdr1->prev = (void *)hdr0;

	struct Header *tmp;
	tmp = Bin[index - 1];
	while(tmp->next != NULL){
	    tmp = (struct Header *)tmp->next;
	}
	tmp->next = hdr0;
	hdr0->prev = (void *)tmp;
	return;
    }

}



void printfHeader(struct Header *header)
{
    printf("Header address: %p\n", header);
    (header->prev == NULL)? printf("Header prev address NULL\n") : printf("Header prev address: %p\n", header->prev);
    (header->next == NULL)? printf("Header next address NULL\n") : printf("Header next address: %p\n", header->next);
    printf("Header chunk_info\n");
    printf("PrevSize_AllcFlg: ");
    printfBinary(header->chunk_info.PrevSize_AllcFlg);
    printf("\n");
    printf("CurSize_MFlg    : ");
    printfBinary(header->chunk_info.CurSize_MFlg);
    printf("\n");
    return;
}



