# AI Glasses C++ 库构建指南

本文档介绍如何将 AI Glasses C++ 代码编译成 `.so` 库文件，并作为第三方库供 Android 项目使用。

---

## 📦 输出产物

构建完成后会生成：

```
dist/
├── include/                    # 公共头文件
│   ├── embedding_matcher.h
│   ├── embedding_model.h
│   ├── jieba_segmenter.h
│   ├── json_parser.h
│   ├── defect_config_loader.h
│   ├── resource_manager.h
│   └── embedding_matcher_jni.h
├── libs/                       # 编译好的库
│   ├── arm64-v8a/
│   │   └── libai_glasses.so
│   ├── armeabi-v7a/
│   │   └── libai_glasses.so
│   ├── x86/
│   │   └── libai_glasses.so
│   └── x86_64/
│       └── libai_glasses.so
└── README.md
```

---

## 🛠️ 构建步骤

### 方法一：使用构建脚本（推荐）

#### Windows

```bash
cd lib

# 完整构建（清理 + 编译 + 打包）
build_android.bat

# 或者分步执行
build_android.bat clean
build_android.bat build
build_android.bat package
```

#### Linux/macOS

```bash
cd lib

# 添加执行权限
chmod +x build_android.sh

# 完整构建
./build_android.sh

# 或者分步执行
./build_android.sh clean
./build_android.sh build
./build_android.sh package
```

---

### 方法二：手动构建

#### 1. 设置环境变量

**Windows (PowerShell)**
```powershell
$env:ANDROID_NDK="C:\Android\android-ndk-r25b"
```

**Linux/macOS**
```bash
export ANDROID_NDK=$HOME/android-ndk-r25b
```

#### 2. 配置构建目录

```bash
cd lib
mkdir build_android
cd build_android
```

#### 3. 运行 CMake

```bash
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_ANDROID_JNI=ON
```

#### 4. 编译

```bash
cmake --build . --config Release
```

#### 5. 安装

```bash
cmake --install . --prefix ../install/arm64-v8a
```

---

## 📱 在 Android 项目中使用

### 步骤 1: 复制库文件

将构建好的 `dist` 目录复制到 Android 项目：

```bash
# 复制头文件
cp -r dist/include YourAndroidProject/app/src/main/cpp/

# 复制库文件
cp -r dist/libs YourAndroidProject/app/src/main/jniLibs/
```

或者在文件资源管理器中手动复制。

### 步骤 2: 配置 CMakeLists.txt

在 Android 项目的 `app/CMakeLists.txt` 中添加：

```cmake
# 查找预编译的库
find_library(ai_glasses-lib 
    ai_glasses 
    PATHS ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}
)

# 你的 JNI 库
add_library(your-jni-lib SHARED src/main/cpp/your-jni.cpp)

# 链接 ai_glasses 库
target_link_libraries(your-jni-lib 
    ${ai_glasses-lib}
    ${log-lib}
    android
)
```

### 步骤 3: 在 JNI 代码中使用

```cpp
#include "embedding_matcher.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_myapp_MainStringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    
    ai_glasses::EmbeddingMatcher matcher;
    matcher.initialize(dict_path, model_path);
    matcher.loadDefectConfig(config_path);
    
    auto result = matcher.findBestMatch("墙留坠");
    
    std::string output = "匹配结果：" + result.matched_text;
    return env->NewStringUTF(output.c_str());
}
```

### 步骤 4: 在 Java 代码中调用

```java
public class MainActivity extends AppCompatActivity {
    
    static {
        System.loadLibrary("your-jni-lib");
        // ai_glasses 库会自动被加载
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // 使用 JNI 调用
        String result = stringFromJNI();
        textView.setText(result);
    }
    
    public native String stringFromJNI();
}
```

---

## 🔧 高级配置

### 1. 只构建特定 ABI

如果只关心特定的 ABI，可以修改构建脚本：

**bash**
```bash
# 只构建 arm64-v8a
ABIS=("arm64-v8a")
./build_android.sh build
```

**批处理**
```batch
set ABIS=arm64-v8a
build_android.bat build
```

