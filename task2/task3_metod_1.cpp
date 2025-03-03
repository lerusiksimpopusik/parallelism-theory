#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
#include <iomanip>
#include <random>

#ifdef NTHREADS
#else
#error "NTHREADS is not defined. Please specified -DNTHREADS=value during compilation."
#endif

#ifdef MATRIX_SIZE
#else
#error "MATRIX_SIZE is not defined. Please specify -DMATRIX_SIZE=value during compilation.(20000x20000 or 40000x40000)"
#endif

const double kITERATION_STEP = 1.0 / 100000.0;
double epsilon = 0.00001;
const int MAX_ITERATIONS = 10000000; // Максимальное число итераций

// Returns the current time in seconds.
double CpuSecond() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

// Computes the matrix-vector product vecRes[MATRIX_SIZE] = matrix[MATRIX_SIZE][MATRIX_SIZE] * vec[MATRIX_SIZE].
void MatrixVectorProductOmp(const long double *matrix, long double *vec, long double *vecRes) {
    // Parallel computation of the matrix-vector product
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vecRes[i] = 0;
        for (size_t j = 0; j < MATRIX_SIZE; j++) {
            vecRes[i] += matrix[i * MATRIX_SIZE + j] * vec[j];
        }
    }
}

// Computes the difference between two vectors vec1[n] -= vec2[MATRIX_SIZE].
void SubtractVecFromVec(long double *vec1, const long double *vec2) {
    // Parallel subtraction of vectors
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vec1[i] -= vec2[i];
    }
}

// Computes the scalar-vector product vec[MATRIX_SIZE] *= scalar.
void MultiplyVecByScalar(long double *vec, const long double &scalar) {
    // Parallel multiplication of a vector by a scalar
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vec[i] *= scalar;
    }
}

// Computes the L2 norm of a vector.
double VecL2Norm(const long double *vec) {
    long double l2Norm = 0.0;

    // Parallel computation of the L2 norm with reduction
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic) reduction(+:l2Norm)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        l2Norm += vec[i] * vec[i];
    }

    return std::sqrt(l2Norm);
}

// Implements the simple iteration method for solving systems of linear equations.
double IterationMethod() {
    long double* matrixAData = new long double[MATRIX_SIZE * MATRIX_SIZE];
    long double* vecBData = new long double[MATRIX_SIZE];
    long double* vecX = new long double[MATRIX_SIZE];
    long double* vecTemp = new long double[MATRIX_SIZE];

    // Initialization of matrix A
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        for (size_t j = 0; j < MATRIX_SIZE; j++) {
            matrixAData[i * MATRIX_SIZE + j] = (i == j) ? 2.0 : 1.0;
        }
    }

    // Initialization of vectors b and x
    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        vecBData[i] = MATRIX_SIZE + 1;
        vecX[i] = 0.0;
    }

    const long double* matrixA = matrixAData;
    const long double* vecB = vecBData;

    printf("%f", VecL2Norm(vecB));
    epsilon *= VecL2Norm(vecB);

    int iterationCount = 0;

    double start = CpuSecond();

    while (iterationCount++ >= 0) {
        // vecTemp = matrixA * vecX
        MatrixVectorProductOmp(matrixA, vecX, vecTemp);

        // vecTemp = matrixA * vecX - b
        SubtractVecFromVec(vecTemp, vecB);

        // Check for convergence
        if (VecL2Norm(vecTemp) < epsilon) break;

        // Check for exceeding the maximum number of iterations
        if (iterationCount >= MAX_ITERATIONS) {
            std::cerr << "Error: Exceeded maximum number of iterations (" << MAX_ITERATIONS << ")." << std::endl;
            delete[] matrixAData;
            delete[] vecBData;
            delete[] vecX;
            delete[] vecTemp;
            exit(13);
        }

        // vecTemp = iterationStep * (matrixA * vecX - b)
        MultiplyVecByScalar(vecTemp, kITERATION_STEP);

        // vecX -= iterationStep * (matrixA * vecX - b)
        SubtractVecFromVec(vecX, vecTemp);
    }

    double end = CpuSecond();

    long double sumAbsoluteError = 0.0;
    long double sumRelativeError = 0.0;

    // Calculation of absolute and relative errors
    std::cout << "Solution x (first 10 elements): ";

    #pragma omp parallel for num_threads(NTHREADS) schedule(dynamic) reduction(+:sumAbsoluteError, sumRelativeError)
    for (int i = 0; i < MATRIX_SIZE; i++) {
        long double absoluteError = std::abs(vecX[i] - 1.0);
        long double relativeError = std::abs((vecX[i] - 1.0) / 1.0);
        
        sumAbsoluteError += absoluteError;
        sumRelativeError += relativeError;

        if (vecX[i] != 0)std::cout <<vecX[i] <<" ";
    }

    std::cout << std::endl;
    std::cout << "Number of iterations performed: " << iterationCount << std::endl;
    std::cout << "Sum of absolute errors: " << sumAbsoluteError << std::endl;
    std::cout << "Sum of relative errors: " << sumRelativeError << std::endl;

    delete[] matrixAData;
    delete[] vecBData;
    delete[] vecX;
    delete[] vecTemp;

    return end - start;
}

int main(int argc, char* argv[]) {
    int matrixSize = MATRIX_SIZE;

    std::cout << "Program using Simple Iteration method for solving linear systems (CLAY)" << std::endl;
    std::cout << "CLAY : A[" << MATRIX_SIZE << "][" << MATRIX_SIZE << "] * x[" << MATRIX_SIZE << "] = b[" << MATRIX_SIZE << "]\n";
    std::cout << "Number of threads: " << NTHREADS << std::endl;
    std::cout << "Memory used: " << static_cast<long double>((MATRIX_SIZE * MATRIX_SIZE + MATRIX_SIZE + MATRIX_SIZE + MATRIX_SIZE) * sizeof(long double)) / (1024 * 1024) << " MiB\n";

    double time = IterationMethod();
    std::cout << "Your calculations took " << std::fixed << std::setprecision(4) << time << " seconds." << std::endl;

    std::cout << "Matrix size: " << MATRIX_SIZE << "x" << MATRIX_SIZE << "\n";

    return 0;
}