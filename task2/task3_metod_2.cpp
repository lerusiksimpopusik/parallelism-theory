#include <iostream>
#include <vector>
#include <omp.h>
#include <cmath>
#include <iomanip>
#include <random>

#ifdef NTHREADS
#else
#error "NTHREADS is not defined. Please specify -DNTHREADS=value during compilation."
#endif

#ifdef MATRIX_SIZE
#else
#error "MATRIX_SIZE is not defined. Please specify -DMATRIX_SIZE=value during compilation."
#endif

const double kITERATION_STEP = 1.0 / 100000.0;
double epsilon = 0.00001;

// Returns the current time in seconds.
double CpuSecond() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

// Compute matrix-vector product vecRes[MATRIX_SIZE] = matrix[MATRIX_SIZE][MATRIX_SIZE] * vec[MATRIX_SIZE].
void MatrixVectorProductOmp(const long double *matrix, long double *vec, long double *vecRes, int &lowerBound, int &upperBound) {
    for (int i = lowerBound; i <= upperBound; i++) {
        vecRes[i] = 0;
        for (int j = 0; j < MATRIX_SIZE; j++) {
            vecRes[i] += matrix[i * MATRIX_SIZE + j] * vec[j];
        }
    }
}

// Compute vector difference vec1[n] -= vec2[MATRIX_SIZE].
void SubtractVecFromVec(long double *vec1, const long double *vec2, int &lowerBound, int &upperBound){
    for (int i = lowerBound; i <= upperBound; i++){
        vec1[i] -= vec2[i];
    }
}

// Compute scalar-vector product vec[MATRIX_SIZE] *= scalar.
void MultiplyVecByScalar(long double *vec, const long double &scalar, int &lowerBound, int &upperBound){
    for (int i = lowerBound; i <= upperBound; i++){
        vec[i] *= scalar;
    }
}

double VecL2NormOmp(const long double *vec, int &lowerBound, int &upperBound){
    long double l2NormOmp = 0.0;
    for (int i = lowerBound; i <= upperBound; i++){
        l2NormOmp += vec[i] * vec[i];
    }
    return l2NormOmp;
}

double IterationMethod() {
    long double* matrixAData = new long double[MATRIX_SIZE * MATRIX_SIZE];
    long double* vecBData = new long double[MATRIX_SIZE];
    long double* vecX = new long double[MATRIX_SIZE];
    long double* vecTemp = new long double[MATRIX_SIZE];

    #pragma omp parallel num_threads(NTHREADS)
    {
        int numThreads = omp_get_num_threads();
        int threadId = omp_get_thread_num();
        int itemsPerThread = MATRIX_SIZE / numThreads;
        int lowerBound = threadId * itemsPerThread;
        int upperBound = (threadId == numThreads - 1) ? (MATRIX_SIZE - 1) : (lowerBound + itemsPerThread - 1);
        for (int i = lowerBound; i <= upperBound; i++) {
            for (int j = 0; j < MATRIX_SIZE; j++)
                matrixAData[i * MATRIX_SIZE + j] = (i == j) ? 2.0 : 1.0;
            vecBData[i] = MATRIX_SIZE+1;
            vecX[i] = 0.0;
        }
    }

    const long double* matrixA = matrixAData; 
    const long double* vecB = vecBData; 

    double l2VecB = 0.0, numerator = 0.0;
    bool stop = false; 

    double startTime = CpuSecond();

    #pragma omp parallel num_threads(NTHREADS)
    {   
        int numThreads = omp_get_num_threads();
        int threadId = omp_get_thread_num();
        int itemsPerThread = MATRIX_SIZE / numThreads;
        int lowerBound = threadId * itemsPerThread;
        int upperBound = (threadId == numThreads - 1) ? (MATRIX_SIZE - 1) : (lowerBound + itemsPerThread - 1);

        double numeratorPart;

        double l2VecBPart =  VecL2NormOmp(vecB, lowerBound, upperBound);
        #pragma omp atomic
        l2VecB += l2VecBPart;

        #pragma omp barrier

        #pragma omp single
        epsilon *= sqrt(l2VecB);
        
        while(!stop) {
            //vecTemp = matrixA * vecX
            MatrixVectorProductOmp(matrixA, vecX, vecTemp, lowerBound, upperBound);

            //vecTemp = matrixA*vecX - b
            SubtractVecFromVec(vecTemp, vecB, lowerBound, upperBound);

            numeratorPart = VecL2NormOmp(vecTemp,  lowerBound, upperBound);

            #pragma omp atomic
            numerator += numeratorPart;

            #pragma omp barrier

            #pragma omp single
            {
                if (sqrt(numerator) < epsilon) {
                    stop = true; 
                }
                numerator = 0.0; 
            }

            #pragma omp barrier

            if (stop) break; 

            //vecTemp = ITERATION_STEP * (matrixA*vecX - b)
            MultiplyVecByScalar(vecTemp, kITERATION_STEP, lowerBound, upperBound);
    
            //vecX -= ITERATION_STEP * (matrixA*vecX - b)
            SubtractVecFromVec(vecX, vecTemp, lowerBound, upperBound);
        }
    }

    double endTime = CpuSecond();

    long double sumAbsoluteError = 0.0;
    long double sumRelativeError = 0.0;

    #pragma omp parallel for num_threads(NTHREADS) schedule(static) reduction(+:sumAbsoluteError,sumRelativeError)
    for (int i = 0; i < MATRIX_SIZE; i++) {
        long double absoluteError = std::abs(vecX[i] - 1.0);
        long double relativeError = std::abs((vecX[i] - 1.0) / 1.0);

        sumAbsoluteError += absoluteError;
        sumRelativeError += relativeError;

        if (vecX[i] != 0)std::cout << vecX[i];
    }

    std::cout << "Sum of absolute errors: " << sumAbsoluteError << std::endl;
    std::cout << "Sum of relative errors: " << sumRelativeError << std::endl;

    delete[] matrixAData;
    delete[] vecBData;
    delete[] vecX;
    delete[] vecTemp;

    return endTime - startTime;
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
