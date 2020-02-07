//Header file for the file caching.c
#ifndef CACHING_H
#define CACHING_H

#include "memory_system.h"

//TLB with valid, tag, and PPN
typedef struct _TLB_entry
{
    int valid;
    int tag;
    int PPN;
} TLB_entry;

//Page table with valid and PPN
typedef struct _pageTable_entry
{
    int valid;
    int PPN;
} pageTable_entry;

//Cache with valid, tag, timestamp, and block data
typedef struct _cache_entry{
    int valid;
    int tag;
    char data0;
    char data1;
    char data2;
    char data3;
    int timestamp;
} cache_entry;

//Two-Way Set Associative Cache
typedef struct _memory_cache
{
    cache_entry entry[2];
} memory_cache;

//prototypes
void updateTLB(int newTag, int newPPN, int index);
void updatePageTable(int newPPN, int VPN);
void updateCache(int newTag, int newData, int index);
void updateCacheData(int newData, int index, int entryNumber);

#define TLB_SIZE 16 // 2^4 = 16 (4 bit index in virtual address)
#define PAGE_TABLE_SIZE 512 // 2^9 = 512 (9 bit VPN in virtual address)
#define CACHE_SIZE 32 // 2^5 = 32 (5 bit index in physical address)

TLB_entry TLB[TLB_SIZE]; //TLB with size 16
pageTable_entry pageTable[PAGE_TABLE_SIZE]; //page table with size 512
memory_cache cache[CACHE_SIZE]; //cache with size 32

#endif
