#include <QApplication>
#include <QPushButton>
#include <iostream>

#include "logic_2048_pm.h"

#ifndef CUDA_UNAVAILABLE
#include <cuda_runtime.h>
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
#else

int testCuda()
{
    return 0;
}

#endif
#define OUTPUT_STREAM std::cout


int main(int argc, char *argv[])
{
    // QApplication a(argc, argv);


    size_t custom_values[64] = {
        0, 0, 0, 2, 0, 0, 0, 2,
        0, 2, 2, 0, 2, 2, 2, 2,

        0, 0, 4, 4, 2, 2, 4, 8,
        4, 4, 8, 2, 0, 0, 0, 0,

        0, 0, 0, 2, 0, 0, 0, 2,
        0, 2, 2, 0, 2, 2, 2, 2,

        2, 2, 4, 8, 0, 0, 0, 2,
        0, 2, 2, 0, 2, 2, 2, 2,

    };

    // 步骤2: 创建4x4x4 Tensor
    Logic2048_tm<size_t, size_t, 2, 8, 8>::data_mesh_type_ tensor(8ull, 8ull)
    // 步骤3: 复制数据（确保大小匹配）
    std::memcpy(tensor.data(), custom_values, sizeof(custom_values));


    Logic2048_tm<size_t, size_t, 2, 8, 8> testObject{tensor};

    testCuda();
    testObject.outputData();


    testObject.operate(1, MoveDirection::Negative);

    std::cout << "\n\n\n";
    testObject.outputData();
    return 0;

    // return QApplication::exec();
}
