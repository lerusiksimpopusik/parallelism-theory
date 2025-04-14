#include <iostream>
#include <stdlib.h>
#include <thread>
#include <cinttypes>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <forward_list>

#ifdef NTHREADS
#else
#error "NTHREADS is not defined. Please specify -DNTHREADS=value during compilation."
#endif

#ifdef MATRIX_SIZE
#else
#error "MATRIX_SIZE is not defined. Please specify -DMATRIX_SIZE=value during compilation.(20000x20000 or 40000x40000)"
#endif

#if THREAD_CONTAINER == 1
#include <vector>
#define CONTAINER std::vector
#elif THREAD_CONTAINER == 2
#include <array> 
#define CONTAINER std::vector
#elif THREAD_CONTAINER == 3
#include <deque>
#define CONTAINER std::deque
#elif THREAD_CONTAINER == 4
#include <list>
#define CONTAINER std::list
#elif THREAD_CONTAINER == 5
#include <forward_list>
#define CONTAINER std::forward_list
#else
#error "Invalid THREAD_CONTAINER value. Use 1-5"
#endif


/**
 * @brief Displays an error message in Stderr.
 * @param message Error message.
 */
void fatal(const char *message) {
    fprintf(stderr, "%s", message);
    exit(13);
}

/**
 * @brief calls malloc and makes an error if the value is null,
 * and returns the result only if the value is excellent from zero.
 * @param size Size isolated memory.
 */
void *xmalloc(size_t size) {
    void *value = malloc(size);
    if (value == nullptr) fatal("Virtual memory exhausted");
    return value;
}

/**
 * @brief Computes matrix-vector product in a specified range of rows.
 * @warning the matrix must be represented in linear form.
 */
void MatrixVectorProductThread(const long double *matrix, const long double *vec, long double *vecRes, int lowerBound,
                               int upperBound) {
    for (size_t i = lowerBound; i <= upperBound; i++) {
        vecRes[i] = 0;
        for (size_t j = 0; j < MATRIX_SIZE; j++) {
            vecRes[i] += matrix[i * MATRIX_SIZE + j] * vec[j];
        }
    }
}

/**
 * @brief Initializes rows of the matrix from lb to ub with default values.
 */
void ParallelInitMatrix(long double *matrix, const int lowerBound, const int upperBound) {
    for (size_t i = lowerBound; i <= upperBound; i++) {
        for (size_t j = 0; j < MATRIX_SIZE; j++)
            matrix[i * MATRIX_SIZE + j] = i + j;
    }
}

/**
 * @brief Initializes part of a vector[lb:ub] with default values.
 */
void ParallelInitVec(long double *vec, const int lowerBound, const int upperBound) {
    for (size_t i = lowerBound; i <= upperBound; i++) {
        vec[i] = i;
    }
}

/**
 * @brief Initializes matrix and vectors in parallel using multiple threads.
 */
void ParallelDataInitialization(long double *matrix, long double *vec1) {
    CONTAINER<std::jthread> threads(NTHREADS);  // Use vector instead of array for threads
    int items_per_thread = MATRIX_SIZE / NTHREADS;

    for (size_t i = 0; i < NTHREADS; i++) {
        int lb = i * items_per_thread;
        int ub = (i == NTHREADS - 1) ? (MATRIX_SIZE - 1) : (lb + items_per_thread - 1);
        threads[i] = std::jthread([matrix, vec1, lb, ub] {
            ParallelInitMatrix(matrix, lb, ub);
            ParallelInitVec(vec1, lb, ub);
        });
    }
}

/**
 * @brief Allocates memory and initializes test data for matrix and vectors.
 */
void InitTestData(long double *&a, long double *&b, long double *&c) {
    a = static_cast<long double *>(xmalloc(sizeof(*a) * MATRIX_SIZE * MATRIX_SIZE));
    b = static_cast<long double *>(xmalloc(sizeof(*b) * MATRIX_SIZE));
    c = static_cast<long double *>(xmalloc(sizeof(*c) * MATRIX_SIZE));

    ParallelDataInitialization(a, b);
}

/**
 * @brief Computes matrix-vector multiplication in parallel using multiple threads.
 */
void ParallelMatrixVectorMultiply(const long double *a, const long double *b, long double *c) {
    CONTAINER<std::jthread> threads(NTHREADS);  // Use vector instead of array for threads
    int items_per_thread = MATRIX_SIZE / NTHREADS;

    for (size_t i = 0; i < NTHREADS; i++) {
        int lb = i * items_per_thread;
        int ub = (i == NTHREADS - 1) ? (MATRIX_SIZE - 1) : (lb + items_per_thread - 1);
        threads[i] = std::jthread(MatrixVectorProductThread, a, b, c, lb, ub);
    }
}

/**
 * @brief Сalculates the time spent on the parallel multiplication of the matrix
 *        by the vector.
 * @param trials Number of trials to perform. Default is 20 trials.
 * @return minimum time (20 runs by default) spent on executing all trials of the parallel part
 */
double TimeExecution(int trials = 20) {
    long double *a, *b, *c;

    double best_time = std::numeric_limits<double>::max();

    for (int i = 0; i < trials; ++i) {
        InitTestData(a, b, c);

        auto start = std::chrono::high_resolution_clock::now();

        ParallelMatrixVectorMultiply(a, b, c);

        auto end = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

        best_time = (best_time < elapsed) ? best_time : elapsed;

        free(a);
        free(b);
        free(c);

        printf("trial = %d\n", i);
    }

    return best_time;
}

int main() {
    int m = MATRIX_SIZE;
    int n = MATRIX_SIZE;
    printf("Matrix-vector product (c[m] = a[m, n] * b[n]; m = %d, n = %d)\n", m, n);
    printf("Memory used: %" PRIu64 " MiB\n", ((m * n + m + n) * sizeof(long double)) >> 20);
    printf("Number of threads: %d\n", NTHREADS);

    printf("Best calculations took %.4lf seconds.\n", TimeExecution());
}
