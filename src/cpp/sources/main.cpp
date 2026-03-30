#include <QApplication>
#include <QPushButton>
#include <iostream>
#include <cuda_runtime.h>
#include "logic_2048_pm.h"


#define OUTPUT_STREAM std::cout


#define CHECK_CUDA_ERROR(err) \
if (err != cudaSuccess) { \
    OUTPUT_STREAM << "CUDA ERROR: " << cudaGetErrorString(err) << " (Line: " << __LINE__ << ")" << std::endl; \
    return 1; \
}


int testCuda()
{
    int device_count = 0;
    // 1. 查询CUDA设备数量
    CHECK_CUDA_ERROR(cudaGetDeviceCount(&device_count));

    if (device_count == 0) {
        OUTPUT_STREAM << "Failed to find NVIDIA GPU for CUDA! " << std::endl;
        return 1;
    }

    OUTPUT_STREAM << "=== FIND: " << device_count << " CUDA Devices" << std::endl;

    // 2. 获取0号设备（RTX 5070）信息
    cudaDeviceProp prop{};
    CHECK_CUDA_ERROR(cudaGetDeviceProperties(&prop, 0));

    // 打印关键硬件信息
    OUTPUT_STREAM << "Device Name: " << prop.name << std::endl;
    OUTPUT_STREAM << "Computing Power: " << prop.major << "." << prop.minor << std::endl;
    OUTPUT_STREAM << "Global Memory: " << prop.totalGlobalMem / 1024 / 1024 << " MB" << std::endl;
    OUTPUT_STREAM << "=====================================" << std::endl;
    OUTPUT_STREAM << "CUDA Toolkit Test Successfully! " << std::endl;


    return 0;
}

int main(int argc, char* argv[]) {
    // QApplication a(argc, argv);
    Logic2048_tm<size_t, size_t, 3, 4, 4, 4> TestObject{};
    TestObject.operate(3, MoveDirection::Negative);
    testCuda();
    return 0;

    // return QApplication::exec();
}
