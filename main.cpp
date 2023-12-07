#include <iostream>
#include "matrix.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>



int main() {

    //    int arr[4] = {1, 2 ,3, 4};
    //
    //    auto ptr = arr;
    //
    //    std::cout << *(ptr += 1) << ' ' << *ptr;

    std::srand(std::time(nullptr));
    Matrix<int> matrix(5, 7);

    int i = 0;
    for (auto it = matrix.begin(); it != matrix.end(); it++) {
        *it = ++i;
    }

    std::cout << matrix;
    return 0;
}