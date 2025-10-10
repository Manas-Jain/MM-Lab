#include "check_heap.h"
#include <stdio.h>
#include <stdint.h>

extern mem_block_header_t *free_head;
extern mem_block_header_t *free_heads[BIN_COUNT];

int check_heap() {
    for (int i = 0; i < BIN_COUNT; ++i) {
        int result = check_bin(free_heads[i]);
        if (result) return result;
    }
    return HEAP_SUCCESS;
}

/*
 * STUDENT TODO: set these variables according to your heap design (tests will fail if you do not
 * set these variables!)
 *    - order:      how is your free list ordered?
 *    - circular:   true if your free list is circular; false otherwise
 */
heap_order order = ORD_MEM;
bool circular = false;

/*
 * check_bin -  used to check that the heap is still in a consistent state.
 * 
 * STUDENT TODO: this function is required to be completed for checkpoint 1
 * 
 *      - Ensure that the free block list is in the order you expect it to be in
 *        (if your list is randomly ordered, this check is not required).
 * 
 *      - Check if any free blocks overlap with each other. 
 * 
 *      - Ensure that each free block is aligned.
 * 
 *      - Ensure that all blocks on the free list are free (no implicit free lists)
 *
 * Should return HEAP_SUCCESS if the heap is consistent or HEAP_FAILURE if an error 
 * is detected.
 */
int check_bin(mem_block_header_t *free_head)
{
    if (free_head == NULL) {
        return HEAP_SUCCESS;
    }

    mem_block_header_t *curr = free_head;
    mem_block_header_t *prev = NULL;
    while (curr != NULL) {
        printf("Address=%p, Size=%zu, Next=%p\n", (void*)curr, get_size(curr), (void*)get_next(curr));
        
        if (prev != NULL && (uintptr_t)prev >= (uintptr_t)curr) {
            printf("Blocks not in memory order: %p >= %p\n", (void*)prev, (void*)curr);
            return HEAP_FAILURE;
        }
        
        if (prev != NULL) {
            char *prev_end = (char*)prev + get_size(prev);
            if (prev_end > (char*)curr) {
                printf("Blocks overlap: prev ends at %p, curr starts at %p\n", (void*)prev_end, (void*)curr);
                return HEAP_FAILURE;
            }
        }
        
        //if it ends in anything but 0000
        if ((uintptr_t)curr & (ALIGNMENT - 1)) {
            printf("Block %p not 16-byte aligned\n", (void*)curr);
            return HEAP_FAILURE;
        }
        
        if (is_allocated(curr)) {
            printf("Block %p is allocated but on free list\n", (void*)curr);
            return HEAP_FAILURE;
        }
        
        prev = curr;
        curr = get_next(curr);
    }
    return HEAP_SUCCESS;
}
