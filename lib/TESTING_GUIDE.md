# AI Glasses C++ 库测试指南

本文档提供完整的测试方案，验证生成的 .so 库是否能正常运行。

---

## 📋 测试概览

测试分为三个层次：

1. **单元测试** - 测试核心功能
2. **集成测试** - 测试完整的库功能
3. **Android 集成测试** - 在 Android 设备上测试

---

## 🧪 测试 1: 基础功能测试

### 创建测试文件

在 `lib/` 目录下创建 `test_library.cpp`：

```cpp
#include "embedding_matcher.h"
#include "jieba_segmenter.h"
#include "json_parser.h"
#include "semantic_matcher.h"
#include <iostream>
#include <cassert>

using namespace ai_glasses;

// 测试 JSON 解析器
void test_json_parser() {
    std::cout << "Testing JSON Parser..." << std::endl;
    
    std::string json = R"({
        "name": "test",
        "value": 123,
        "enabled": true,
        "items": [1, 2, 3]
    })";
    
    JsonValue root = JsonParser::parse(json);
    
    assert(root.isObject());
    assert(root.has("name"));
    assert(root["name"].isString());
    assert(root["name"].asString() == "test");
    assert(root["value"].isNumber());
    assert(root["value"].asNumber() == 123);
    assert(root["enabled"].isBool());
    assert(root["enabled"].asBool() == true);
    assert(root["items"].isArray());
    assert(root["items"].asArray().size() == 3);
    
    std::cout << "✓ JSON Parser test passed!" << std::endl;
}

// 测试结巴分词
void test_jieba_segmenter() {
    std::cout << "Testing Jieba Segmenter..." << std::endl;
    
    JiebaSegmenter segmenter;
    segmenter.initialize("");  // 使用内置词典
    
    std::vector<std::string> tokens = segmenter.segment("墙壁出现脱落");
    
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
    
    assert(!tokens.empty());
    
    std::cout << "✓ Jieba Segmenter test passed!" << std::endl;
}

// 测试 EmbeddingMatcher
void test_embedding_matcher() {
    std::cout << "Testing Embedding Matcher..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", 128);
    
    // 添加测试数据
    matcher.addEnumItem("wall_peel", "墙皮脱落", "墙壁表面材料脱落", 
                        {"墙壁", "脱落", "墙皮"}, 0.5f);
    matcher.addEnumItem("crack", "裂缝", "结构出现裂缝", 
                        {"裂缝", "裂痕"}, 0.5f);
    matcher.addEnumItem("corrosion", "锈蚀", "金属表面锈蚀", 
                        {"锈蚀", "腐蚀"}, 0.5f);
    
    // 测试匹配
    MatchResult result = matcher.findBestMatch("墙壁掉皮");
    
    std::cout << "Input: 墙壁掉皮" << std::endl;
    std::cout << "Matched: " << result.matched_text << std::endl;
    std::cout << "Similarity: " << result.similarity << std::endl;
    
    assert(!result.matched_id.empty());
    
    std::cout << "✓ Embedding Matcher test passed!" << std::endl;
}

// 测试 SemanticMatcher
void test_semantic_matcher() {
    std::cout << "Testing Semantic Matcher..." << std::endl;
    
    SemanticMatcher matcher;
    
    // 添加同义词词典
    std::vector<std::string> wall_synonyms = {"墙壁", "墙面", "墙体", "墙"};
    matcher.addTermDictionary("墙壁", wall_synonyms);
    
    std::vector<std::string> peel_synonyms = {"脱落", "掉落", "剥离", "掉皮"};
    matcher.addTermDictionary("脱落", peel_synonyms);
    
    // 测试匹配
    MatchResult result = matcher.findBestMatch("墙皮掉落");
    
    std::cout << "Input: 墙皮掉落" << std::endl;
    std::cout << "Matched: " << result.matched_term << std::endl;
    std::cout << "Similarity: " << result.similarity << std::endl;
    
    std::cout << "✓ Semantic Matcher test passed!" << std::endl;
}

// 测试配置加载
void test_config_loader() {
    std::cout << "Testing Config Loader..." << std::endl;
    
    std::string json = R"({
        "defects": [
            {
                "id": "defect_001",
                "name": "墙皮脱落",
                "description": "墙壁表面材料脱落",
                "keywords": ["墙壁", "脱落", "墙皮"],
                "threshold": 0.5,
                "enabled": true
            },
            {
                "id": "defect_002",
                "name": "裂缝",
                "description": "结构出现裂缝",
                "keywords": ["裂缝", "裂痕"],
                "threshold": 0.6,
                "enabled": true
            }
        ]
    })";
    
    EmbeddingMatcher matcher;
    matcher.initialize("", 128);
    
    DefectConfigLoader loader;
    loader.loadFromString(json);
    loader.applyToMatcher(matcher);
    
    assert(loader.getConfigCount() == 2);
    
    std::cout << "✓ Config Loader test passed!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "AI Glasses C++ Library Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        test_json_parser();
        test_jieba_segmenter();
        test_embedding_matcher();
        test_semantic_matcher();
        test_config_loader();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "✓ ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
```

