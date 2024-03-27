/* SPDX-License-Identifier: MIT
 * Copyright (C) 2024 Avnet
 * Authors: Nikola Markovic <nikola.markovic@avnet.com> et al.
 */
#ifndef HEAP_TRACKER_H
#define HEAP_TRACKER_H

typedef struct {
    float random_failure_percentage;  // from 0.0 to 1.0 a chance that malloc will fail
    int num_successful_allocations;   // Set to a number of allocations that will succeed before all start to fail. 0 otherwise.
} HeapSimConfig;

extern HeapSimConfig ht_config;

// hooks:
void* ht_malloc(size_t size);
void ht_free(void *);

void ht_reset_config(void);
void ht_init(void);

// tracks number of frees and allocs. Negative value means double free, positive value means leak
int ht_get_num_current_allocations(void);
void ht_print_status(void);
void ht_print_summary(void);

#endif // HEAP_SIMULATOR_H