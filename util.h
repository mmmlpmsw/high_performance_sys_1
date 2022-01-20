#pragma once

#include <stddef.h>

typedef struct AlgorithmResult AlgorithmResult;
struct AlgorithmResult {
    double firstStepTime;
    double secondStepTime;
};

size_t min(size_t a, size_t b);
void print_array(int* array, size_t size);
int reference_compare(const void* a, const void* b);
int* random_array_allocated(int min, int max, size_t size);
void merge_partitioned_array(int* array, size_t size, size_t partitions_num);
