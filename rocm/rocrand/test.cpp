#include <hip/hip_runtime.h>
#include <rocrand/rocrand.hpp>
#include <vector>
#include <numeric>
#include <cmath>
#include <iostream>

int main()
{
    size_t size = 1024 * 1024;
    float mean = -1.24f;
    float std = 0.43f;
    rocrand_generator gen;
    rocrand_create_generator(&gen, ROCRAND_RNG_PSEUDO_XORWOW);

    float *x;
    hipMalloc((void**)&x, sizeof *x * size);
    rocrand_generate_normal(gen, x, size, mean, std);

    std::vector<float> x_d(size);
    hipMemcpy(x_d.data(), x, sizeof *x * size, hipMemcpyDeviceToHost);

    float mean_hat = std::accumulate(x_d.begin(), x_d.end(), 0.0f) / size;

    // Tolerance set so that test may at most fail in 1 of 10,000 runs
    float tol = 3e-1;
    if(std::abs(mean - mean_hat) > tol){
        std::cout << "Tolerance in mean not reached:\n"
            << mean_hat << " differs more than " << tol
            << " from " << mean << std::endl;
        return 1;
    }
    std::cout << "TESTS PASSED!" << std::endl;

    rocrand_destroy_generator(gen);
    hipFree(x);
}
