#!/bin/bash

# AI Glasses C++ 库 - 桌面版本构建脚本

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo_step() {
    echo -e "${GREEN}[STEP]${NC} $1"
}

echo_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# 检查依赖
check_dependencies() {
    echo_step "检查系统依赖..."
    
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if ! command -v g++ &> /dev/null; then
        missing_deps+=("g++")
    fi
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        echo_error "缺少以下依赖："
        for dep in "${missing_deps[@]}"; do
            echo "  - $dep"
        done
        echo ""
        echo_info "安装方法："
        echo "  sudo apt update && sudo apt install -y cmake make g++ build-essential"
        exit 1
    fi
    
    echo_success "系统依赖检查通过"
}

# 清理构建目录
clean_build() {
    echo_step "清理旧的构建文件..."
    rm -rf build_desktop
    echo_success "清理完成"
}

# 创建构建目录
setup_build_dir() {
    echo_step "创建构建目录..."
    mkdir -p build_desktop
    cd build_desktop
}

# 配置 CMake
configure_cmake() {
    echo_step "配置 CMake..."
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_ANDROID_JNI=OFF \
        -DBUILD_SHARED_LIBS=ON \
        -DBUILD_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX=./install
    
    echo_success "CMake 配置完成"
}

# 编译
build() {
    echo_step "开始编译..."
    
    # 确保在正确的目录
    if [ ! -f "Makefile" ]; then
        cd build_desktop
    fi
    
    make -j$(nproc)
    
    echo_success "编译完成"
}

# 验证构建
verify_build() {
    echo_step "验证构建结果..."
    
    # 确保在正确的目录
    if [ -f "Makefile" ]; then
        cd ..
    fi
    
    if [ -f "build_desktop/lib/libai_glasses.so" ]; then
        echo_info "共享库：build_desktop/lib/libai_glasses.so"
        echo_info "文件大小：$(du -h build_desktop/lib/libai_glasses.so | cut -f1)"
        echo_success "桌面版本构建成功"
    else
        echo_error "构建失败：找不到 libai_glasses.so"
        exit 1
    fi
    
    # 检查符号
    echo_info "导出符号数：$(nm -D build_desktop/lib/libai_glasses.so | grep " T " | wc -l)"
}

# 主函数
main() {
    echo "========================================"
    echo "AI Glasses C++ Library - Desktop Build"
    echo "========================================"
    echo ""
    
    case "${1:-build}" in
        clean)
            clean_build
            ;;
        build)
            check_dependencies
            clean_build
            setup_build_dir
            configure_cmake
            build
            verify_build
            ;;
        rebuild)
            check_dependencies
            clean_build
            setup_build_dir
            configure_cmake
            build
            verify_build
            ;;
        *)
            echo "用法：$0 {build|clean|rebuild}"
            echo ""
            echo "  build   - 构建桌面版本（默认）"
            echo "  clean   - 清理构建文件"
            echo "  rebuild - 重新构建"
            exit 1
            ;;
    esac
    
    echo ""
    echo_success "桌面版本构建完成！"
    echo ""
    echo_info "下一步："
    echo_info "1. 运行测试：./run_tests.sh run"
    echo_info "2. 库文件位置：build_desktop/lib/libai_glasses.so"
    echo ""
}

main "$@"
