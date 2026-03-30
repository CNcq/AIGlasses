# 快速测试指南

## 🚀 快速开始

### 1. 构建库

首先确保已经构建了库：

```bash
cd lib
./build_android.sh
```

### 2. 运行测试

测试脚本已创建完成，现在可以直接运行：

```bash
# 赋予执行权限
chmod +x run_tests.sh

# 运行所有测试
./run_tests.sh all

# 或者分步测试
./run_tests.sh verify   # 只验证 .so 文件
./run_tests.sh compile  # 编译测试程序
./run_tests.sh run      # 运行测试
```

---

## 📋 测试脚本说明

### 可用命令

| 命令 | 说明 |
|------|------|
| `./run_tests.sh all` | 运行所有测试（默认） |
| `./run_tests.sh verify` | 验证 .so 文件 |
| `./run_tests.sh compile` | 编译测试程序 |
| `./run_tests.sh run` | 运行测试 |
| `./run_tests.sh clean` | 清理测试文件 |
| `./run_tests.sh help` | 显示帮助信息 |

### 测试流程

#### 步骤 1: 验证 .so 文件

```bash
./run_tests.sh verify
```

输出示例：
```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 验证 .so 文件...
[INFO] 检查文件类型...
build_android/lib/libai_glasses.so: ELF 64-bit LSB shared object, x86-64
[INFO] 检查依赖库...
[INFO] 检查导出符号...
[INFO] 检查 JNI 导出函数...
[SUCCESS] .so 文件验证通过
```

#### 步骤 2: 编译测试程序

```bash
./run_tests.sh compile
```

输出示例：
```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 编译测试程序...
[SUCCESS] 测试程序编译成功
```

#### 步骤 3: 运行测试

```bash
./run_tests.sh run
```

输出示例：
```
========================================
AI Glasses C++ Library Test Suite
========================================

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

## 🔍 手动测试

### 检查 .so 文件

```bash
# 检查文件类型
file build_android/lib/libai_glasses.so

# 检查依赖
ldd build_android/lib/libai_glasses.so

# 检查导出符号
nm -D build_android/lib/libai_glasses.so | grep " T "

# 检查 JNI 函数
nm -D build_android/lib/libai_glasses.so | grep "Java_"
```

### 手动编译测试

```bash
g++ -std=c++14 \
    -I./include \
    -L./build_android/lib \
    -o test_library \
    test_library.cpp \
    -lai_glasses \
    -Wl,-rpath,./build_android/lib

# 运行测试
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build_android/lib
./test_library
```

---

## ✅ 预期输出

成功的测试应该显示：

```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 验证 .so 文件...
[INFO] 检查文件类型...
build_android/lib/libai_glasses.so: ELF 64-bit LSB shared object, x86-64
[INFO] 检查依赖库...
[INFO] 检查导出符号...
[INFO] 检查 JNI 导出函数...
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

## 🔧 故障排查

### 问题 1: 找不到 .so 文件

**错误：**
```
[ERROR] 找不到 libai_glasses.so
```

**解决：**
```bash
# 确保已构建
./build_android.sh
```

### 问题 2: 权限不足

**错误：**
```
bash: ./run_tests.sh: Permission denied
```

**解决：**
```bash
chmod +x run_tests.sh
```

### 问题 3: 链接错误

**错误：**
```
error while loading shared libraries: libai_glasses.so
```

**解决：**
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build_android/lib
```

### 问题 4: 编译失败

**错误：**
```
undefined reference to ...
```

**解决：**
```bash
# 检查库文件是否存在
ls -lh build_android/lib/

# 检查符号
nm -D build_android/lib/libai_glasses.so | grep " T "
```

---

## 📚 相关文档

- [TESTING_GUIDE.md](TESTING_GUIDE.md) - 完整测试指南
- [README.md](README.md) - 库的基本说明
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 详细构建指南

---

## 🎯 总结

现在你可以：

✅ **验证 .so 文件** - 检查库文件是否正确生成  
✅ **运行单元测试** - 测试所有核心功能  
✅ **自动化测试** - 一键运行所有测试  
✅ **快速排查问题** - 详细的错误提示  

**开始测试吧！** 🚀

```bash
chmod +x run_tests.sh
./run_tests.sh all
```
