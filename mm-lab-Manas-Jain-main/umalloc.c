#include "umalloc.h"
#include "csbrk.h"
#include <stdio.h>
#include <assert.h>
#include "ansicolors.h"

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Manas Jain mj32722" ANSI_RESET;

/*
 * The following helpers can be used to interact with the mem_block_header_t
 * struct, they can be adjusted as necessary.
 */

mem_block_header_t *free_heads[BIN_COUNT];

/*
 * select_bin - selects a free list bin to use based on the 
 * block size.
 * REQUIRED   
 */

mem_block_header_t* select_bin(size_t size) {
    // Student TODO
    size_t total_size = size + sizeof(mem_block_header_t);
    if (total_size <= 64) return free_heads[0];
    else if (total_size <= 256) return free_heads[1];
    else if (total_size <= 1024) return free_heads[2];    
    else return free_heads[3];
}

/**
 * get the bin index itself instead of just the head based on same sizes as select_bin
 */
int get_bin_index(size_t total_size) {
    if (total_size <= 64) return 0;
    else if (total_size <= 256) return 1;
    else if (total_size <= 1024) return 2;
    else return 3;
}

/*
 * block_metadata - returns true if a block is marked as allocated.
 */
bool is_allocated(mem_block_header_t *block) {
    assert(block != NULL);
    return block->block_metadata & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(mem_block_header_t *block) {
    assert(block != NULL);
    block->block_metadata |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(mem_block_header_t *block) {
    assert(block != NULL);
    block->block_metadata &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(mem_block_header_t *block) {
    assert(block != NULL);
    return block->block_metadata & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
mem_block_header_t *get_next(mem_block_header_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * set_block_metadata
 * Optional helper method that can be used to initialize the fields for the 
 * memory block struct. 
 */
void set_block_metadata(mem_block_header_t *block, size_t size, bool alloc) {
    // Optional student todo
    assert(block != NULL);
    block->block_metadata = size & ~(ALIGNMENT - 1);
    if (alloc) {
        block->block_metadata |= 0x1;
    }
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(mem_block_header_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_header - given a payload, returns the block.
 */
mem_block_header_t *get_header(void *payload) {
    assert(payload != NULL);
    return ((mem_block_header_t *)payload) - 1;
}

/**
 * insert block in memory order given the head of the appropriate bin and the new block
 */
void insert_in_memory_order(mem_block_header_t **head, mem_block_header_t *new_block) {
    if (*head == NULL || (uintptr_t)new_block < (uintptr_t)*head) {
        new_block->next = *head;
        *head = new_block;
        return;
    }
    mem_block_header_t *current = *head;
    while (current->next != NULL && (uintptr_t)current->next < (uintptr_t)new_block) {
        current = current->next;
    }
    new_block->next = current->next;
    current->next = new_block;
}

/**
 *remove block from free list given the block to remove
 */
void remove_from_free_list(mem_block_header_t *block) {
    size_t block_size = get_size(block);
    int bin = get_bin_index(block_size);

    if (free_heads[bin] == block) {
        free_heads[bin] = block->next;
        block->next = NULL;
        return;
    }
    
    mem_block_header_t *current = free_heads[bin];
    while (current != NULL && current->next != block) {
        current = current->next;
    }
    
    if (current != NULL) {
        current->next = block->next;
        block->next = NULL;
    }
}

/*
 * The following are helper functions that can be implemented to assist in your
 * design, but they are not required. 
 */

/*
 * find - finds a free block that can satisfy the umalloc request.
 */
mem_block_header_t *find(size_t payload_size) {
    // Student TODO
    //first fit
    size_t total_size = payload_size + sizeof(mem_block_header_t);
    total_size = ALIGN(total_size);
    int start_bin = get_bin_index(total_size);
    for (int bin = start_bin; bin < BIN_COUNT; bin++) {
        mem_block_header_t *current = free_heads[bin];
        
        while (current != NULL) {
            size_t current_size = get_size(current);
            
            if (current_size >= total_size) {
                return current;
            }
            
            current = get_next(current);
        }
    }
    
    return NULL;
}

/*
 * extend - extends the heap if more memory is required.
 */
mem_block_header_t *extend(size_t size) {
    // Student TODO
    size_t extend_size = size;
    if (extend_size <= 128) {
        extend_size = 256;
    } else if (extend_size <= 512) {
        extend_size = 1024;
    } else if (extend_size < PAGESIZE) {
        extend_size = PAGESIZE;
    }
    
    void *new_memory = csbrk(extend_size);
    if (new_memory == (void*)-1) {
        return NULL;
    }
    
    mem_block_header_t *new_block = (mem_block_header_t*)new_memory;
    set_block_metadata(new_block, extend_size, false);
    new_block->next = NULL;
    int bin_index = get_bin_index(extend_size);
    insert_in_memory_order(&free_heads[bin_index], new_block);
    return new_block;
}

/*
 * split - splits a given block in parts, one allocated, one free.
 */
mem_block_header_t *split(mem_block_header_t *block, size_t new_block_size) {
    // Student TODO
    if (block == NULL) {
        return NULL;
    }
    size_t original_size = get_size(block);
    size_t remaining_size = original_size - new_block_size;
    
    //always split if something is left
    if (remaining_size >= sizeof(mem_block_header_t)) {
        set_block_metadata(block, new_block_size, false);
        
        mem_block_header_t *remainder = (mem_block_header_t*)((char*)block + new_block_size);
        set_block_metadata(remainder, remaining_size, false);
        remainder->next = NULL;
        
        int bin_index = get_bin_index(remaining_size);
        insert_in_memory_order(&free_heads[bin_index], remainder);
    }
    
    return block;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
mem_block_header_t *coalesce(mem_block_header_t *block) {
    // Student TODO
    if (block == NULL) {
        return NULL;
    }

    size_t current_size = get_size(block);
    mem_block_header_t *right_neighbor = (mem_block_header_t *)((char *)block + current_size);
    // bool can_coalesce_right = false;

    // for(int i = 0; i < BIN_COUNT; i++) {
    //     mem_block_header_t *curr = free_heads[i];
    //     while (curr != NULL) {
    //         if ((void *)curr == (void *)right_neighbor && !is_allocated(curr)) {
    //             can_coalesce_right = true;
    //             break;
    //         }
    //         curr = curr->next;
    //     }
    // }

    // if (can_coalesce_right) {
    //     mem_block_header_t *next_block = right_neighbor->next;
    //     remove_from_free_list(right_neighbor);
    //     size_t new_size = current_size + get_size(right_neighbor);
    //     set_block_metadata(block, new_size, false);
    //     block->next = next_block;
    // }

    int bin_index = get_bin_index(current_size);
    mem_block_header_t *curr = free_heads[bin_index];
    while (curr != NULL) {
        if ((void *)curr == (void *)right_neighbor && !is_allocated(curr)) {
            mem_block_header_t *next_block = right_neighbor->next;
            remove_from_free_list(right_neighbor);
            size_t new_size = current_size + get_size(right_neighbor);
            set_block_metadata(block, new_size, false);
            block->next = next_block;
            break;
        }
        curr = curr->next;
    }

    // mem_block_header_t *left_neighbor = NULL;
    // for (int i = 0; i < BIN_COUNT; i++) {
    //     mem_block_header_t *curr = free_heads[i];
    //     while (curr != NULL) {
    //         if ((void *)((char *)curr + get_size(curr)) == (void *)block) {
    //             left_neighbor = curr;
    //             break;
    //         }
    //         curr = curr->next;
    //     }
    //     if (left_neighbor != NULL) {
    //         mem_block_header_t *next_block = block->next;
    //         remove_from_free_list(left_neighbor);
    //         size_t new_size = get_size(left_neighbor) + current_size;
    //         set_block_metadata(left_neighbor, new_size, false);
    //         left_neighbor->next = next_block;
    //         block = left_neighbor;
    //         break;
    //     }
    // }
    
    mem_block_header_t *left_neighbor = NULL;
    curr = free_heads[bin_index];
    while (curr != NULL) {
        if ((mem_block_header_t *)((char *)curr + get_size(curr)) == (void *)block) {
            left_neighbor = curr;
            break;
        }
        curr = curr->next;
    }
    if (left_neighbor != NULL) {
        mem_block_header_t *next_block = block->next;
        remove_from_free_list(left_neighbor);
        size_t new_size = get_size(left_neighbor) + get_size(block);
        set_block_metadata(left_neighbor, new_size, false);
        left_neighbor->next = next_block;
        block = left_neighbor;
    }
    
    return block;
}

/*
The following functions are REQUIRED to implement
*/


/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    // Student TODO
    for (int i = 0; i < BIN_COUNT; i++) {
        free_heads[i] = NULL;
    }
    void *heap_start = csbrk(PAGESIZE);
    //man sbrk
    if (heap_start == (void*)-1) {
        return -1;
    }
    
    mem_block_header_t *first_block = (mem_block_header_t*)heap_start;
    set_block_metadata(first_block, PAGESIZE, false);
    first_block->next = NULL;
    int bin_index = get_bin_index(PAGESIZE);
    insert_in_memory_order(&free_heads[bin_index], first_block);
    
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size)
{
    // STUDENT TODO
    if (size == 0) {
        return NULL;
    }
    mem_block_header_t *block = find(size);
    if (block == NULL) {
        size_t total_size = size + sizeof(mem_block_header_t);
        total_size = ALIGN(total_size);
        block = extend(total_size);
        if (block == NULL) {
            return NULL;
        }
    }
    remove_from_free_list(block);
    size_t total_size = size + sizeof(mem_block_header_t);
    total_size = ALIGN(total_size);
    block = split(block, total_size);
    allocate(block);
    block->next = NULL;
    return get_payload(block);
}

/**
 * @param ptr the pointer to the memory to be freed,
 * must have been called by a previous malloc call
 * @brief frees the memory space pointed to by ptr.
 */
void ufree(void *ptr)
{
    // STUDENT TODO
    if (ptr == NULL) {
        return;
    }
    mem_block_header_t *block = get_header(ptr);
    deallocate(block);
    block->next = NULL;
    block = coalesce(block);
    size_t block_size = get_size(block);
    int bin_index = get_bin_index(block_size);
    insert_in_memory_order(&free_heads[bin_index], block);
}