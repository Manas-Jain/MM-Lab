#include "umalloc.h"
int check_heap();
int check_bin();

typedef enum {
    HEAP_SUCCESS,
    HEAP_FAILURE,
} heap_err;

typedef enum {
    ORD_SIZE,           // Size-Ordered Free List (smallest to largest)
    ORD_MEM,            // Memory-Ordered Free List (lowest to highest)
    ORD_RAND,           // Random-Ordered Free List
    ORD_OTHER = -1      // Unconventional Free List Order (if you add anything to the header choose this!)
} heap_order;

extern heap_order order;
extern bool circular;