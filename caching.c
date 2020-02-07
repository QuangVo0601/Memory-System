#include <stdio.h>
#include <stdlib.h>
#include "memory_system.h"
#include "caching.h"
/* Feel free to implement additional header files as needed */

/*
 * This program is a implementation of a Virtual Address to Physical Address memory mapping using bit-level operators.
 * A paged memory management system with three components, CPU, Cache, and Memory, where for a given virtual address, the program will output the associated byte in the system memory.
 */

//Initialization of TLB, Page Table, and Two-Way Set Associative Cache
void
initialize() {
/* if there is any initialization you would like to have, do it here */
/*  This is called for you in the main program */
    
    //initialize an empty TLB with all entries have valid set to 0
    int i;
    for(i = 0; i < TLB_SIZE; i++){
        TLB[i].valid = 0;
        TLB[i].tag = 0;
        TLB[i].PPN = 0;
    }
    
    //initialize an empty page table with all entries have valid set to 0
    int j;
    for(j = 0; j < PAGE_TABLE_SIZE; j++){
        pageTable[j].valid = 0;
        pageTable[j].PPN = 0;
    }
    
    //initialize an empty cache with all entries have valid set to 0
    int k, l;
    for(k = 0; k < CACHE_SIZE; k++){ 
        for(l = 0; l < 2; l++){
            cache[k].entry[l].valid = 0;
            cache[k].entry[l].tag = 0;
            cache[k].entry[l].data0 = '\0';
            cache[k].entry[l].data1 = '\0';
            cache[k].entry[l].data2 = '\0';
            cache[k].entry[l].data3 = '\0';
            cache[k].entry[l].timestamp = 0;
        }
    }
}

//This function updates the TLB when PPN is found in Page Table or is loaded from memory
void updateTLB(int newTag, int newPPN, int index){
    
    TLB[index].tag = newTag;
    TLB[index].PPN = newPPN;
    TLB[index].valid = 1;
}

//This function updates the Page Table when PPN is loaded from memory
void updatePageTable(int newPPN, int VPN){
    
    pageTable[VPN].PPN = newPPN;
    pageTable[VPN].valid = 1;
}

//This function updates Cache when data is returned from memory
void updateCache(int newTag, int newData, int index){
    
    //when both entries are invalid, use the first one
    if(cache[index].entry[0].valid == 0 && cache[index].entry[1].valid == 0){
        cache[index].entry[0].tag = newTag;
        cache[index].entry[0].valid = 1;
        cache[index].entry[0].timestamp = 1; //entry 0 is now newer
        cache[index].entry[1].timestamp = 0; //entry 1 is now older
        updateCacheData(newData, index, 0);
    }
    //when entry 0 is invalid and entry 1 is valid, use entry 0
    else if(cache[index].entry[0].valid == 0 && cache[index].entry[1].valid == 1){
        cache[index].entry[0].tag = newTag;
        cache[index].entry[0].valid = 1;
        cache[index].entry[0].timestamp = 1; //entry 0 is now newer
        cache[index].entry[1].timestamp = 0; //entry 1 is now older
        updateCacheData(newData, index, 0);
    }
    //when entry 0 is valid and entry 1 is invalid, use entry 1
    else if(cache[index].entry[0].valid == 1 && cache[index].entry[1].valid == 0){
        cache[index].entry[1].tag = newTag;
        cache[index].entry[1].valid = 1;
        cache[index].entry[1].timestamp = 1; //entry 1 is now newer
        cache[index].entry[0].timestamp = 0; //entry 0 is now older
        updateCacheData(newData, index, 1);
    }
    //when both entries are valid
    else{
        //when entry 0 is older than entry 1, use entry 0
        if(cache[index].entry[0].timestamp < cache[index].entry[1].timestamp){
            cache[index].entry[0].tag = newTag;
            cache[index].entry[0].valid = 1;
            cache[index].entry[0].timestamp = 1; //entry 0 is now newer
            cache[index].entry[1].timestamp = 0; //entry 1 is now older
            updateCacheData(newData, index, 0);
        }
        //when entry 1 is older than entry 0, use entry 1
        else{
            cache[index].entry[1].tag = newTag;
            cache[index].entry[1].valid = 1;
            cache[index].entry[1].timestamp = 1; //entry 1 is now newer
            cache[index].entry[0].timestamp = 0; //entry 0 is now older
            updateCacheData(newData, index, 1);
        }
    }
}

//This functions supports updateCache() function to store new data in little endian
void updateCacheData(int newData, int index, int entryNumber){
    
    cache[index].entry[entryNumber].data0 = newData & 0xFF;
    cache[index].entry[entryNumber].data1 = (newData >> 8) & 0xFF;
    cache[index].entry[entryNumber].data2 = (newData >> 16) & 0xFF;
    cache[index].entry[entryNumber].data3 = (newData >> 24) & 0xFF;
}

/* You will implement the two functions below:
 *     * you may add as many additional functions as you like
 *     * you may add header files for your data structures
 *     * you MUST call the relevant log_entry() functions (described below)
 *          or you will not receive credit for all of your hard work!
 */

