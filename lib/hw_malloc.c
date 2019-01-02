#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include "hw_malloc.h"


void *hw_malloc(size_t bytes)
{
    unsigned int required_size = (unsigned int)bytes + (unsigned int)sizeof(struct Header);
    printf("hw_malloc required size: %d\n", required_size);

    if(required_size <= (1 << 15)){
	// Use heap to mallocate
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
    else{
        // use mmap to distribute
        void *ptr;
        ptr = mmap(NULL, (bytes + sizeof(struct Header)), PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
        struct Header *header = ptr;
        header->prev = header;
	header->next = NULL;
	header->chunk_info.PrevSize_AllcFlg = required_size << 1;
	header->chunk_info.CurSize_MFlg = (required_size << 1) + 1;


	return ptr;
    }

}

int hw_free(void *mem)
{
    if(!checkAddr(mem)){
        printf("This address is not mallocated\n");
	return 0;
    }
    else{
        struct Header *ptr = mem;
	if((ptr->chunk_info.PrevSize_AllcFlg & 0x1)){
	    // allocate address
	    putBin(mem);
	    mllocAddr.addrList[checkAddr(mem) - 1] = NULL;
	}
	else if((ptr->chunk_info.CurSize_MFlg & 0x1)){
	    // mmap address
	    size_t length = ptr->chunk_info.CurSize_MFlg >> 1;
	    munmap(mem, length);
	    mllocAddr.addrList[checkAddr(mem) - 1] = NULL;
	}



    }
    return 0;
}

void *get_start_sbrk(void)
{
    return start_brk;
}



void putBin(void *mem)
{
    // allocated address
    struct Header *ptr = mem;
    void *bd;
    size_t i = 0;
    int ifMerge = 0;
	    
    bd = mem + (ptr->chunk_info.CurSize_MFlg >> 1);
    while((1 << i) != (ptr->chunk_info.CurSize_MFlg >> 1)){i += 1;}
    i = i - 4;
	    
    struct Header *tmp;
    tmp = Bin[i];
    while(tmp->next != NULL){
        if(tmp == bd){
            ifMerge = 1;
	    break;
	}
	tmp = tmp->next;
    }

    if(ifMerge){
        // merge
	// split from the linked list of bin
	if(tmp->next == NULL){
	    // tmp is the tail
	    ((struct Header *)tmp->prev)->next = NULL;
	}
	else{
	    ((struct Header *)tmp->prev)->next = tmp->next;
	    ((struct Header *)tmp->next)->prev = tmp->prev;
	}
	// change the chunk_info
	ptr->chunk_info.PrevSize_AllcFlg = (ptr->chunk_info.PrevSize_AllcFlg & 0xFFFFFFFE) << 1;
	ptr->chunk_info.CurSize_MFlg = (ptr->chunk_info.CurSize_MFlg & 0xFFFFFFFE) << 1;
        
	// try to put back to bin
	putBin(ptr);
    }
    else{
        // Don't need to merge
	// change the chunk_info
	ptr->chunk_info.PrevSize_AllcFlg = (ptr->chunk_info.PrevSize_AllcFlg & 0xFFFFFFFE);
	ptr->chunk_info.CurSize_MFlg = (ptr->chunk_info.CurSize_MFlg & 0xFFFFFFFE);
        ptr->next = NULL;

        // put back to Bin	
        tmp = Bin[i];
        while(tmp->next != NULL){
	    tmp = tmp->next;
        }
	
	tmp->next = ptr;
	ptr->prev = tmp;
    }
            
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

	struct Header *tail;
	tail = Bin[index]->next;
	while(tail->next != NULL)
            tail = (struct Header *)tail->next;
	// now we got the tail
	((struct Header *)tail->prev)->next = NULL;
	tail->prev = tail;
	tail->next = NULL;
	hdr0 = tail;
	
	/*
	hdr0 = Bin[index]->next;
	Bin[index]->next = hdr0->next;
        */

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



void MallocAddrInit()
{
    mllocAddr.i = 0;
    for(size_t i = 0; i < 20; ++i){
        mllocAddr.addrList[i] = NULL;
    }
}



void printfAllocAddr()
{
    printf("The allocated address list:\n");
    for(size_t j = 0; j < mllocAddr.i; ++j){
        if(mllocAddr.addrList[j] != NULL){
	    //printf("%p\n", mllocAddr.addrList[j]);
	    printfHeader(mllocAddr.addrList[j]);
	}
    }
}



int checkAddr(void *mem)
{
    for(size_t j = 0; j < mllocAddr.i; ++j){
        if(mllocAddr.addrList[j] == mem)
            return j + 1;
    }
    return 0;
}




int printfBin(char *param)
{
    struct Header *ptr;
    if(!strcmp(param, "bin[0]")){
        ptr = Bin[0];
    }
    else if(!strcmp(param, "bin[1]")){
        ptr = Bin[1];
    }
    else if(!strcmp(param, "bin[2]")){
        ptr = Bin[2];
    }
    else if(!strcmp(param, "bin[3]")){
        ptr = Bin[3];
    }
    else if(!strcmp(param, "bin[4]")){
        ptr = Bin[4];
    }
    else if(!strcmp(param, "bin[5]")){
        ptr = Bin[5];
    }
    else if(!strcmp(param, "bin[6]")){
        ptr = Bin[6];
    }
    else if(!strcmp(param, "bin[7]")){
        ptr = Bin[7];
    }
    else if(!strcmp(param, "bin[8]")){
        ptr = Bin[8];
    }
    else if(!strcmp(param, "bin[9]")){
        ptr = Bin[9];
    }
    else if(!strcmp(param, "bin[10]")){
        ptr = Bin[10];
    }
    else 
        return 0;

    while(ptr->next != NULL){
	ptr = ptr->next;
        printfHeader(ptr);
    }

    return 1;
}



