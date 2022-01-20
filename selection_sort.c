#include "selection_sort.h"
#include <omp.h>

static void swap(int array[], size_t indexA, size_t indexB) {
    int buffer = array[indexA];
    array[indexA] = array[indexB];
    array[indexB] = buffer;
}

static size_t find_min_value_index(const int array[], size_t size) {
    size_t result = 0;
    for (size_t i = 1; i < size; i++) {
        if (array[i] < array[result])
            result = i;
    }
    return result;
}

AlgorithmResult selection_sort_simple(int array[], size_t array_size) {
    double start = omp_get_wtime();
    for (size_t target = 0; target < array_size - 1; target++)
        swap(array, target, find_min_value_index(array + target, array_size - target) + target);

    double end = omp_get_wtime();

    return (AlgorithmResult) { end - start, -1 };
}

AlgorithmResult selection_sort_parallel_method1(int array[], size_t array_size) {
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
            selection_sort_simple(array + start_index, next_partition_start - start_index);
        }
    }

    double start_step_2 = omp_get_wtime();

    merge_partitioned_array(array, array_size, threads_used);

    double end = omp_get_wtime();

    return (AlgorithmResult) {start_step_2 - start_step_1, end - start_step_2 };
}

AlgorithmResult selection_sort_parallel_method2(int array[], size_t array_size) {
    double start = omp_get_wtime();

    for (size_t target = 0; target < array_size - 1; target++) {

        size_t search_array_min_value_index = 0;

        int* search_array = array + target;
        size_t search_array_size = array_size - target;

        #pragma omp parallel default(none) shared(search_array, search_array_size, search_array_min_value_index)
        {
            int thread_id = omp_get_thread_num();
            int threads = omp_get_num_threads();

            if (threads > search_array_size)
                threads = (int) search_array_size;

            if (thread_id < search_array_size) {
                size_t chunk_size = search_array_size / threads;
                size_t next_partition_start = (thread_id == threads - 1) ? search_array_size : (thread_id + 1) * chunk_size;
                size_t start_index = thread_id * chunk_size;
                size_t local_min_value_index = find_min_value_index(search_array + start_index, next_partition_start - start_index) + start_index;
                #pragma omp critical
                {
                    if (search_array[local_min_value_index] < search_array[search_array_min_value_index])
                        search_array_min_value_index = local_min_value_index;
                }
            }
        }

        swap(array, target, target + search_array_min_value_index);
    }

    double end = omp_get_wtime();

    return (AlgorithmResult) { end - start, -1 };
}
