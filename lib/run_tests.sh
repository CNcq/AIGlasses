#!/bin/bash

# AI Glasses C++ 库测试脚本

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo_step() {
    echo -e "${GREEN}[STEP]${NC} $1"
}

echo_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

echo_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

echo_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# 检查是否已构建
check_build() {
    if [ ! -d "build_android" ]; then
        echo_error "构建目录不存在，请先运行构建"
        echo_info "运行：./build_android.sh"
        exit 1
    fi
    
    if [ ! -f "./build_android/x86_64/lib/libai_glasses.so" ]; then
        echo_error "找不到 libai_glasses.so"
        echo_info "请先运行构建：./build_android.sh"
        exit 1
    fi
}

# 测试 .so 文件
test_shared_library() {
    echo_step "验证 .so 文件..."
    
    # 检查文件是否存在
    if [ ! -f "./build_android/x86_64/lib/libai_glasses.so" ]; then
        echo_error "找不到 libai_glasses.so"
        exit 1
    fi
    
    # 检查文件类型
    echo_info "检查文件类型..."
    file ./build_android/x86_64/lib/libai_glasses.so
    
    # 检查依赖（仅适用于本地库，Android 库会跳过）
    echo_info "检查依赖库..."
    if ldd ./build_android/x86_64/lib/libai_glasses.so 2>&1 | grep -q "invalid ELF header\|not a dynamic executable"; then
        echo_info "注意：这是 Android 库，无法在桌面系统上检查依赖（这是正常的）"
    else
        ldd ./build_android/x86_64/lib/libai_glasses.so || true
    fi
    
    # 检查符号表
    echo_info "检查导出符号..."
    nm -D ./build_android/x86_64/lib/libai_glasses.so | grep " T " | head -20
    
    # 检查 JNI 函数
    echo_info "检查 JNI 导出函数..."
    nm -D ./build_android/x86_64/lib/libai_glasses.so | grep "Java_" | head -10
    
    echo_success ".so 文件验证通过"
}

# 编译测试
compile_tests() {
    echo_step "编译测试程序..."
    
    if [ ! -f "test_library.cpp" ]; then
        echo_error "找不到 test_library.cpp"
        echo_info "请先创建测试文件（参考 TESTING_GUIDE.md）"
        exit 1
    fi
    
    # 检查库文件是否存在
    if [ ! -f "./build_android/x86_64/lib/libai_glasses.so" ]; then
        echo_error "找不到库文件"
        echo_info "请先运行构建：./build_android.sh"
        exit 1
    fi
    
    # 检查编译器
    if ! command -v g++ &> /dev/null; then
        echo_error "g++ 未安装"
        echo_info "安装：sudo apt install g++"
        exit 1
    fi
    
    echo_info "编译命令：g++ -std=c++14 -I./include -L./build_android/x86_64/lib -o test_library test_library.cpp -lai_glasses -Wl,-rpath,./build_android/x86_64/lib"
    
    g++ -std=c++14 \
        -I./include \
        -L./build_android/x86_64/lib \
        -o test_library \
        test_library.cpp \
        -lai_glasses \
        -Wl,-rpath,./build_android/x86_64/lib \
        2>&1
    
    if [ $? -eq 0 ]; then
        echo_success "测试程序编译成功"
    else
        echo_error "测试程序编译失败"
        echo_info "可能的原因："
        echo_info "  1. 库文件不完整或损坏 - 请重新运行 ./build_android.sh"
        echo_info "  2. 缺少符号定义 - 检查是否所有源文件都已添加到 CMakeLists.txt"
        echo_info "  3. 头文件不匹配 - 清理并重新构建"
        echo_info ""
        echo_info "建议执行："
        echo_info "  rm -rf build_android"
        echo_info "  ./build_android.sh"
        exit 1
    fi
}

# 运行测试
run_tests() {
    echo_step "运行测试..."
    echo ""
    
    if [ ! -f "test_library" ]; then
        echo_error "测试程序不存在，请先编译"
        echo_info "运行：$0 compile"
        exit 1
    fi
    
    # 设置库路径
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build_android/x86_64/lib
    
    ./test_library
    
    if [ $? -eq 0 ]; then
        echo ""
        echo_success "所有测试通过!"
    else
        echo ""
        echo_error "测试失败"
        exit 1
    fi
}

# 清理
clean() {
    echo_step "清理测试文件..."
    rm -f test_library test_library.cpp
    echo_success "清理完成"
}

# 显示帮助
show_help() {
    echo "AI Glasses C++ 库测试脚本"
    echo ""
    echo "用法：$0 {compile|run|verify|clean|all|help}"
    echo ""
    echo "命令说明："
    echo "  compile - 编译测试程序"
    echo "  run     - 运行测试"
    echo "  verify  - 验证 .so 文件"
    echo "  clean   - 清理测试文件"
    echo "  all     - 运行所有测试（默认）"
    echo "  help    - 显示帮助信息"
    echo ""
    echo "示例："
    echo "  $0 verify              # 只验证 .so 文件"
    echo "  $0 compile             # 编译测试程序"
    echo "  $0 run                 # 运行测试"
    echo "  $0 all                 # 运行所有测试"
    echo ""
    echo "注意：运行测试前需要先构建库（./build_android.sh）"
}

# 主函数
main() {
    echo "========================================"
    echo "AI Glasses C++ Library Test Suite"
    echo "========================================"
    echo ""
    
    case "${1:-all}" in
        compile)
            check_build
            compile_tests
            ;;
        run)
            check_build
            run_tests
            ;;
        verify)
            check_build
            test_shared_library
            ;;
        clean)
            clean
            ;;
        all)
            check_build
            test_shared_library
            compile_tests
            run_tests
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo_error "未知命令：$1"
            echo ""
            show_help
            exit 1
            ;;
    esac
}

main "$@"
