# AI Glasses C++ 库 - Linux 快速开始

## 🚀 5 分钟快速构建 (Linux)

本指南专为 Linux 环境设计，支持 Ubuntu、Debian、CentOS、Fedora、Arch 等发行版。

---

## 📋 前提条件

### 1. 系统要求

- **操作系统**: Linux (Ubuntu 18.04+, CentOS 7+, Fedora 30+ 等)
- **架构**: x86_64 或 ARM64
- **内存**: 至少 4GB RAM（推荐 8GB）
- **磁盘**: 至少 5GB 可用空间

### 2. 必需工具

- **CMake**: 3.10 或更高版本
- **Make**: 4.0 或更高版本
- **G++**: 7.0 或更高版本
- **Git**: 2.0 或更高版本（推荐）

### 3. Android NDK

- **版本**: r20 或更高版本（推荐 r25c）
- **下载**: [Android NDK Downloads](https://developer.android.com/ndk/downloads)

---

## 🔧 快速安装

### 方法一：自动配置（推荐）

我们提供了自动配置脚本，一键安装所有依赖：

```bash
cd lib

# 添加执行权限
chmod +x setup_linux.sh

# 运行配置脚本
./setup_linux.sh
```

脚本会自动：
- ✅ 检测 Linux 发行版
- ✅ 安装必需的构建工具
- ✅ （可选）下载并安装 Android NDK
- ✅ 验证安装

### 方法二：手动安装

#### Ubuntu/Debian

```bash
# 安装构建工具
sudo apt update
sudo apt install -y cmake make g++ build-essential git wget unzip

# 下载 NDK（如果需要）
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip -d $HOME
rm android-ndk-r25c-linux.zip

# 设置环境变量
export ANDROID_NDK=$HOME/android-ndk-r25c
echo 'export ANDROID_NDK=$HOME/android-ndk-r25c' >> ~/.bashrc
source ~/.bashrc
```

#### CentOS/RHEL/Fedora

```bash
# Fedora
sudo dnf install -y cmake make gcc-c++ git wget unzip

# CentOS/RHEL
sudo yum install -y cmake make gcc-c++ git wget unzip

# 下载并安装 NDK（同上）
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip -d $HOME
export ANDROID_NDK=$HOME/android-ndk-r25c
```

#### Arch Linux/Manjaro

```bash
sudo pacman -S --noconfirm cmake make gcc git wget unzip

# 下载并安装 NDK
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip -d $HOME
export ANDROID_NDK=$HOME/android-ndk-r25c
```

#### openSUSE

```bash
sudo zypper install -y cmake make gcc-c++ git wget unzip
# 下载并安装 NDK（同上）
```

---

## 🏗️ 构建库

### 1. 验证环境

```bash
cd lib

# 检查环境和依赖
./build_android.sh check
```

输出示例：
```
========================================
AI Glasses Android 库构建 (Linux)
========================================

[STEP] 检查系统环境...
[INFO] 系统环境检查通过
[INFO]   CMake: cmake version 3.22.1
[INFO]   Make: GNU Make 4.3
[INFO]   G++: g++ (Ubuntu 11.2.0-19ubuntu1) 11.2.0

[STEP] 检查 Android NDK...
[INFO] 使用 NDK: /home/user/android-ndk-r25c (版本：25.2.9519653)
```

### 2. 一键构建

```bash
# 完整构建（清理 + 编译 + 打包）
./build_android.sh
```

### 3. 分步构建

```bash
# 只构建
./build_android.sh build

# 只打包
./build_android.sh package

# 清理
./build_android.sh clean
```

---

## 📦 构建产物

构建完成后会生成 `dist/` 目录：

```
dist/
├── include/              # 头文件
│   ├── embedding_matcher.h
│   ├── embedding_model.h
│   ├── jieba_segmenter.h
│   └── ...
└── libs/
    ├── arm64-v8a/
    │   └── libai_glasses.so
    ├── armeabi-v7a/
    │   └── libai_glasses.so
    ├── x86/
    │   └── libai_glasses.so
    └── x86_64/
        └── libai_glasses.so
```

**这个 `dist/` 目录就是可以分发的第三方库！**

---

## 📱 集成到 Android 项目

### 1. 复制库文件

```bash
# 从头文件
cp -r dist/include YourAndroidProject/app/src/main/cpp/

# 复制库文件
cp -r dist/libs YourAndroidProject/app/src/main/jniLibs/
```

### 2. 配置 CMakeLists.txt

在 Android 项目的 `app/CMakeLists.txt` 中添加：

```cmake
# 查找预编译的库
find_library(ai_glasses-lib 
    ai_glasses 
    PATHS ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}
)

# 你的 JNI 库
add_library(your-jni-lib SHARED src/main/cpp/your-jni.cpp)

# 链接库
target_link_libraries(your-jni-lib 
    ${ai_glasses-lib}
    ${log-lib}
    android
)
```

### 3. 使用示例

```java
// Java 代码
public class MainActivity extends AppCompatActivity {
    
    static {
        System.loadLibrary("your-jni-lib");
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // 使用 JNI 调用
        String result = matchText("墙留坠");
        textView.setText(result);
    }
    
    public native String matchText(String input);
}
```

---

## 🔍 故障排查

### 问题 1: 找不到 NDK

**错误信息:**
```
[ERROR] Android NDK 未找到
```

**解决方法:**
```bash
# 设置 NDK 路径
export ANDROID_NDK=/path/to/android-ndk

# 永久设置
echo 'export ANDROID_NDK=/path/to/android-ndk' >> ~/.bashrc
source ~/.bashrc
```

### 问题 2: CMake 版本过低

**错误信息:**
```
CMake 3.10 or higher is required
```

**解决方法:**

Ubuntu 18.04:
```bash
# 使用 Snap 安装最新版 CMake
sudo snap install cmake --classic

# 或使用 Kitware 的 APT 仓库
sudo apt-get install -y software-properties-common lsb-release
sudo apt-get install -y wget
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
    | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
sudo apt-get update
sudo apt-get install -y cmake
```

### 问题 3: 编译时内存不足

**症状:**
编译过程中系统卡死或 OOM

**解决方法:**
```bash
# 限制并行编译的任务数
cd build_android/arm64-v8a
make -j2  # 只使用 2 个核心
```

### 问题 4: 找不到 g++

**错误信息:**
```
g++: command not found
```

**解决方法:**
```bash
# Ubuntu/Debian
sudo apt install -y g++

# CentOS/Fedora
sudo dnf install -y gcc-c++

# Arch
sudo pacman -S gcc
```

---

## 🎯 高级用法

### 只构建特定 ABI

如果你只关心特定的架构：

```bash
# 编辑构建脚本，修改 ABIS 数组
# 或者手动构建单个 ABI

cd lib
mkdir -p build_android/arm64-v8a
cd build_android/arm64-v8a

cmake ../.. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
make install
```

### 调试版本

```bash
./build_android.sh clean

# 手动配置调试版本
cd build_android
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_SHARED_LIBS=ON

make -j$(nproc)
```

### 静态库

```bash
# 修改 CMakeLists.txt
# 将 option(BUILD_SHARED_LIBS "Build shared libraries" ON)
# 改为 OFF

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DBUILD_SHARED_LIBS=OFF

make -j$(nproc)
```

---

## 📚 相关文档

- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 详细构建指南
- [ANDROID_INTEGRATION.md](ANDROID_INTEGRATION.md) - Android 集成示例
- [README.md](README.md) - 库的基本说明

---

## 💡 提示

1. **首次构建时间**: 完整构建所有 ABI 大约需要 10-15 分钟
2. **推荐 ABI**: 现代 Android 设备主要使用 `arm64-v8a`
3. **开发建议**: 开发阶段可以只构建 `arm64-v8a` 以节省时间
4. **性能优化**: 使用 `-j$(nproc)` 充分利用多核 CPU

---

## 🎉 总结

现在你已经成功在 Linux 环境下构建了 AI Glasses C++ 库！

**下一步**: 将 `dist/` 目录集成到你的 Android 项目中！ 🚀
