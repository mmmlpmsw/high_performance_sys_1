#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "tim_sort.h"

const int RUN = 32;

static void insertion_sort(int array[], size_t start, size_t end_exclusive) {
    for (size_t i = start + 1; i < end_exclusive; i++) {
        int buffer = array[i];
        ssize_t j = (ssize_t) i - 1;
        while (j >= (ssize_t) start && array[j] > buffer) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = buffer;
    }
}

void merge_runs(int array[], size_t start, size_t middle, size_t end_exclusive) {
    size_t left_array_size = middle - start + 1;
    size_t right_array_size = end_exclusive - middle - 1;
    int* left_array = malloc(left_array_size * sizeof(int));
    int* right_array = malloc(right_array_size * sizeof(int));

    for (size_t i = 0; i < left_array_size; i++)
        left_array[i] = array[start + i];
    for (size_t i = 0; i < right_array_size; i++)
        right_array[i] = array[middle + 1 + i];

    size_t  left_i = 0,
            right_i = 0,
            dest_i = start;

    // Merge 'left_array' and 'right_array' into 'array'
    while (left_i < left_array_size && right_i < right_array_size) {
        if (left_array[left_i] <= right_array[right_i]) {
            array[dest_i] = left_array[left_i];
            left_i++;
        } else {
            array[dest_i] = right_array[right_i];
            right_i++;
        }
        dest_i++;
    }

    // Copy remaining of 'left_array'
    while (left_i < left_array_size)
        array[dest_i++] = left_array[left_i++];

    // Copy remaining of 'right_array'
    while (right_i < right_array_size)
        array[dest_i++] = right_array[right_i++];
}

AlgorithmResult tim_sort_simple(int array[], size_t array_size) {
    double start_step_1 = omp_get_wtime();

    for (size_t i = 0; i < array_size; i += RUN)
        insertion_sort(array, i, min(i + RUN, array_size));

    double start_step_2 = omp_get_wtime();

    for (size_t size = RUN; size < array_size; size *= 2) {
        for (size_t left = 0; left < array_size; left += 2*size) {
            size_t middle = left + size - 1;
            size_t next_chunk_start = min(left + 2*size, array_size);
            if (middle < next_chunk_start)
                merge_runs(array, left, middle, next_chunk_start);
        }
    }

    double end = omp_get_wtime();

    return (AlgorithmResult) {start_step_2 - start_step_1, end - start_step_2 };
}

AlgorithmResult tim_sort_parallel_method1(int array[], size_t array_size) {
    double start_step_1 = omp_get_wtime();

    int threads_used = 0;

    #pragma omp parallel default(none) shared(array, array_size, threads_used)
    {
        int thread_id = omp_get_thread_num();
        int threads = omp_get_num_threads();

        if (thread_id == 0)
            threads_used = threads;

        if (threads > array_size)
            threads = (int) array_size;

        if (thread_id < array_size) {
            size_t chunk_size = array_size / threads;
            size_t next_partition_start = (thread_id == threads - 1) ? array_size : (thread_id + 1) * chunk_size;
            size_t start_index = thread_id * chunk_size;
            tim_sort_simple(array + start_index, next_partition_start - start_index);
        }
    }

    double start_step_2 = omp_get_wtime();

    merge_partitioned_array(array, array_size, threads_used);

    double end = omp_get_wtime();

    return (AlgorithmResult) {start_step_2 - start_step_1, end - start_step_2 };
}

AlgorithmResult tim_sort_parallel_method2(int array[], size_t array_size) {
    double start_step_1 = omp_get_wtime();

    #pragma omp parallel default(none) shared(array_size, RUN, array)
    {
        int thread_id = omp_get_thread_num();
        int threads = omp_get_num_threads();

        if (threads > array_size)
            threads = (int) array_size;

        if (thread_id < array_size) {
            size_t runs = array_size/RUN;
            size_t runs_chunk_size = runs/threads;
            size_t start = runs_chunk_size * thread_id * RUN;
            size_t end = runs_chunk_size * (thread_id + 1) * RUN;
            if (thread_id == threads - 1)
                end = array_size;
            for (size_t i = start; i < end; i += RUN)
                insertion_sort(array, i, min(i + RUN, array_size));
        }
    }

    double start_step_2 = omp_get_wtime();

    for (size_t size = RUN; size < array_size; size *= 2) {
        #pragma omp parallel default(none) shared(array_size, array, size)
        {
            int thread_id = omp_get_thread_num();
            int threads = omp_get_num_threads();

            if (threads > array_size)
                threads = (int) array_size;

            if (thread_id < array_size) {
                size_t step = 2 * size;
                size_t chunks = array_size / step;
                size_t chunks_chunk_size = chunks / threads;
                size_t start = chunks_chunk_size * thread_id * step;
                size_t end = chunks_chunk_size * (thread_id + 1) * step;
                if (thread_id == threads - 1)
                    end = array_size;

                for (size_t left = start; left < end; left += step) {
                    size_t middle = left + size - 1;
                    size_t next_chunk_start = min(left + step, array_size);
                    if (middle < next_chunk_start)
                        merge_runs(array, left, middle, next_chunk_start);
                }
            }
        }
    }

    double end = omp_get_wtime();

    return (AlgorithmResult) {start_step_2 - start_step_1, end - start_step_2 };
}
