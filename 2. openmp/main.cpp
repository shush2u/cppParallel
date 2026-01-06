#include <iostream>
#include <vector>
#include <omp.h>

int calculateSumOfSquares(std::vector<int> *numbers) {
    int result = 0;

    #pragma omp parallel for default(none) num_threads(4) shared(numbers, std::cout) reduction(+:result)
    for (int i = 0; i < numbers->size(); i++) {
        #pragma omp critical
        {
            std::cout << "(TID " << omp_get_thread_num() << ") " << result << " += " << (*numbers)[i] << "^2 = " << (*numbers)[i] * (*numbers)[i] << std::endl;
        }
        result += (*numbers)[i] * (*numbers)[i];
    }

    return result;
}

int main(int argc, char** argv) {

    int n;
    std::cout << "Enter integer > 0:" << std::endl;
    std::cin >> n;
    std::vector<int> numbers(n);
    
    for (int i = 0; i < n; i++) {
        numbers[i] = 2 * i;
    }    

    int result = calculateSumOfSquares(&numbers);

    std::cout << "Sum of squares of the first " << n << " even numbers" << ": " << result << std::endl;

    return 0;
}