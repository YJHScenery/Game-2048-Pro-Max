cmake_minimum_required(VERSION 4.1.2)
project(Game_2048_Quick LANGUAGES CXX CUDA)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(HPC_PLATFORM CUDA)

if(HPC_PLATFORM MATCHES "CUDA")

    set(CMAKE_CUDA_STANDARD 20)          # CUDA C++17
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    set(CMAKE_CUDA_ARCHITECTURES 90)     # ✅ RTX 5070 专属算力9.0（关键！）
    # set(CMAKE_CUDA_COMPILER cl.exe)      # Windows指定VS编译器

    # ===================== 3. 找到CUDA工具包（自动搜索） =====================
    find_package(CUDAToolkit REQUIRED)

    set(CUDA_SOURCE_FILES
            src/cpp/cuda/cuda_interface.cu
    )

    set(CANN_SOURCE_FILES
    )

elseif (HPC_PLATFORM MATCHES "CANN")

    set(CUDA_SOURCE_FILES
    )
    set(CANN_SOURCE_FILES
    )

else()
    set(CUDA_SOURCE_FILES
    )
    set(CANN_SOURCE_FILES
    )

endif ()

set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/msvc2022_64")


include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Eigen
)


set(CPP_HEADER_FILES
        src/cpp/headers/logic_2048_pm.h
        src/cpp/headers/cuda_interface.h
        src/cpp/headers/tmp_utility.h
        src/cpp/headers/hpc_interface.h
        src/cpp/headers/cann_interface.h
        src/cpp/headers/cpu_interface.h
        src/cpp/headers/basic_dependency.h
)

set(CPP_SOURCE_FILES
        src/cpp/sources/logic_2048_pm.cpp
        src/cpp/sources/main.cpp
)


set(RESOURCE_FILES
        src/resources/resources.qrc
        src/qml/qml.qrc

)


set(QT_PACKAGES_LIST
        Core Gui Widgets Quick Qml
)


find_package(Qt6 COMPONENTS
        ${QT_PACKAGES_LIST}
        REQUIRED)

add_executable(${PROJECT_NAME}
        ${CPP_HEADER_FILES}
        ${CPP_SOURCE_FILES}
        ${CUDA_SOURCE_FILES}
        ${CANN_SOURCE_FILES}
        ${RESOURCE_FILES}
        )

foreach (QT_LIB ${QT_PACKAGES_LIST})
    target_link_libraries(${PROJECT_NAME} PRIVATE
            Qt::${QT_LIB}
    )
endforeach ()

if(HPC_PLATFORM MATCHES "CUDA")
# ===================== 6. 链接CUDA核心库 =====================
target_link_libraries(${PROJECT_NAME} PRIVATE CUDA::cudart CUDA::cuda_driver)
elseif (HPC_PLATFORM MATCHES "CANN")
else ()
endif ()

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB ${QT_PACKAGES_LIST})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()


