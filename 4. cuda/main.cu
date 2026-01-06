#include <iostream>
#include <vector>

__global__ void calculateSquare(int* d_input, int* d_output, const int N, int* d_maxSquare) {
    int threadId = blockIdx.x * blockDim.x + threadIdx.x;
    int stride = blockDim.x * gridDim.x;

    for (int i = threadId; i < N; i += stride) {
        d_output[i] = d_input[i] * d_input[i];
        atomicMax(d_maxSquare, d_output[i]);
    }
}

int main(int argc, char** argv) {
    int N = 100;
    std::vector<int> input(N);
    std::vector<int> output(N);
    int maxSquare = -1;

    for (int i = 0; i < N; i++) {
        input[i] = i;
    }

    int* d_input;
    int* d_output;
    int* d_maxSquare;

    cudaMalloc(&d_input, N * sizeof(int));
    cudaMalloc(&d_output, N * sizeof(int));
    cudaMalloc(&d_maxSquare, sizeof(int));

    cudaMemcpy(d_input, input.data(), N * sizeof(int), cudaMemcpyHostToDevice);

    calculateSquare<<<2, 5>>>(d_input, d_output, N, d_maxSquare);
    cudaDeviceSynchronize();

    cudaMemcpy(output.data(), d_output, N * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(&maxSquare, d_maxSquare, sizeof(int), cudaMemcpyDeviceToHost);

    std::cout << "Max square = " << maxSquare << std::endl;
    std::cout << "Results:" << std::endl;
    for (int i = 0; i < N; i++) {
        std::cout << input[i] << "^2 = " << output[i] << std::endl;
    }

    cudaFree(d_input);
    cudaFree(d_output);
    cudaFree(d_maxSquare);

    return 0;
}