/**************************************************************************
 * C S 429 MM-lab
 * 
 * csbrk.c - A wrapper for sbrk system call. Used to keep track of calls
 * and introduce an upper limit to the amount of memory one can request at any
 * one time.
 * 
 * Copyright (c) 2021 M. Hinton. All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/

 #include "csbrk.h"
 #include <sys/mman.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <assert.h>
 #include <unistd.h>
 #include <stdbool.h>
 
typedef struct sbrk_block_struct
{
    uint64_t sbrk_start;
    uint64_t sbrk_end;
    struct sbrk_block_struct *next;
} sbrk_block;

sbrk_block *sbrk_blocks = NULL;
size_t sbrk_bytes;
size_t num_sbrks = 0;
void *last_start = NULL;
 
 /*
  * csbrk - A wrapper for sbrk. Places a maximum on the maximum amount of memory
  * that can be requested. If tracking is enabled, keeps track of the sbrk regions
  * allocated for correctness and utilization.  
  */
void *csbrk(intptr_t increment)
{
#ifdef TRACK_CSBRK
    void *ret = NULL;
    if (num_sbrks % 7 == 1) {
        // ret is the start of the newly allocated memory
        last_start = sbrk(increment + PAGESIZE * 16);
        mprotect(last_start, PAGESIZE * 16, PROT_NONE);
        ret = 16 * PAGESIZE + last_start;
    }
    else if (num_sbrks % 7 == 2) {
        //use the prev memory
        if (increment < 16 * PAGESIZE && last_start) {
            mprotect(last_start, increment, PROT_READ|PROT_WRITE|PROT_EXEC);
            ret = last_start;
            last_start = NULL;
        }
        else {
            ret = sbrk(increment);
        }

    }
    else if (num_sbrks % 7 == 5) {
        // block off the end
        ret = sbrk(increment + PAGESIZE * 3);
        mprotect((void *) ((char *) ret + increment), PAGESIZE * 3, PROT_NONE);
    }
    else {
        ret = sbrk(increment);
    }

    num_sbrks++;

    sbrk_bytes += increment;
    uint64_t sbrk_start_temp = (uint64_t)ret;
    uint64_t sbrk_end_temp = sbrk_start_temp + (uint64_t)increment;
    bool coalesced = false;
    sbrk_block *temp = sbrk_blocks;
    while (temp != NULL)
    {
        if (temp->sbrk_end == sbrk_start_temp){
            temp->sbrk_end = sbrk_end_temp;
            coalesced = true;
            break;
        }
        temp = temp->next;
    }

    if (!coalesced) {
        sbrk_block *temp = malloc(sizeof(sbrk_block));
        temp->sbrk_start = sbrk_start_temp;
        temp->sbrk_end = sbrk_end_temp;

        temp->next = sbrk_blocks;
        sbrk_blocks = temp;
    }
 
    return ret;
#endif
}
 
 /*
  * check_malloc_output - Checks that a payload returned by umalloc falls within
  * one of the sbrk regions.
  */
int check_malloc_output(void *payload_start, size_t payload_length)
{
    uint64_t start_uint = (uint64_t)payload_start;
    uint64_t end_uint = start_uint + (uint64_t)payload_length;
    sbrk_block *temp = sbrk_blocks;
    while (temp != NULL)
    {
        if (start_uint >= temp->sbrk_start && end_uint <= temp->sbrk_end)
        {
            return 0;
        }
        temp = temp->next;
    }

    return -1;
}