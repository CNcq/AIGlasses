# AI Glasses Android 集成示例

本项目演示如何使用预编译的 `libai_glasses.so` 库。

## 📁 项目结构

```
android/
└── app/
    ├── src/main/
    │   ├── java/com/aiglasses/
    │   │   ├── EmbeddingMatcher.java      # Java 封装
    │   │   ├── EmbedMatchResult.java      # 结果类
    │   │   └── MainActivity.java          # 示例 Activity
    │   ├── cpp/
    │   │   └── ai_glasses_jni.cpp         # JNI 实现（可选）
    │   ├── jniLibs/
    │   │   ├── arm64-v8a/
    │   │   │   └── libai_glasses.so
    │   │   └── ...
    │   ├── assets/
    │   │   ├── models/
    │   │   │   └── embedding.txt
    │   │   └── config/
    │   │       └── defects.json
    │   ├── res/
    │   └── AndroidManifest.xml
    ├── CMakeLists.txt
    └── build.gradle
```

## 🔧 配置步骤

### 1. 复制库文件

从 `lib/dist/` 复制：

```bash
# 复制头文件
cp -r lib/dist/include android/app/src/main/cpp/

# 复制库文件
cp -r lib/dist/libs android/app/src/main/jniLibs/
```

### 2. 配置 build.gradle

```gradle
android {
    defaultConfig {
        applicationId "com.aiglasses.demo"
        minSdkVersion 21
        targetSdkVersion 33
        
        externalNativeBuild {
            cmake {
                cppFlags "-std=c++14 -frtti -fexceptions"
                abiFilters 'arm64-v8a', 'armeabi-v7a'
            }
        }
    }
    
    externalNativeBuild {
        cmake {
            path "CMakeLists.txt"
        }
    }
}
```

### 3. 配置 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.4.1)

project("aiglasses-demo")

# 查找预编译的库
find_library(ai_glasses-lib 
    ai_glasses 
    PATHS ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}
)

# 如果不需要自定义 JNI，可以直接使用预编译库
# 如果需要自定义 JNI，创建自己的库
add_library(your-jni-lib SHARED
    src/main/cpp/your-jni.cpp
)

# 链接库
target_link_libraries(your-jni-lib 
    ${ai_glasses-lib}
    ${log-lib}
    android
)
```

## 💻 使用示例

### 方式一：直接使用 Java 封装

```java
public class MainActivity extends AppCompatActivity {
    
    private EmbeddingMatcher matcher;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // 初始化
        String dictPath = getApplicationContext()
            .getDir("jieba", Context.MODE_PRIVATE).getAbsolutePath();
        String modelPath = getAssetsPath("models/embedding.txt");
        String configPath = getAssetsPath("config/defects.json");
        
        matcher = new EmbeddingMatcher();
        matcher.initialize(dictPath, modelPath);
        matcher.loadDefectConfig(configPath);
        
        // 匹配
        String input = "墙留坠很严重";
        MatchResult result = matcher.findBestMatch(input);
        
        Log.d("Match", "结果：" + result.getMatchedText());
        Log.d("Match", "相似度：" + result.getSimilarity());
    }
    
    private String getAssetsPath(String assetName) {
        // 实现从 assets 复制文件到缓存目录的逻辑
        // 返回实际文件路径
    }
}
```

### 方式二：自定义 JNI 调用

```cpp
// your-jni.cpp
#include <jni.h>
#include <string>
#include "embedding_matcher.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_aiglasses_MainStringFromJNI(
        JNIEnv* env,
        jobject /* this */,
        jstring input) {
    
    // 初始化匹配器（可以做成单例）
    static ai_glasses::EmbeddingMatcher matcher;
    static bool initialized = false;
    
    if (!initialized) {
        matcher.initialize(dict_path, model_path);
        matcher.loadDefectConfig(config_path);
        initialized = true;
    }
    
    // 获取输入
    const char* input_chars = env->GetStringUTFChars(input, nullptr);
    std::string input_text(input_chars);
    env->ReleaseStringUTFChars(input, input_chars);
    
    // 匹配
    auto result = matcher.findBestMatch(input_text);
    
    // 返回结果
    std::string output = "匹配：" + result.matched_text + 
                        " (相似度：" + std::to_string(result.similarity) + ")";
    return env->NewStringUTF(output.c_str());
}
```

```java
// MainActivity.java
public class MainActivity extends AppCompatActivity {
    
