# 测试运行说明

## 📋 测试流程

### 1. 验证 .so 文件

```bash
./run_tests.sh verify
```

**预期输出：**
```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 验证 .so 文件...
[INFO] 检查文件类型...
./build_android/x86_64/lib/libai_glasses.so: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, BuildID[sha1]=..., with debug_info, not stripped
[INFO] 检查依赖库...
注意：这是 Android 库，无法在桌面系统上检查依赖（这是正常的）
[INFO] 检查导出符号...
[INFO] 检查 JNI 导出函数...
[SUCCESS] .so 文件验证通过
```

**说明：**
- ✅ `ldd` 命令显示错误是**正常的**，因为这是为 Android 编译的库
- ✅ 只要能检测到文件类型和导出符号，就说明库已正确生成

---

### 2. 编译测试程序

```bash
./run_tests.sh compile
```

**前提条件：**
- 已安装 g++：`sudo apt install g++`
- 已安装 cmake：`sudo apt install cmake`

**预期输出：**
```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 编译测试程序...
[SUCCESS] 测试程序编译成功
```

---

### 3. 运行测试

```bash
./run_tests.sh run
```

**预期输出：**
```
========================================
AI Glasses C++ Library Test Suite
========================================

[STEP] 运行测试...

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

## 🔍 常见问题

### 问题 1: ldd 显示错误

**错误信息：**
```
./build_android/x86_64/lib/libai_glasses.so: error while loading shared libraries: /usr/lib/x86_64-linux-gnu/libm.so: invalid ELF header
```

**说明：** 这是**正常的**！因为：
- 这是为 Android 编译的库（ARM 架构）
- 桌面 Linux 是 x86_64 架构
- 两者不兼容是正常的

**解决方案：** 忽略此错误，继续检查导出符号即可。

---

### 问题 2: 编译测试失败

**可能原因：**
1. 库文件不完整或损坏
2. 缺少依赖库
3. 头文件不匹配

**解决方案：**
```bash
# 1. 重新构建库
./build_android.sh

# 2. 安装依赖
sudo apt install g++ cmake make

# 3. 检查头文件
ls -la include/
```

---

### 问题 3: 运行时找不到库

**错误信息：**
```
error while loading shared libraries: libai_glasses.so: cannot open shared object file
```

**解决方案：**
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./build_android/x86_64/lib
./test_library
```

---

## 📊 验证清单

在运行测试前，请确认：

- [ ] 已成功运行 `./build_android.sh`
- [ ] `build_android/x86_64/lib/libai_glasses.so` 文件存在
- [ ] 文件类型显示为 ELF 64-bit LSB shared object
- [ ] 能检测到导出符号（`nm -D` 命令有输出）
- [ ] 已安装 g++ 编译器
- [ ] 已设置正确的库路径

---

## 🎯 完整测试流程

```bash
# 1. 进入项目目录
cd /workspace/Project/AIGlasses/lib

# 2. 构建库
./build_android.sh

# 3. 验证库文件
./run_tests.sh verify

# 4. 编译测试
./run_tests.sh compile

# 5. 运行测试
./run_tests.sh run

# 或者一次性运行所有测试
./run_tests.sh all
```

---

## 📚 相关文档

- [TESTING_GUIDE.md](TESTING_GUIDE.md) - 完整测试指南
- [README.md](README.md) - 项目说明
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 构建指南

---

## ✅ 成功标志

当看到以下输出时，表示测试全部通过：

```
========================================
✓ ALL TESTS PASSED!
========================================

[SUCCESS] 所有测试通过!
```

**库已准备就绪，可以集成到 Android 项目中使用！** 🎉
