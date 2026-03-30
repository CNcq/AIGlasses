#!/bin/bash

# AI Glasses Android 库构建脚本 (Linux 优化版)
# 适用于 Linux 环境，支持 Ubuntu、CentOS、Fedora 等发行版

set -e

# ============================================================================
# 配置变量
# ============================================================================
ANDROID_NDK=${ANDROID_NDK:-"$HOME/android-ndk"}
BUILD_DIR=${BUILD_DIR:-"build_android"}
INSTALL_DIR=${INSTALL_DIR:-"install"}
MIN_API=21

# 支持的 ABI
ABIS=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# ============================================================================
# 辅助函数
# ============================================================================

echo_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# 检查命令是否存在
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 获取 Linux 发行版信息
get_linux_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    elif [ -f /etc/redhat-release ]; then
        echo "rhel"
    else
        echo "unknown"
    fi
}

# ============================================================================
# 环境检查
# ============================================================================

check_environment() {
    echo_step "检查系统环境..."
    
    local missing_deps=()
    
    # 检查必需工具
    if ! command_exists cmake; then
        missing_deps+=("cmake")
    fi
    
    if ! command_exists make; then
        missing_deps+=("make")
    fi
    
    if ! command_exists g++; then
        missing_deps+=("g++")
    fi
    
    # 检查可选工具
    if ! command_exists git; then
        echo_warn "Git 未安装（推荐安装）"
    fi
    
    # 报告缺失的依赖
    if [ ${#missing_deps[@]} -gt 0 ]; then
        echo_error "缺少以下依赖："
        for dep in "${missing_deps[@]}"; do
            echo "  - $dep"
        done
        
        local distro=$(get_linux_distro)
        echo ""
        echo_info "安装方法："
        
        case "$distro" in
            ubuntu|debian|linuxmint)
                echo "  sudo apt update && sudo apt install -y ${missing_deps[*]}"
                ;;
            fedora)
                echo "  sudo dnf install -y ${missing_deps[*]}"
                ;;
            centos|rhel|rocky)
                echo "  sudo yum install -y ${missing_deps[*]}"
                ;;
            arch|manjaro)
                echo "  sudo pacman -S ${missing_deps[*]}"
                ;;
            opensuse|suse)
                echo "  sudo zypper install -y ${missing_deps[*]}"
                ;;
            *)
                echo "  请使用您的包管理器安装"
                ;;
        esac
        exit 1
    fi
    
    echo_info "系统环境检查通过"
    echo_info "  CMake: $(cmake --version | head -n1)"
    echo_info "  Make: $(make --version | head -n1)"
    echo_info "  G++: $(g++ --version | head -n1)"
}

# 检查 NDK
check_ndk() {
    echo_step "检查 Android NDK..."
    
    if [ ! -d "$ANDROID_NDK" ]; then
        echo_error "Android NDK 未找到：$ANDROID_NDK"
        echo ""
        echo_info "请设置 ANDROID_NDK 环境变量："
        echo "  export ANDROID_NDK=/path/to/android-ndk"
        echo ""
        echo_info "或者安装 NDK："
        echo "  # 方法 1: 使用 Android Studio SDK Manager"
        echo "  # 方法 2: 手动下载并解压到 ~/android-ndk"
        echo ""
        echo_info "下载地址：https://developer.android.com/ndk/downloads"
        exit 1
    fi
    
    # 检查 NDK 版本
    if [ -f "$ANDROID_NDK/source.properties" ]; then
        local ndk_version=$(grep "Pkg.Revision" "$ANDROID_NDK/source.properties" | cut -d'=' -f2)
        echo_info "使用 NDK: $ANDROID_NDK (版本：$ndk_version)"
    else
        echo_info "使用 NDK: $ANDROID_NDK"
    fi
    
    # 检查工具链文件
    if [ ! -f "$ANDROID_NDK/build/cmake/android.toolchain.cmake" ]; then
        echo_error "未找到 CMake 工具链文件"
        exit 1
    fi
}

# ============================================================================
# 构建函数
# ============================================================================

# 清理构建目录
clean() {
    echo_step "清理构建目录..."
    rm -rf "$BUILD_DIR"
    rm -rf "$INSTALL_DIR"
    rm -rf "dist"
    echo_info "清理完成"
}

# 构建单个 ABI
build_abi() {
    local abi=$1
    local abi_build_dir="$BUILD_DIR/$abi"
    
    echo_step "构建 ABI: $abi"
    
    mkdir -p "$abi_build_dir"
    cd "$abi_build_dir"
    
    # 配置 CMake
    cmake ../.. \
        -G "Unix Makefiles" \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI="$abi" \
        -DANDROID_PLATFORM="android-$MIN_API" \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DBUILD_ANDROID_JNI=ON \
        -DBUILD_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX="../../$INSTALL_DIR/$abi"
    
    # 编译
    local cpu_count=$(nproc)
    echo_info "使用 $cpu_count 个 CPU 核心进行编译..."
    make -j$cpu_count
    
    # 安装
    make install
    
    cd ../..
    
    echo_info "✓ $abi 构建成功"
}