    static {
        System.loadLibrary("your-jni-lib");
    }
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        String result = stringFromJNI("墙留坠");
        textView.setText(result);
    }
    
    public native String stringFromJNI(String input);
}
```

## 📦 资源文件处理

### 从 Assets 复制文件到本地

```java
public class ResourceManager {
    
    public static String copyAssetToFile(Context context, String assetName) {
        try {
            File cacheDir = context.getCacheDir();
            File outputFile = new File(cacheDir, assetName);
            
            if (outputFile.exists()) {
                return outputFile.getAbsolutePath();
            }
            
            // 创建目录
            outputFile.getParentFile().mkdirs();
            
            // 复制文件
            InputStream in = context.getAssets().open(assetName);
            FileOutputStream out = new FileOutputStream(outputFile);
            
            byte[] buffer = new byte[4096];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
            
            in.close();
            out.flush();
            out.close();
            
            return outputFile.getAbsolutePath();
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}
```

## 🎯 完整示例

### MainActivity.java

```java
package com.aiglasses;

import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;
import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;

public class MainActivity extends AppCompatActivity {
    
    private static final String TAG = "MainActivity";
    private EmbeddingMatcher matcher;
    private TextView resultText;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        resultText = findViewById(R.id.result_text);
        
        // 初始化匹配器
        initMatcher();
        
        // 测试匹配
        testMatch("墙留坠很严重");
    }
    
    private void initMatcher() {
        try {
            // 复制资源文件
            String dictPath = getFilesDir().getAbsolutePath();
            String modelPath = copyAsset("models/embedding.txt");
            String configPath = copyAsset("config/defects.json");
            
            // 创建匹配器
            matcher = new EmbeddingMatcher();
            
            // 初始化
            boolean success = matcher.initialize(dictPath, modelPath);
            if (!success) {
                Log.e(TAG, "初始化失败");
                return;
            }
            
            // 加载配置
            success = matcher.loadDefectConfig(configPath);
            if (!success) {
                Log.e(TAG, "加载配置失败");
                return;
            }
            
            Log.i(TAG, "初始化成功");
            
        } catch (Exception e) {
            Log.e(TAG, "初始化异常", e);
        }
    }
    
    private String copyAsset(String assetName) throws Exception {
        File outputFile = new File(getCacheDir(), assetName);
        
        if (outputFile.exists()) {
            return outputFile.getAbsolutePath();
        }
        
        outputFile.getParentFile().mkdirs();
        
        InputStream in = getAssets().open(assetName);
        FileOutputStream out = new FileOutputStream(outputFile);
        
        byte[] buffer = new byte[4096];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
        
        in.close();
        out.flush();
        out.close();
        
        return outputFile.getAbsolutePath();
    }
    
    private void testMatch(String input) {
        if (matcher == null) {
            resultText.setText("匹配器未初始化");
            return;
        }
        
        MatchResult result = matcher.findBestMatch(input);
        
        String output = String.format(
            "输入：%s\n匹配：%s\n相似度：%.2f\n是否匹配：%s",
            input,
            result.getMatchedText(),
            result.getSimilarity(),
            result.isMatch() ? "是" : "否"
        );
        
        resultText.setText(output);
        Log.d(TAG, output);
    }
}
```

## 🚀 运行项目

1. **构建库**
   ```bash
   cd lib
   ./build_android.sh
   ```

2. **复制库到 Android 项目**
   ```bash
   cp -r dist/include ../android/app/src/main/cpp/
   cp -r dist/libs ../android/app/src/main/jniLibs/
   ```

3. **复制资源文件**
   ```bash
   cp ../resources/models/embedding.txt android/app/src/main/assets/models/
   cp ../resources/config/defects_example.json android/app/src/main/assets/config/defects.json
   ```

4. **打开 Android Studio**
   - 打开 `android/` 目录
   - 等待 Gradle 同步完成
   - 运行项目

## 📝 注意事项

1. **首次运行时需要复制资源文件**
   - Jieba 词典会自动下载
   - Embedding 模型需要放在 assets 或本地路径

2. **权限**
   - 确保有文件读写权限
   - Android 6.0+ 需要动态请求权限

3. **性能优化**
   - 使用单例模式保存匹配器实例
   - 预计算向量可以显著提升性能

## 🎉 完成！

现在你已经成功将预编译的 `.so` 库集成到 Android 项目中！ 🚀
