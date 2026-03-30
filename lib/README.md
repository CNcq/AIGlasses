# AI Glasses Embedding Matcher - C++ 库 (Linux 版)

## 📦 项目说明

这是一个纯 C++ 实现的语义匹配库，专为 **Linux 环境**设计，用于中文缺陷描述的语义匹配。

### 核心功能
- ✅ Jieba 中文分词
- ✅ Embedding 向量计算
- ✅ 余弦相似度匹配
- ✅ 预计算向量缓存
- ✅ JSON 配置加载
- ✅ JNI 接口支持
- ✅ **Linux 原生支持**

### 支持的 Linux 发行版
- ✅ Ubuntu 18.04+
- ✅ Debian 9+
- ✅ CentOS 7+
- ✅ Fedora 30+
- ✅ Arch Linux
- ✅ openSUSE

---

## 🚀 快速开始

### 1. 安装依赖

**自动安装（推荐）：**
```bash
cd lib
chmod +x setup_linux.sh
./setup_linux.sh
```

**手动安装（Ubuntu/Debian）：**
```bash
sudo apt update
sudo apt install -y cmake make g++ build-essential git wget unzip
```

### 2. 设置 Android NDK

```bash
# 下载 NDK
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip -d $HOME

# 设置环境变量
export ANDROID_NDK=$HOME/android-ndk-r25c
echo 'export ANDROID_NDK=$HOME/android-ndk-r25c' >> ~/.bashrc
source ~/.bashrc
```

### 3. 构建库

```bash
cd lib
./build_android.sh
```

构建完成后，`dist/` 目录就是可以分发的第三方库！

---

## 🏗️ 构建说明

### 构建为动态库 (.so)

```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
```

### 构建为静态库 (.a)

```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=OFF ..
make -j$(nproc)
```

### 交叉编译 for Android

```bash
mkdir build_android && cd build_android
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-21 \
      -DBUILD_SHARED_LIBS=ON \
      ..
make -j$(nproc)
```

---

## 📁 输出文件

构建完成后会生成：
- `libai_glasses.so` - 动态库
- `libai_glasses.a` - 静态库（可选）
- `include/` - 头文件目录

---

## 📱 Android 集成

### 1. 复制库文件

```bash
# 从头文件
cp -r dist/include YourAndroidProject/app/src/main/cpp/

# 复制库文件
cp -r dist/libs YourAndroidProject/app/src/main/jniLibs/
```

### 2. 配置 CMakeLists.txt

```cmake
find_library(ai_glasses-lib 
    ai_glasses 
    PATHS ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}
)

target_link_libraries(your-jni-lib 
    ${ai_glasses-lib}
    ${log-lib}
    android
)
```

### 3. 使用示例

```cpp
#include "embedding_matcher.h"

ai_glasses::EmbeddingMatcher matcher;
matcher.initialize(dict_path, model_path);
matcher.loadDefectConfig(config_path);

auto result = matcher.findBestMatch("墙留坠");
```

---

## 🔧 依赖项

### 必需
- C++14 或更高版本
- CMake 3.10+
- G++ 7.0+
- Make 4.0+

### Android 构建（可选）
- Android NDK r20+

### 自动安装

使用提供的脚本自动安装所有依赖：
```bash
./setup_linux.sh
```

---

## 📖 文档

- [QUICKSTART.md](QUICKSTART.md) - 5 分钟快速开始
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 详细构建指南
- [ANDROID_INTEGRATION.md](ANDROID_INTEGRATION.md) - Android 集成示例

---

## 🎯 特性

### Linux 原生优化
- ✅ 自动检测 Linux 发行版
- ✅ 智能依赖管理
- ✅ 支持主流包管理器（apt, dnf, yum, pacman, zypper）
- ✅ 优化的编译参数

### 跨平台支持
- ✅ 支持多种 Android ABI
- ✅ 支持 x86_64 和 ARM64 架构
- ✅ 兼容各种 Linux 发行版

### 性能优化
- ✅ 多核并行编译
- ✅ 预计算向量缓存
- ✅ 优化的内存使用

---

## 🎉 总结

这是一个专为 Linux 环境优化的 C++ 库，提供了完整的构建工具和文档，让你可以快速在 Android 项目中集成语义匹配功能。

**构建完成后，`dist/` 目录就是可以分发的第三方库！** 🚀

---

## 📝 许可证

MIT License
