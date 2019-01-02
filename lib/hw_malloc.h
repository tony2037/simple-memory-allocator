#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stdlib.h>

void *start_brk;
void *heap_top;


struct MallocAddr{
    void *addrList[20];
    size_t i;
} mllocAddr;

struct chunk_info_t{
    unsigned int PrevSize_AllcFlg; // {Previous Size}[31]{Allocate Flag}[1]
    unsigned int CurSize_MFlg; // {Current Size}[31]{MMAP Flag}[1]
};

struct Header{
    void *prev; // linked list
    void *next;
    struct chunk_info_t chunk_info;
};

struct Header *Bin[12];

void *hw_malloc(size_t bytes);
int hw_free(void *mem);
void *get_start_sbrk(void);
void brkInit();
void BinInit();
void MallocAddrInit();
void printfBinary(unsigned int bin);
void split(size_t index);
void printfHeader(struct Header *header);
void printfAllocAddr();
int checkAddr(void *mem);
void putBin(void *mem);

#endif