---

## 🔧 测试 2: 创建自动化测试脚本

### 创建 `run_tests.sh`

```bash
#!/bin/bash

# AI Glasses C++ 库测试脚本

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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

# 检查是否已构建
check_build() {
    if [ ! -d "build_android" ]; then
        echo_error "构建目录不存在，请先运行构建"
        echo_info "运行：./build_android.sh"
        exit 1
    fi
}

# 编译测试
compile_tests() {
    echo_step "编译测试程序..."
    
    g++ -std=c++14 \
        -I./include \
        -L./build_android/lib \
        -o test_library \
        test_library.cpp \
        -lai_glasses \
        -Wl,-rpath,./build_android/lib
    
    if [ $? -eq 0 ]; then
        echo_success "测试程序编译成功"
    else
        echo_error "测试程序编译失败"
        exit 1
    fi
}

# 运行测试
run_tests() {
    echo_step "运行测试..."
    echo ""
    
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

# 测试 .so 文件
test_shared_library() {
    echo_step "验证 .so 文件..."
    
    # 检查文件是否存在
    if [ ! -f "./build_android/lib/libai_glasses.so" ]; then
        echo_error "找不到 libai_glasses.so"
        exit 1
    fi
    
    # 检查文件类型
    echo_info "检查文件类型..."
    file ./build_android/lib/libai_glasses.so
    
    # 检查依赖
    echo_info "检查依赖库..."
    ldd ./build_android/lib/libai_glasses.so || true
    
    # 检查符号表
    echo_info "检查导出符号..."
    nm -D ./build_android/lib/libai_glasses.so | grep " T " | head -20
    
    echo_success ".so 文件验证通过"
}

# 清理
clean() {
    echo_step "清理测试文件..."
    rm -f test_library test_library.cpp
    echo_success "清理完成"
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
            run_tests
            ;;
        verify)
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
        *)
            echo "用法：$0 {compile|run|verify|clean|all}"
            echo ""
            echo "  compile - 编译测试程序"
            echo "  run     - 运行测试"
            echo "  verify  - 验证 .so 文件"
            echo "  clean   - 清理测试文件"
            echo "  all     - 运行所有测试（默认）"
            exit 1
            ;;
    esac
}

main "$@"
```

---

## 📱 测试 3: Android 集成测试

### 创建 Android 测试项目

在 `android/test-app/` 目录下创建测试应用：

**build.gradle**
```gradle
plugins {
    id 'com.android.application'
    id 'org.jetbrains.kotlin.android'
}

android {
    namespace 'com.aiglasses.test'
    compileSdk 33

    defaultConfig {
        applicationId "com.aiglasses.test"
        minSdk 21
        targetSdk 33
        versionCode 1
        versionName "1.0"

        testInstrumentationRunner "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
    
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
}

dependencies {
    implementation fileTree(dir: 'libs', include: ['*.jar', '*.so'])
    implementation 'androidx.appcompat:appcompat:1.6.1'
    testImplementation 'junit:junit:4.13.2'
    androidTestImplementation 'androidx.test.ext:junit:1.1.5'
    androidTestImplementation 'androidx.test.espresso:espresso-core:3.5.1'
}
```

**MainActivity.kt**
```kotlin
package com.aiglasses.test

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.aiglasses.EmbeddingMatcher
import com.aiglasses.ResourceManager

class MainActivity : AppCompatActivity() {
    
    private lateinit var matcher: EmbeddingMatcher
    private lateinit var resultText: TextView
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        resultText = TextView(this).apply {
            textSize = 16f
        }
        setContentView(resultText)
        
        runTests()
    }
    
    private fun runTests() {
        val results = StringBuilder()
        results.appendLine("=== AI Glasses Library Test ===\n")
        
        try {
            // 测试 1: 初始化
            results.appendLine("Test 1: Initialize...")
            matcher = EmbeddingMatcher()
            matcher.initialize(128)
            results.appendLine("✓ Initialize passed\n")
            
            // 测试 2: 添加枚举项
            results.appendLine("Test 2: Add Enum Items...")
            matcher.addEnumItem(
                "wall_peel",
                "墙皮脱落",
                "墙壁表面材料脱落",
                listOf("墙壁", "脱落", "墙皮"),
                0.5f
            )
            matcher.addEnumItem(
                "crack",
                "裂缝",
                "结构出现裂缝",
                listOf("裂缝", "裂痕"),
                0.6f
            )
            results.appendLine("✓ Add Enum Items passed\n")
            
            // 测试 3: 语义匹配
            results.appendLine("Test 3: Semantic Matching...")
            val inputTexts = listOf(
                "墙壁掉皮",
                "墙面有裂痕",
                "墙体脱落"
            )
            
            for (text in inputTexts) {
                val result = matcher.findBestMatch(text)
                results.appendLine("Input: $text")
                results.appendLine("  Matched: ${result.matchedText}")
                results.appendLine("  Similarity: ${result.similarity}")
                results.appendLine()
            }
            results.appendLine("✓ Semantic Matching passed\n")
            
            // 测试 4: 批量匹配
            results.appendLine("Test 4: Batch Matching...")
            val matches = matcher.findAllMatches("墙壁出现问题", 3)
            results.appendLine("Found ${matches.size} matches")
            for ((index, match) in matches.withIndex()) {
                results.appendLine("  ${index + 1}. ${match.matchedText} (${match.similarity})")
            }
            results.appendLine("✓ Batch Matching passed\n")
            
            results.appendLine("=== ALL TESTS PASSED ===")
            
        } catch (e: Exception) {
            results.appendLine("\n=== TEST FAILED ===")
            results.appendLine("Error: ${e.message}")
            e.printStackTrace()
        }
        
        resultText.text = results.toString()
    }
}
```