### 2. 构建静态库

修改 `CMakeLists.txt`：

```cmake
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
```

然后重新编译。

### 3. 自定义 API 级别

```bash
cmake .. \
    -DANDROID_PLATFORM=android-24 \  # 最低 API 24
    ...
```

### 4. 调试版本

```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    ...
```

---

## 📊 构建配置说明

### CMake 选项

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `BUILD_SHARED_LIBS` | 构建动态库 (.so) | ON |
| `BUILD_ANDROID_JNI` | 包含 JNI 接口 | ON |
| `BUILD_TESTS` | 构建测试程序 | OFF |

### Android 配置

| 配置 | 说明 | 默认值 |
|------|------|--------|
| `ANDROID_ABI` | 目标架构 | arm64-v8a |
| `ANDROID_PLATFORM` | 最低 API 级别 | android-21 |
| `ANDROID_NDK` | NDK 路径 | 环境变量 |

---

## ⚠️ 常见问题

### 1. CMake 找不到 NDK

**错误信息：**
```
Could not find Android NDK
```

**解决方法：**
确保正确设置了 `ANDROID_NDK` 环境变量：

**Windows:**
```powershell
setx ANDROID_NDK "C:\Android\android-ndk-r25b"
```

**Linux/macOS:**
```bash
echo 'export ANDROID_NDK=$HOME/android-ndk-r25b' >> ~/.bashrc
source ~/.bashrc
```

### 2. 编译时找不到头文件

**错误信息：**
```
fatal error: 'android/log.h' file not found
```

**解决方法：**
确保安装了完整的 NDK，并且路径正确。

### 3. 运行时找不到 so 文件

**错误信息：**
```
java.lang.UnsatisfiedLinkError: couldn't find DSO to load: libai_glasses.so
```

**解决方法：**
1. 确保 so 文件在正确的目录：`app/src/main/jniLibs/<abi>/`
2. 检查 ABI 是否匹配设备
3. 在 `build.gradle` 中配置：

```gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a'
        }
    }
}
```

### 4. 链接错误

**错误信息：**
```
undefined reference to `ai_glasses::EmbeddingMatcher::initialize'
```

**解决方法：**
1. 确保在 CMakeLists.txt 中链接了库
2. 检查库路径是否正确
3. 确认 ABI 匹配

---

## 📁 完整项目结构

```
AIGlasses/
├── lib/                          # 库项目目录
│   ├── CMakeLists.txt            # 库的构建配置
│   ├── README.md                 # 库说明
│   ├── build_android.sh          # Linux 构建脚本
│   ├── build_android.bat         # Windows 构建脚本
│   ├── cmake/
│   │   └── ai_glasses-config.cmake.in
│   └── src/ -> ../src/           # 符号链接到源码
│
├── android/                      # Android 示例项目
│   └── app/
│       ├── src/main/
│       │   ├── cpp/              # JNI 代码
│       │   └── jniLibs/          # 预编译 so 文件
│       └── CMakeLists.txt
│
├── include/                      # 头文件
└── src/                          # 源代码
```

---

## 🎯 最佳实践

### 1. 版本管理

为库文件添加版本号：

```cmake
set_target_properties(ai_glasses PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)
```

### 2. 分离调试和发布版本

```bash
# 调试版本
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 发布版本
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 3. 使用预编译头文件（可选）

对于大型项目，可以启用预编译头文件加速构建。

### 4. 自动化测试

在 `CMakeLists.txt` 中启用测试：

```cmake
option(BUILD_TESTS "Build tests" ON)
```

---

## 📖 相关文档

- [C++ 使用指南](../CPP_EMBEDDING_GUIDE.md)
- [配置指南](../CONFIG_GUIDE.md)
- [快速开始](../QUICKSTART.md)

---

## 🎉 总结

通过本指南，你可以：

✅ 将 C++ 代码编译成 `.so` 库  
✅ 支持多种 Android ABI  
✅ 作为第三方库集成到 Android 项目  
✅ 通过 JNI 调用 C++ 功能  

**构建完成后，`dist` 目录就是可以分发的第三方库！** 🚀
