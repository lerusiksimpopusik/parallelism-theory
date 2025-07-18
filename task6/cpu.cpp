#include <iostream>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <boost/program_options.hpp>
#include <omp.h>
#include <nvtx3/nvToolsExt.h>

namespace po = boost::program_options;

void initialize(double* A, double* Anew, size_t size) {
    memset(A, 0, size * size * sizeof(double));
    memset(Anew, 0, size * size * sizeof(double));
    A[0] = 10.0;
    A[size-1] = 20.0;
    A[size*(size-1)] = 30.0;
    A[size*size-1] = 20.0;
    //border interpolation
    double top_left = A[0];
    double top_right = A[size-1];
    double bottom_left = A[size*(size-1)];
    double bottom_right = A[size*size-1];
    for (int i = 1; i < size-1; ++i) {
        A[i] = top_left + (top_right-top_left)*i / static_cast<double>(size-1); // top  
        A[size*(size-1) + i] = bottom_left + (bottom_right-bottom_left)*i / static_cast<double>(size-1); // bottom  

        A[size*i] = top_left + (bottom_left-top_left)*i / static_cast<double>(size-1); // left 
        A[size*i + size-1] = top_right + (bottom_right-top_right)*i / static_cast<double>(size-1); // right 
    }
}

double calculate_next_grid(double* restrict A, double *restrict Anew, size_t size) {
    double error = 0.0;
    #pragma acc parallel loop reduction(max:error)
    for (int i = 1; i < size-1; ++i) {
        #pragma acc loop
        for (int j = 1; j < size-1; ++j) {
            Anew[i*size + j] = 0.25 * ( A[(i+1)*size + j] + A[(i-1)*size + j]  
                                           + A[i*size + j-1] + A[i*size + j+1]);  
            error = fmax( error, fabs(Anew[i*size + j] - A[i*size + j]));  
        }
    }
    return error;
}

void copy_matrix(double *restrict A, double *restrict Anew, int size) {
    #pragma acc parallel loop 
    for (int i = 1; i < size-1; i++) {
        #pragma acc loop
        for (int j = 1; j < size-1; j++) {
            A[i * size + j] = Anew[i * size + j];
        }
    }
}

void deallocate(double *restrict A, double *restrict Anew) {
    free(A);
    free(Anew);
}

void print_grid(double* A, size_t size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            std::cout << std::setprecision(4) << A[i*size + j] << "  ";
        }
        std::cout << '\n';
    }
}

int main(int argc, char* argv[]) {

    int size = 128;
    double accuracy;
    int max_iterations;

    po::options_description desc("Options");
    desc.add_options()
        ("help", "show option description")
        ("size", po::value<int>(&size)->default_value(256), "grid size (NxN)")
        ("accuracy", po::value<double>(&accuracy)->default_value(1e-6), "maximum allowed error")
        ("max_iterations", po::value<int>(&max_iterations)->default_value(1e+6), "maximum allowed iterations");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }
    std::cout << "Program started!\n\n";
    double *restrict A = (double *)malloc(sizeof(double) * size * size);
    double *restrict Anew = (double *)malloc(sizeof(double) * size * size);

    double error = accuracy+1.0;
    double iteration = 0;
    
    nvtxRangePushA("init");
    initialize(A, Anew, size); // put all zeros except corners
    nvtxRangePop();
    
    const auto start{std::chrono::steady_clock::now()};
    
    nvtxRangePushA("while");
    while (error > accuracy && iteration < max_iterations) {
        nvtxRangePushA("calc");
        error = calculate_next_grid(A, Anew, size);
        nvtxRangePop();
        
        nvtxRangePushA("copy");
        copy_matrix(A, Anew, size);
        nvtxRangePop();
        iteration++;
    }  
    nvtxRangePop();
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};

    std::cout << "Time:  " << elapsed_seconds.count() << " sec\n";
    std::cout << "Iterations:  " << iteration << "\n";
    std::cout << "Error value:  " << error << std::endl;
    deallocate(A, Anew);
    return 0;
}