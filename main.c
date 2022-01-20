#include <stdio.h>
#include "util.h"

#include "selection_sort.h"
#include "tim_sort.h"

#include <omp.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>

// tim sort & selection sort

typedef AlgorithmResult (*AlgorithmFunction)(int[], size_t);
typedef struct AlgorithmInfo AlgorithmInfo;
struct AlgorithmInfo {
    AlgorithmFunction function;
    char* name;
    bool hasSecondStep;
};

void test(
        const AlgorithmInfo algorithms[],
        size_t algorithms_size,
        const size_t sizes[],
        size_t sizes_size,
        int threads_num,
        char* output
) {
    FILE* file = fopen(output, "w");
    omp_set_num_threads(threads_num);

    // Printing CSV headers
    fprintf(file, "Array size");
    for (size_t algorithm_index = 0; algorithm_index < algorithms_size; algorithm_index++) {
        fprintf(file, ",%s", algorithms[algorithm_index].name);
        if (algorithms[algorithm_index].hasSecondStep)
            fprintf(file, " (step 1), %s (step 2)", algorithms[algorithm_index].name);
    }
    fprintf(file, "\n");

    for (size_t size_index = 0; size_index < sizes_size; size_index++) {
        size_t array_size = sizes[size_index];

        // Preparing source and reference arrays
        int* source_array = random_array_allocated(0, (int) array_size, array_size);
        int* reference_array = malloc(array_size * sizeof(int));
        memcpy(reference_array, source_array, array_size * sizeof(int));
        qsort(reference_array, array_size, sizeof(int), reference_compare);

        int* result_buffer_array = malloc(array_size * sizeof(int));

        // First CSV column (array size)
        fprintf(file, "%lu", array_size);

        // Running tests for different algorithms
        for (size_t algorithm_index = 0; algorithm_index < algorithms_size; algorithm_index++) {
            AlgorithmInfo algorithm = algorithms[algorithm_index];
            memcpy(result_buffer_array, source_array, array_size * sizeof(int));

            printf("Testing '%s' (array size: %lu, threads: %d)... ", algorithm.name, array_size, threads_num);
            fflush(stdout);

            AlgorithmResult result = algorithm.function(result_buffer_array, array_size);
            if (memcmp(result_buffer_array, reference_array, array_size * sizeof(int)) == 0)
                printf("OK");
            else
                printf("WRONG RESULT");

            if (algorithm.hasSecondStep) {
                printf(" (%f + %f s)", result.firstStepTime, result.secondStepTime);
                fprintf(file, ",%f,%f", result.firstStepTime, result.secondStepTime);
            } else {
                printf(" (%f s)", result.firstStepTime);
                fprintf(file, ",%f", result.firstStepTime);
            }

            printf("\n");
        }

        fprintf(file, "\n");
    }

    printf("Report saved in file '%s'\n", output);
    fclose(file);
}

int main(int argc, char** argv) {
    AlgorithmInfo algorithms[] = {
            { selection_sort_simple,            "Selection simple",     false },
            { selection_sort_parallel_method1,  "Selection parallel 1", true },
            { selection_sort_parallel_method2,  "Selection parallel 2", false },
            { tim_sort_simple,                  "Tim simple",           true },
            { tim_sort_parallel_method1,        "Tim parallel 1",       true },
            { tim_sort_parallel_method2,        "Tim parallel 2",       true },
    };
    size_t sizes[] = {1, 10, 100, 1000, 10000, 100000};
    int threads[] = {1, 2, 4, 6};

    int sizes_size = sizeof(sizes)/sizeof(sizes[0]);
    int threads_size = sizeof(threads)/sizeof(threads[0]);
    int algorithms_size = sizeof(algorithms)/sizeof(algorithms[0]);

    struct stat st = {0};
    // Create 'reports' directory if it doesn't exist
    if (stat("reports", &st) == -1)
        mkdir("reports", 0777);

    // Run tests for different threads' number
    for (size_t i = 0; i < threads_size; i++) {
        char name_buffer[256];
        snprintf(name_buffer, sizeof(name_buffer), "reports/sizes_test_%d_threads.csv", threads[i]);
        test(algorithms, algorithms_size, sizes, sizes_size, threads[i], name_buffer);
    }

    return 0;
}