//This function converts the incoming virtual address to a physical address
int
get_physical_address(int virt_address) {
/*
   Convert the incoming virtual address to a physical address. 
     * if virt_address too large, 
          log_entry(ILLEGALVIRTUAL,virt_address); 
          return -1
     * if PPN is in the TLB, 
	  compute PA 
          log_entry(ADDRESS_FROM_TLB,PA);
          return PA
     * else use the page table function to get the PPN:
          * if VPN is in the Page Table
	          compute PA 
                  add the VPN and PPN in the TLB
	          log_entry(ADDRESS_FROM_PAGETABLE,PA);
	          return PA
	  * else load the frame into the page table
	          PPN = load_frame(VPN) // use this provided library function
                  add the VPN and PPN in to the page table
                  add the VPN and PPN in to the TLB
 		  compute PA
		  log_entry(ADDRESS_FROM_PAGE_FAULT_HANDLER,PA);
 		  return PA
*/
    
    int PA;
    int tag = 0;
    int index = 0;
    int VPN = 0;
    int VPO = 0;
    int PPN = 0;
    int PPO = 0;
    
    //check for valid virtual address
    if(virt_address > 0x3FFFF){ //max 18 bits of virtual address
        log_entry(ILLEGALVIRTUAL, virt_address);
        return -1;
    }
    
    tag = (virt_address >> 13) & 0x1F; //get 5 bits of tag
    index = (virt_address >> 9) & 0xF; //get 4 bits of index
    VPN = (virt_address >> 9) & 0x1FF; //get 9 bits of VPN
    VPO = virt_address & 0x1FF; //get 9 bits of VPO
    PPO = VPO; //9 bit PPO
    
    //check for valid generated VPN
    if(VPN > 0x1FF){ //max 9 bits of VPN
        log_entry(ILLEGALVPN, VPN);
        return -1;
    }
    
    //check to see if PPN is in TLB
    if(TLB[index].valid == 1 && TLB[index].tag == tag){
        PPN = TLB[index].PPN; // get 11 bits of PPN
        PA = ((PPN << 9) & 0xFFE00) | PPO; //calculate 20 bits of PA
        log_entry(ADDRESS_FROM_TLB, PA);
    }
    //check to see if PPN is in page table
    else if(pageTable[VPN].valid == 1){
        PPN = pageTable[VPN].PPN;
        PA = ((PPN << 9) & 0xFFE00) | PPO;
        updateTLB(tag, PPN, index); //update TLB
        log_entry(ADDRESS_FROM_PAGETABLE,PA);
    }
    //load the frame and return the PPN from memory
    else{
        PPN = load_frame(VPN);
        updatePageTable(PPN, VPN); //update page table
        updateTLB(tag, PPN, index); //update TLB
        //printf("PPN in page table is 0x%x\n", pageTable[VPN].PPN);
        PA = ((PPN << 9) & 0xFFE00) | PPO;
        log_entry(ADDRESS_FROM_PAGE_FAULT_HANDLER,PA);
    }
    
    //check for valid generated physical address
    if(PA > 0xFFFFF){ //max 20 bits of physical address
        log_entry(PHYSICALERROR, PA);
        return -1;
    }

    return PA;
}

//This function uses the incoming physical address to find the relevant byte
char
get_byte(int phys_address) {
/*
   Use the incoming physical address to find the relevant byte. 
     * if data is in the cache, use the offset (last 2 bits of address)
          to compute the byte to be returned data
          log_entry(DATA_FROM_CACHE,byte);
          return byte 
     * else use the function get_long_word(phys_address) to get the 
          4 bytes of data where the relevant byte will be at the
          given offset (last 2 bits of address)
          log_entry(DATA_FROM_MEMORY,byte);
          return byte

NOTE: if the incoming physical address is too large, there is an
error in the way you are computing it...
*/
    char byte;
    int tag = 0;
    int index = 0;
    int offset = 0;
    
    tag = (phys_address >> 7) & 0x1FFF; //get 13 bits of tag
    index = (phys_address >> 2) & 0x1F; //get 5 bits of index
    offset = phys_address & 0x3; //get 2 bits of byte offset
    
    //check to see if data is in cache
    //check entry 0
    if(cache[index].entry[0].valid == 1 && cache[index].entry[0].tag == tag){
        //the relevant byte will be at the given offset
        if(offset == 0){
            byte = cache[index].entry[0].data0;
        }
        else if(offset == 1){
            byte = cache[index].entry[0].data1;
        }
        else if(offset == 2){
            byte = cache[index].entry[0].data2;
        }
        else{
            byte = cache[index].entry[0].data3;
        }
        
        log_entry(DATA_FROM_CACHE,byte);
    }
    //check entry 1
    else if(cache[index].entry[1].valid == 1 && cache[index].entry[1].tag == tag){
        //the relevant byte will be at the given offset
        if(offset == 0){
            byte = cache[index].entry[1].data0;
        }
        else if(offset == 1){
            byte = cache[index].entry[1].data1;
        }
        else if(offset == 2){
            byte = cache[index].entry[1].data2;
        }
        else{
            byte = cache[index].entry[1].data3;
        }
        
        log_entry(DATA_FROM_CACHE,byte);
        
    }
    //get data directly from main memory
    else{
        
        int data = get_word(phys_address); //get 4-byte integer from main memory
        
        //the relevant byte will be at the given offset
        if(offset == 0){
            byte = data & 0xFF;
        }
        else if(offset == 1){
            byte = (data >> 8) & 0xFF;
        }
        else if(offset == 2){
            byte = (data >> 16) & 0xFF;
        }
        else{
            byte = (data >> 24) & 0xFF;
        }
        
        updateCache(tag, data, index); //update cache with new data
        log_entry(DATA_FROM_MEMORY,byte);
        
    }
   return byte;
}

