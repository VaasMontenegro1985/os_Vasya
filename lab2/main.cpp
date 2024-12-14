#include <iostream> 
#include <thread> 
#include <chrono> 
#include <algorithm> 
#include <cstring> 
#include <mutex> 
#include <math.h> 
#include <vector> 
#include <limits> 
#include <queue> 

#define KERNEL_SIZE 3 

typedef std::pair<int, int> Task; 
 
const std::vector<double> kernel = { 
    1.0, 0.0, -1.0,
    1.0, 0.0, -1.0,
    1.0, 0.0, -1.0
}; 
 
void convolution (std::vector<double> &matrix, std::vector<double> &res, size_t rows, size_t cols, size_t x, size_t y) { 
    double s = 0.0; 
 
    for (int i = 0; i < KERNEL_SIZE; i++) { 
        for (int j = 0; j < KERNEL_SIZE; j++) { 
            size_t matX = x + j - KERNEL_SIZE / 2; 
            size_t matXLimited = std::max(0ul, std::min(cols - 1, matX)); 
            size_t matY = y + i - KERNEL_SIZE / 2; 
            size_t matYLimited = std::max(0ul, std::min(rows - 1, matY)); 
 
            s += kernel[i * KERNEL_SIZE + j] * matrix[matYLimited * cols + matXLimited]; 
        } 
    } 
 
    res[y * cols + x] = s; 
} 
 
void convolutionThread (int i, std::vector<double> &matrix, std::vector<double> &res, size_t rows, size_t cols, std::queue<Task> pixelsCalcTasks) { 
    Task task; 
 
    while (!pixelsCalcTasks.empty()) { 
        task = pixelsCalcTasks.front(); 
        pixelsCalcTasks.pop(); 
 
        convolution(matrix, res, rows, cols, task.first, task.second); 
    } 
     
} 
 
void applyFilter (std::vector<double> &matrix, std::vector<double> &res, size_t rows, size_t cols, int maxThreads) { 
    std::vector<std::thread> threads;
    std::vector<std::queue<Task>> tasksQueues(maxThreads); 
    std::queue<Task> pixelsCalcTasks; 
 
    for (int i = 0; i < rows; i++) { 
        for (int j = 0; j < cols; j++) { 
            tasksQueues[(i * cols + j) % maxThreads].push(std::pair(j, i)); 
        } 
    } 

    threads.clear(); 

    for (int i = 0; i < maxThreads; i++) { 
        threads.emplace_back(std::thread( 
            convolutionThread,  
            i, 
            std::ref(matrix),  
            std::ref(res),  
            rows,  
            cols,
            std::ref(tasksQueues[i]) 
        )); 
    } 

    for (int i = 0; i < maxThreads; i++) threads[i].join(); 
} 
 
int main(int argc, char const *argv[]) { 
    if (argc < 2) return 1; 
 
    int hardwareThreads = std::thread::hardware_concurrency(); 
    int maxThreads = std::min(atoi(argv[1]), hardwareThreads); 

    int k, rows, cols; 
     
    std::cout << "Input num to apply: "; 
    std::cin >> k; 
 
    std::cout << "Input matrix rows count: "; 
    std::cin >> rows; 
 
    std::cout << "Input matrix cols count: "; 
    std::cin >> cols; 
 
    std::vector<double> matrix(rows * cols); 
    std::vector<double> resMatrix(rows * cols); 
 
    for (int i = 0; i < rows; i++) { 
        for (int j = 0; j < cols; j++) { 
            //std::cout << "Input element (" << i << "; " << j << "): "; 
            std::cin >> matrix[i * cols + j]; 
        } 
    } 
 
    auto t1 = std::chrono::high_resolution_clock::now(); 
    for (int i = 0; i < k; i++) { 
        if (i % 2 == 0) applyFilter(matrix, resMatrix, rows, cols, maxThreads); 
        else applyFilter(resMatrix, matrix, rows, cols, maxThreads); 
    } 
 
    if (k % 2 == 0) { 
        for (int i = 0; i < resMatrix.size(); i++) resMatrix[i] = matrix[i]; 
    }  

    auto t2 = std::chrono::high_resolution_clock::now(); 

// std::cout << std::endl; 
// for (int i = 0; i < rows; i++) { 
//     for (int j = 0; j < cols; j++) { 
//         std::cout << resMatrix[i * cols + j] << "\t\t\t"; 
//     } 
//     std::cout << std::endl; 
// } 
 
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    std::cout << "Execution takes " << duration << " micro secs" << std::endl; 
 
    return 0; 
} 
 
// 88704