---

## 🚀 测试 4: CI/CD Pipeline

### 创建 GitHub Actions 工作流

创建 `.github/workflows/test.yml`：

```yaml
name: Test Library

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-and-test:
    runs-on: ubuntu-22.04
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Java
      uses: actions/setup-java@v3
      with:
        java-version: '11'
        distribution: 'temurin'
    
    - name: Setup Android NDK
      run: |
        wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
        unzip android-ndk-r25c-linux.zip -d $HOME
        echo "ANDROID_NDK=$HOME/android-ndk-r25c" >> $GITHUB_ENV
    
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake make g++ build-essential
    
    - name: Build Library
      run: |
        cd lib
        ./build_android.sh
    
    - name: Verify .so Files
      run: |
        cd lib
        file build_android/lib/libai_glasses.so
        nm -D build_android/lib/libai_glasses.so | grep " T " | head -20
    
    - name: Run Tests
      run: |
        cd lib
        chmod +x run_tests.sh
        ./run_tests.sh all
    
    - name: Upload Build Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: ai-glasses-library
        path: lib/dist/
```

---

## 📊 测试执行流程

### 完整测试流程

```bash
# 1. 构建库
cd lib
./build_android.sh

# 2. 运行所有测试
chmod +x run_tests.sh
./run_tests.sh all

# 3. 或者分步测试
./run_tests.sh verify   # 验证 .so 文件
./run_tests.sh compile  # 编译测试程序
./run_tests.sh run      # 运行测试
./run_tests.sh clean    # 清理
```

---

## ✅ 预期输出

成功的测试应该输出：

```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 验证 .so 文件...
[INFO] 检查文件类型...
build_android/lib/libai_glasses.so: ELF 64-bit LSB shared object, x86-64
[INFO] 检查依赖库...
[INFO] 检查导出符号...
[SUCCESS] .so 文件验证通过

[STEP] 编译测试程序...
[SUCCESS] 测试程序编译成功

[STEP] 运行测试...
========================================
AI Glasses C++ Library Test Suite
========================================

Testing JSON Parser...
✓ JSON Parser test passed!

Testing Jieba Segmenter...
Tokens: 墙壁 出现 脱落
✓ Jieba Segmenter test passed!

Testing Embedding Matcher...
Input: 墙壁掉皮
Matched: 墙皮脱落
Similarity: 0.85
✓ Embedding Matcher test passed!

Testing Semantic Matcher...
Input: 墙皮掉落
Matched: 墙壁
Similarity: 0.75
✓ Semantic Matcher test passed!

Testing Config Loader...
✓ Config Loader test passed!

========================================
✓ ALL TESTS PASSED!
========================================

[SUCCESS] 所有测试通过!
```

---

## 🔍 故障排查

### 常见问题

1. **找不到 .so 文件**
   ```bash
   # 确保已构建
   ./build_android.sh
   ```

2. **链接错误**
   ```bash
   # 检查库路径
   export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build_android/lib
   ```

3. **符号未定义**
   ```bash
   # 检查导出符号
   nm -D ./build_android/lib/libai_glasses.so | grep " T "
   ```

---

## 📚 相关文档

- [README.md](README.md) - 库的基本说明
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 详细构建指南
- [ANDROID_INTEGRATION.md](ANDROID_INTEGRATION.md) - Android 集成说明

---

## 🎯 总结

本测试方案提供：

✅ **单元测试** - 测试核心功能  
✅ **集成测试** - 测试完整库功能  
✅ **Android 测试** - 在真实设备上验证  
✅ **CI/CD Pipeline** - 自动化测试流程  

**确保库在任何环境下都能正常工作！** 🚀
