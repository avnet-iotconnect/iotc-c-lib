/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "heap_tracker.h"

#define MAX_ADDRESSES 1000

HeapSimConfig ht_config = {0};
struct {
    int malloc_calls; // increments when malloc(), decrements when free(). Should be 0 at the end of test.
    int allocations_on_heap;     // Track how many allocations we make in total.

    // we could track heap size for a future implementation. would need to record each malloc size and track when freeing
    // size_t heap_remaining;

    // we could track allocations for a future implementation. some struct that tracks addresses allocated and sizes
    // void* allocations[MAX_ADDRESSES];
} ht_context = {0};



void ht_reset_config() {
    memset(&ht_config, 0, sizeof(ht_config));
}

void ht_init() {
    // we can init heap remaining based on the left heap size configured

    memset(&ht_context, 0, sizeof(ht_context));
}

int ht_get_num_current_allocations(void) {
    return ht_context.allocations_on_heap;
}

void ht_print_status(void) {
    printf("Mallocs: %d. On heap: %d.\n", ht_context.malloc_calls, ht_context.allocations_on_heap);
}
void ht_print_summary(void) {
    ht_print_status();
    if (ht_context.allocations_on_heap) {
        printf(" --- MEMORY LEAK DETECTED --- ");
    } else {
        printf("No memory leaks detected.");
    }
}
void *ht_malloc(size_t size) {
    ht_context.malloc_calls++;

    if (ht_config.num_successful_allocations > 0) { // has to be at least 1
        // if we want to simulate N successful allocations and all further ones to fail
        if (ht_context.malloc_calls > ht_config.num_successful_allocations) {
            return NULL;
        }
    }

    double r = (double) rand() / RAND_MAX;
    if (r < (double) ht_config.random_failure_percentage) {
        return NULL;
    }
    ht_context.allocations_on_heap++;
    return malloc(size);
}


void ht_free(void *ptr) {
    if (NULL != ptr) {
        ht_context.allocations_on_heap--;
    }
    free(ptr);
}
