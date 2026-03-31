# Game_2048_Quick（2048 Pro Max）

这是一个把 **2048 游戏规则抽象为“高维张量/棋盘运算”** 的学习型工程项目：

- 支持自定义棋盘维度与边长（例如经典 2D $4\times4$，以及更高维/更大尺寸）
- C++20 + 模板元编程（TMP）作为高维索引与派发的基础
- 异构计算：CPU + CUDA（工程中预留国产算力迁移思路）
- 交互与可视化：Qt 6（Qt Quick / QML / Quick3D）

未来拓展路线图见：FUTURE.md。

## 技术栈与依赖

- C++：STD C++20
- 构建系统：CMake（项目 `cmake_minimum_required(VERSION 3.30.1)`）
  - 我们的测试平台为 CMake 4.3。因此我们无法保证其能够在较低版本中正常工作。
  - 我们唯一能确定的是需要选择支持 C++20 除模块外所有标准特性的 CMake 版本。 
- UI：Qt 6.9.3（MSVC2022 64-bit）
- GPU：CUDA Toolkit（已验证缓存环境为 CUDA v13.0.88）
  - 当前测试平台理论适配 NVIDIA GeForce RTX 50 Series
  - 可根据主机需要自行调整 CMake 相关参数
  - 暂不支持 AMD/Intel GPU 以及昇腾 CANN，未来考虑拓展后者。 
- 数学/张量：Eigen（仓库内 `include/Eigen`）

Qt 组件（CMake 里链接）：Core / Gui / Widgets / Quick / Qml / Quick3D。

## 环境准备（Windows / MSVC2022）

### 1) 安装 Visual Studio 2022

需要组件：

- Desktop development with C++
- MSVC v143 or v145（x64） 
- Windows 10/11 SDK

### 2) 安装 Qt 6.9.3（MSVC2022_64）

本项目当前在 CMake 中硬编码了 Qt 路径，需要根据实际环境修改。

- `D:/Qt/6.9.3/msvc2022_64`

如果你的 Qt 安装路径不同，请修改根目录 CMakeLists.txt 中的：

- `set(CMAKE_PREFIX_PATH "D:/Qt/6.9.3/msvc2022_64")`

### 3) 安装 CUDA Toolkit

本项目使用：

- `find_package(CUDAToolkit REQUIRED)`

已存在的构建缓存显示开发机为：

- CUDA Toolkit v13.0.88（`nvcc.exe` 位于 `.../CUDA/v13.0/bin/`）

注意：仅安装显卡驱动通常不足以提供 `cudart` 开发库；开发机需要安装 CUDA Toolkit。

### 4) 配置 CUDA 架构（

根目录 CMakeLists.txt 当前设置：

- `set(CMAKE_CUDA_ARCHITECTURES 90)`

这表示默认以 **SM90（算力 9.0）** 编译（适用于 NVIDIA GeForce RTX 50 Series GPU）。如果你的显卡不是 SM90，请改成你的实际算力，例如：

- `75`（Turing） / `80`、`86`（Ampere） / `89`（Ada 部分型号） / `90`（Hopper / Blackwell 部分）

你也可以用“先改成你确定的数值”来避免 NVCC 报错。

## 构建（命令行）

项目已在 Ninja 生成器下工作良好（现有构建缓存使用 Ninja）。推荐使用 Ninja（单配置）以匹配当前 CMake 中的 Debug/Release 处理方式。

### Debug 构建

在仓库根目录执行：

1) 生成：

- `cmake -S . -B out/build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug`

2) 编译：

- `cmake --build out/build-debug`

### Release 构建

1) 生成：

- `cmake -S . -B out/build-release -G Ninja -DCMAKE_BUILD_TYPE=Release`

2) 编译：

- `cmake --build out/build-release`

### 运行

可执行文件位于构建目录（例如 `out/build-debug/`）下。

如果 Qt 运行时部署正确，双击或在终端运行 exe 即可。

## 部署/打包（Windows）

项目的 CMakeLists.txt 在 Windows 下包含了部署逻辑：

1) 复制 Qt DLL 到 exe 同目录
2) 复制 `qwindows.dll` 到 `plugins/platforms/`
3) 若能找到 `windeployqt`，则在 **POST_BUILD** 阶段自动执行 `windeployqt`，把 Qt 运行时与 QML 模块一起部署到输出目录
4) 若提示 "does not seem to be a Qt executable"，请考虑使用 `windeployqt6` 代替 `windeployqt`。
5) 若部署工具无法运行，考虑是否将其加入到环境变量。

### 自动部署（推荐）

正常情况下，只要 Qt 安装完整且 `windeployqt` 位于 Qt 的 `bin/` 目录，编译完成后输出目录已经具备运行所需的 Qt 运行时文件。

分发时：

- 直接把“exe 所在的整个输出目录”打包成 zip（包含 `plugins/` 与相关 Qt DLL）

### 手动部署（当 CMake 输出警告找不到 windeployqt）

如果你看到类似 “windeployqt not found …” 的警告，可以手动执行：

- `D:/Qt/6.9.3/msvc2022_64/bin/windeployqt.exe --no-translations --qmldir src/qml path\\to\\Game_2048_Quick.exe`

并确认输出目录存在：

- `plugins/platforms/qwindows.dll`

### CUDA 运行时说明

本项目链接 `CUDA::cudart` 与 `CUDA::cuda_driver`：

- 目标机器需要安装合适版本的 NVIDIA 驱动
- 若目标机未安装 CUDA Toolkit，可能还需要随程序一起分发 `cudart64_*.dll`（具体文件名与版本取决于你的 CUDA Toolkit）

## 常见问题（排查清单）

### 1) 找不到 Qt / Qt6Config.cmake

- 确认 CMakeLists.txt 中 `CMAKE_PREFIX_PATH` 指向你的 Qt 安装目录（应包含 `lib/cmake/Qt6`）

### 2) CUDA 编译失败 / 架构不匹配

- 调整 `CMAKE_CUDA_ARCHITECTURES` 为你的 GPU 算力

### 3) 运行时报 QML 模块缺失

- 确认 `windeployqt` 成功执行
- 若手动部署，确保 `--qmldir src/qml` 指向项目 QML 目录

## 目录结构速览

- `src/cpp/`：C++ 核心逻辑、CUDA 接口、游戏驱动
- `src/qml/`：Qt Quick / QML 视图
- `src/resources/`：资源（qrc）
- `include/Eigen/`：Eigen 依赖
- `test/`：基准/实验代码（benchmark）
