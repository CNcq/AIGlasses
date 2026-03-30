#!/bin/bash

# AI Glasses Linux 环境配置脚本
# 自动安装构建所需的依赖

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

# 获取 Linux 发行版信息
get_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    elif [ -f /etc/redhat-release ]; then
        echo "rhel"
    else
        echo "unknown"
    fi
}

# 检测包管理器
detect_package_manager() {
    if command -v apt >/dev/null 2>&1; then
        echo "apt"
    elif command -v dnf >/dev/null 2>&1; then
        echo "dnf"
    elif command -v yum >/dev/null 2>&1; then
        echo "yum"
    elif command -v pacman >/dev/null 2>&1; then
        echo "pacman"
    elif command -v zypper >/dev/null 2>&1; then
        echo "zypper"
    else
        echo "unknown"
    fi
}

# 安装依赖
install_dependencies() {
    local distro=$(get_distro)
    local pm=$(detect_package_manager)
    
    echo_step "检测到 Linux 发行版：$distro"
    echo_step "使用包管理器：$pm"
    echo ""
    
    # 基础构建工具
    local base_deps=(
        "cmake"
        "make"
        "g++"
        "build-essential"
    )
    
    case "$pm" in
        apt)
            echo_info "更新软件包列表..."
            sudo apt update
            
            echo_info "安装基础构建工具..."
            sudo apt install -y cmake make g++ build-essential git wget unzip
            ;;
            
        dnf)
            echo_info "安装基础构建工具..."
            sudo dnf install -y cmake make gcc-c++ git wget unzip
            ;;
            
        yum)
            echo_info "安装基础构建工具..."
            sudo yum install -y cmake make gcc-c++ git wget unzip
            ;;
            
        pacman)
            echo_info "安装基础构建工具..."
            sudo pacman -S --noconfirm cmake make gcc git wget unzip
            ;;
            
        zypper)
            echo_info "安装基础构建工具..."
            sudo zypper install -y cmake make gcc-c++ git wget unzip
            ;;
            
        *)
            echo_error "不支持的包管理器：$pm"
            echo_info "请手动安装以下工具：cmake, make, g++, git, wget, unzip"
            exit 1
            ;;
    esac
    
    echo_info "✓ 基础依赖安装完成"
}

# 安装 Android NDK（可选）
install_ndk() {
    echo ""
    echo_step "安装 Android NDK (可选)"
    echo ""
    
    read -p "是否需要下载并安装 Android NDK? (y/n): " answer
    
    if [[ ! "$answer" =~ ^[Yy]$ ]]; then
        echo_info "跳过 NDK 安装"
        echo_info "你可以稍后手动下载 NDK 并设置 ANDROID_NDK 环境变量"
        return
    fi
    
    local ndk_version="r25c"
    local ndk_url="https://dl.google.com/android/repository/android-ndk-${ndk_version}-linux.zip"
    local ndk_dir="$HOME/android-ndk"
    
    echo_info "下载 Android NDK $ndk_version..."
    echo_info "下载地址：$ndk_url"
    echo_info "这可能需要几分钟，取决于网络速度..."
    
    # 下载 NDK
    wget -c --tries=3 --read-timeout=30 "$ndk_url" -O "/tmp/android-ndk.zip"
    
    # 解压 NDK
    echo_info "解压 NDK 到 $ndk_dir..."
    unzip -q /tmp/android-ndk.zip -d "$HOME"
    
    # 重命名目录
    if [ -d "$HOME/android-ndk-linux" ]; then
        mv "$HOME/android-ndk-linux" "$ndk_dir"
    fi
    
    # 清理
    rm -f /tmp/android-ndk.zip
    
    echo_info "✓ NDK 安装完成：$ndk_dir"
    echo ""
    echo_info "请运行以下命令设置环境变量："
    echo "  export ANDROID_NDK=$ndk_dir"
    echo ""
    echo_info "建议添加到 ~/.bashrc 或 ~/.bash_profile："
    echo "  echo 'export ANDROID_NDK=$ndk_dir' >> ~/.bashrc"
    echo "  source ~/.bashrc"
}

# 验证安装
verify_installation() {
    echo ""
    echo_step "验证安装..."
    
    local success=true
    
    # 检查 CMake
    if command -v cmake >/dev/null 2>&1; then
        echo_info "✓ CMake: $(cmake --version | head -n1)"
    else
        echo_error "✗ CMake 未安装"
        success=false
    fi
    
    # 检查 Make
    if command -v make >/dev/null 2>&1; then
        echo_info "✓ Make: $(make --version | head -n1)"
    else
        echo_error "✗ Make 未安装"
        success=false
    fi
    
    # 检查 G++
    if command -v g++ >/dev/null 2>&1; then
        echo_info "✓ G++: $(g++ --version | head -n1)"
    else
        echo_error "✗ G++ 未安装"
        success=false
    fi
    
    # 检查 NDK（如果已设置）
    if [ -n "$ANDROID_NDK" ] && [ -d "$ANDROID_NDK" ]; then
        echo_info "✓ Android NDK: $ANDROID_NDK"
    else
        echo_warn "Android NDK 未设置（可选）"
    fi
    
    echo ""
    if [ "$success" = true ]; then
        echo_info "✓ 所有必需的工具已安装"
    else
        echo_error "✗ 部分工具未安装成功"
        exit 1
    fi
}

# 显示使用说明
show_usage() {
    echo ""
    echo "========================================"
    echo "环境配置完成!"
    echo "========================================"
    echo ""
    echo "下一步："
    echo "1. 如果已安装 NDK，设置环境变量："
    echo "   export ANDROID_NDK=/path/to/android-ndk"
    echo ""
    echo "2. 进入 lib 目录并构建："
    echo "   cd lib"
    echo "   ./build_android.sh"
    echo ""
    echo "========================================"
}

# 主函数
main() {
    echo "========================================"
    echo "AI Glasses Linux 环境配置"
    echo "========================================"
    echo ""
    
    # 检查是否为 root
    if [ "$EUID" -eq 0 ]; then
        echo_warn "警告：以 root 用户运行"
        echo_info "建议使用普通用户运行此脚本，需要时会自动提示输入密码"
    fi
    
    # 安装依赖
    install_dependencies
    
    # 可选安装 NDK
    install_ndk
    
    # 验证安装
    verify_installation
    
    # 显示使用说明
    show_usage
}

# 运行主函数
main "$@"
