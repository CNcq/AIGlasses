#!/bin/bash

# AI Glasses C++ 库简单测试脚本

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

# 检查库文件
check_library() {
    echo_step "检查库文件..."
    
    if [ ! -f "./build_android/x86_64/lib/libai_glasses.so" ]; then
        echo_error "找不到 libai_glasses.so"
        echo_info "请先运行构建：./build_android.sh"
        exit 1
    fi
    
    echo_info "库文件大小：$(du -h ./build_android/x86_64/lib/libai_glasses.so | cut -f1)"
    echo_info "文件类型：$(file ./build_android/x86_64/lib/libai_glasses.so | cut -d: -f2)"
    echo_info "导出符号数：$(nm -D ./build_android/x86_64/lib/libai_glasses.so | grep " T " | wc -l)"
    
    echo_success "库文件检查通过"
}

# 验证 JNI 符号
verify_jni_symbols() {
    echo_step "验证 JNI 导出符号..."
    
    local required_symbols=(
        "Java_com_aiglasses_SemanticMatcher_nativeCreate"
        "Java_com_aiglasses_SemanticMatcher_nativeDestroy"
        "Java_com_aiglasses_SemanticMatcher_nativeAddTermDictionary"
        "Java_com_aiglasses_SemanticMatcher_nativeFindBestMatch"
        "Java_com_aiglasses_SemanticMatcher_nativeFindAllMatches"
        "Java_com_aiglasses_SemanticMatcher_nativeSetSimilarityThreshold"
        "Java_com_aiglasses_SemanticMatcher_nativeGetSimilarityThreshold"
    )
    
    local all_found=true
    
    for symbol in "${required_symbols[@]}"; do
        if nm -D ./build_android/x86_64/lib/libai_glasses.so | grep -q "$symbol"; then
            echo_info "✓ $symbol"
        else
            echo_error "✗ $symbol (未找到)"
            all_found=false
        fi
    done
    
    if [ "$all_found" = true ]; then
        echo_success "所有 JNI 符号都已导出"
    else
        echo_error "缺少部分 JNI 符号"
        exit 1
    fi
}

# 验证核心功能符号
verify_core_symbols() {
    echo_step "验证核心功能符号..."
    
    local core_symbols=(
        "_ZN11ai_glasses16EmbeddingMatcher"
        "_ZN11ai_glasses16JiebaSegmenter"
        "_ZN11ai_glasses10JsonParser"
        "_ZN11ai_glasses15SemanticMatcher"
    )
    
    for symbol in "${core_symbols[@]}"; do
        if nm -D ./build_android/x86_64/lib/libai_glasses.so | grep -q "$symbol"; then
            echo_info "✓ $symbol"
        else
            echo_info "⚠ $symbol (未找到，可能是内联函数)"
        fi
    done
    
    echo_success "核心功能符号检查完成"
}

# 主函数
main() {
    echo "========================================"
    echo "AI Glasses C++ Library Verification"
    echo "========================================"
    echo ""
    
    check_library
    echo ""
    verify_jni_symbols
    echo ""
    verify_core_symbols
    echo ""
    
    echo "========================================"
    echo_success "库验证完成！"
    echo "========================================"
    echo ""
    echo_info "库文件已正确生成并包含所有必需的符号"
    echo_info "可以在 Android 项目中使用"
    echo ""
    echo_info "下一步："
    echo_info "1. 将库文件复制到 Android 项目的 libs 目录"
    echo_info "2. 在 build.gradle 中配置 JNI 库"
    echo_info "3. 使用 Java 接口调用库功能"
    echo ""
}

main "$@"
