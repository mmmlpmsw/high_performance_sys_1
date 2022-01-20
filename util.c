#include "util.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

size_t min(size_t a, size_t b) {
    return a < b ? a : b;
}

void print_array(int* array, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    puts("");
}

int reference_compare(const void* a, const void* b) {
    int int_a = * ( (int*) a );
    int int_b = * ( (int*) b );

    if ( int_a == int_b ) return 0;
    else if ( int_a < int_b ) return -1;
    else return 1;
}

int* random_array_allocated(int min, int max, size_t size) {
    int* result = calloc(size, sizeof(int));
    for (size_t i = 0; i < size; i++)
        result[i] = (int)(min + random() % (max - min));
    return result;
}

void merge_partitioned_array(int* array, size_t size, size_t partitions_num) {
    int* src = calloc(size, sizeof(int));
    size_t* indices = calloc(partitions_num, sizeof(size_t));
    for (size_t i = 0; i < partitions_num; i++)
        indices[i] = i * (size / partitions_num);

    memcpy(src, array, size * sizeof(int));
    int* target_item = array;
    while (target_item < array + size) {
        size_t min_index_index = -1;
        for (size_t partition_id = 0; partition_id < partitions_num; partition_id++) {
            size_t next_partition_start = (partition_id == partitions_num - 1) ? size : (partition_id + 1) * (size / partitions_num);
            if (indices[partition_id] < next_partition_start && (min_index_index == -1 || src[indices[partition_id]] < src[indices[min_index_index]])) {
                min_index_index = partition_id;
            }
        }
        *target_item = src[indices[min_index_index]];
        indices[min_index_index]++;
        target_item++;
    }
    free(indices);
    free(src);
}