# 构建所有 ABI
build_all() {
    echo_step "开始构建所有 ABI..."
    
    for abi in "${ABIS[@]}"; do
        build_abi "$abi"
    done
    
    echo_info "✓ 所有 ABI 构建完成"
}

# 打包输出
package() {
    echo_step "打包库文件..."
    
    local package_dir="dist"
    mkdir -p "$package_dir"
    
    # 复制头文件
    if [ -d "$INSTALL_DIR/arm64-v8a/include" ]; then
        cp -r "$INSTALL_DIR/arm64-v8a/include" "$package_dir/"
        echo_info "  ✓ 复制头文件"
    fi
    
    # 复制所有 ABI 的 so 文件
    mkdir -p "$package_dir/libs"
    for abi in "${ABIS[@]}"; do
        if [ -d "$INSTALL_DIR/$abi/lib" ]; then
            mkdir -p "$package_dir/libs/$abi"
            cp "$INSTALL_DIR/$abi/lib/"*.so "$package_dir/libs/$abi/"
            echo_info "  ✓ 复制 $abi 库文件"
        fi
    done
    
    # 复制文档
    if [ -f "README.md" ]; then
        cp "README.md" "$package_dir/"
    fi
    
    if [ -f "LICENSE" ]; then
        cp "LICENSE" "$package_dir/"
    fi
    
    # 创建使用说明
    cat > "$package_dir/USAGE.md" << 'EOF'
# AI Glasses 库使用说明

## 文件结构

```
dist/
├── include/          # 头文件
├── libs/            # 库文件
│   ├── arm64-v8a/   # ARM 64 位
│   ├── armeabi-v7a/ # ARM 32 位
│   ├── x86/         # x86
│   └── x86_64/      # x86_64
└── README.md
```

## 在 Linux 项目中使用

### 方法 1: CMake find_package

```cmake
find_package(ai_glasses REQUIRED)
target_link_libraries(your_target ai_glasses::ai_glasses)
```

### 方法 2: 手动指定

```cmake
include_directories(/path/to/dist/include)
link_directories(/path/to/dist/libs/${ANDROID_ABI})
target_link_libraries(your_target ai_glasses)
```

## 在 Android 项目中使用

### 1. 复制库文件

```bash
cp -r dist/include YourProject/app/src/main/cpp/
cp -r dist/libs YourProject/app/src/main/jniLibs/
```

### 2. 配置 CMakeLists.txt

```cmake
find_library(ai_glasses-lib 
    ai_glasses 
    PATHS ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}
)

target_link_libraries(your-target ${ai_glasses-lib})
```

## 示例代码

### C++ 使用

```cpp
#include "embedding_matcher.h"

ai_glasses::EmbeddingMatcher matcher;
matcher.initialize(dictPath, modelPath);
auto result = matcher.findBestMatch("墙留坠");
```

### Java 使用 (通过 JNI)

```java
static {
    System.loadLibrary("ai_glasses");
}

EmbeddingMatcher matcher = new EmbeddingMatcher();
matcher.initialize(dictPath, modelPath);
```
EOF
    
    echo_info "✓ 打包完成：$package_dir"
}

# ============================================================================
# 主函数
# ============================================================================

show_help() {
    echo "用法：$0 [选项]"
    echo ""
    echo "选项:"
    echo "  check       检查系统环境和依赖"
    echo "  clean       清理构建目录"
    echo "  build       构建所有 ABI"
    echo "  package     打包输出"
    echo "  all         检查 + 清理 + 构建 + 打包 (默认)"
    echo "  help        显示帮助信息"
    echo ""
    echo "环境变量:"
    echo "  ANDROID_NDK   Android NDK 路径 (默认：\$HOME/android-ndk)"
    echo "  BUILD_DIR     构建目录 (默认：build_android)"
    echo "  INSTALL_DIR   安装目录 (默认：install)"
    echo ""
    echo "示例:"
    echo "  $0                      # 完整构建"
    echo "  $0 check                # 只检查环境"
    echo "  $0 build                # 只构建"
    echo "  $0 package              # 只打包"
    echo "  ANDROID_NDK=/path $0    # 指定 NDK 路径"
    echo ""
    echo "支持的 ABI:"
    echo "  ${ABIS[*]}"
}

main() {
    local command=${1:-"all"}
    
    echo "========================================"
    echo "AI Glasses Android 库构建 (Linux)"
    echo "========================================"
    echo ""
    
    case "$command" in
        check)
            check_environment
            check_ndk
            ;;
        clean)
            check_environment
            check_ndk
            clean
            ;;
        build)
            check_environment
            check_ndk
            build_all
            ;;
        package)
            package
            ;;
        all)
            check_environment
            check_ndk
            clean
            build_all
            package
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo_error "未知命令：$command"
            echo ""
            show_help
            exit 1
            ;;
    esac
    
    echo ""
    echo "========================================"
    echo "✓ 构建完成!"
    echo "========================================"
}

main "$@"
