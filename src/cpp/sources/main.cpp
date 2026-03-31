#include <QGuiApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <cstring>
#include <iostream>
#include <string>

#include "logic_2048_pm.h"

#define OUTPUT_STREAM std::cout

#ifndef CUDA_UNAVAILABLE
#include <cuda_runtime.h>

#define CHECK_CUDA_ERROR(err)                                                                                     \
    if (err != cudaSuccess)                                                                                       \
    {                                                                                                             \
        OUTPUT_STREAM << "CUDA ERROR: " << cudaGetErrorString(err) << " (Line: " << __LINE__ << ")" << std::endl; \
        return 1;                                                                                                 \
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

static int runBenchmarks()
{
    size_t custom_values[64] = {
        0, 0, 0, 2,
        0, 0, 0, 2,
        0, 2, 2, 0,
        2, 2, 2, 2,

        0, 0, 4, 4,
        2, 2, 4, 8,
        4, 4, 8, 2,
        0, 0, 0, 0,

        0, 0, 0, 2,
        0, 0, 0, 2,
        0, 2, 2, 0,
        2, 2, 2, 2,

        2, 2, 4, 8,
        0, 0, 0, 2,
        0, 2, 2, 0,
        2, 2, 2, 2,
    };

    Logic2048_tm<size_t, size_t, 2, 8, 8>::data_mesh_type_ tensor(8ull, 8ull);
    std::memcpy(tensor.data(), custom_values, sizeof(custom_values));

    Logic2048_tm<size_t, size_t, 2, 8, 8> testObject{tensor};

    testCuda();
    testObject.outputData();

    testObject.operate(1, MoveDirection::Negative);

    std::cout << "\n\n\n";
    testObject.outputData();

    void benchmark_logic2048_tensor_6d_operate();
    benchmark_logic2048_tensor_6d_operate();

    void benchmark_logic2048_tensor_operate_custom();
    benchmark_logic2048_tensor_operate_custom();

    return 0;
}

int main(int argc, char* argv[])
{
    if (argc >= 2 && std::string(argv[1]) == "--bench") {
        return runBenchmarks();
    }

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.addImportPath(QCoreApplication::applicationDirPath() + "/qml");
    const QUrl url(QStringLiteral("qrc:/qml/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl)
        {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }
    return app.exec();
}
