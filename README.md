# AI Glasses - 移动端语义匹配解决方案

## 项目概述

这是一个面向移动端设备的语义匹配解决方案，实现了以下功能：
- 语音输入转文本（Android原生语音识别）
- 文本标准化和纠错
- 基于词典的语义匹配和近义词识别
- 高性能C++核心 + JNI接口设计

## 核心特性

### 1. 语义匹配算法
- **编辑距离（Levenshtein Distance）**：计算字符串相似度，权重60%
- **余弦相似度（Cosine Similarity）**：基于分词的语义相似度，权重40%
- **组合相似度评分**：综合两种算法得出最终匹配度

### 2. 词典管理
- 支持添加标准术语及其近义词
- 可动态扩展词典
- 支持设置相似度阈值

### 3. 移动端集成
- Android JNI接口
- 原生语音识别集成
- 简洁易用的Java API

## 项目结构

```
AIGlasses/
├── CMakeLists.txt              # 主CMake构建文件
├── include/
│   ├── semantic_matcher.h      # 语义匹配核心头文件
│   └── ai_glasses_jni.h        # JNI接口头文件
├── src/
│   ├── semantic_matcher.cpp    # 语义匹配核心实现
│   └── ai_glasses_jni.cpp      # JNI接口实现
├── android/                     # Android项目
│   ├── app/
│   │   ├── src/main/
│   │   │   ├── java/com/aiglasses/
│   │   │   │   ├── MainActivity.java      # 主Activity
│   │   │   │   ├── SemanticMatcher.java   # Java封装类
│   │   │   │   └── MatchResult.java       # 匹配结果类
│   │   │   ├── res/
│   │   │   │   └── layout/activity_main.xml
│   │   │   └── AndroidManifest.xml
│   │   └── build.gradle
│   ├── build.gradle
│   └── settings.gradle
└── test/                       # 测试代码
    ├── main.cpp
    └── CMakeLists.txt
```

## 使用方法

### 1. C++核心使用示例

```cpp
#include "semantic_matcher.h"

using namespace ai_glasses;

// 创建匹配器
SemanticMatcher matcher;
matcher.setSimilarityThreshold(0.5f);

// 添加词典
matcher.addTermDictionary("墙皮脱落", {
    "墙留坠", "墙皮掉了", "墙面脱落", "墙皮剥落"
});

// 查找最佳匹配
MatchResult result = matcher.findBestMatch("墙留坠很严重");
std::cout << "匹配结果: " << result.matched_term 
          << " (相似度: " << result.similarity << ")";
```

### 2. Android端使用示例

```java
SemanticMatcher matcher = new SemanticMatcher();
matcher.setSimilarityThreshold(0.5f);

// 添加词典
matcher.addTermDictionary("墙皮脱落", new String[]{
    "墙留坠", "墙皮掉了", "墙面脱落"
});

// 匹配
MatchResult result = matcher.findBestMatch("我发现在xx地方墙壁出现墙皮脱落");
if (result.isMatch) {
    Log.d(TAG, "匹配到: " + result.matchedTerm);
}

// 释放资源
matcher.destroy();
```

## 构建指南

### Android构建

1. 确保已安装Android Studio、NDK和CMake
2. 使用Android Studio打开 `android/` 目录
3. 同步Gradle并构建项目

或使用命令行：

```bash
cd android
./gradlew assembleDebug
```

### 桌面端测试构建

```bash
cd test
mkdir build && cd build
cmake ..
cmake --build .
./ai_glasses_test
```

## 算法说明

### 编辑距离
计算将一个字符串转换为另一个字符串所需的最少单字符编辑操作数（插入、删除、替换）。

### 余弦相似度
将文本转换为向量空间模型，计算两个向量之间的夹角余弦值。

### 组合评分
```
最终相似度 = 0.6 × 编辑距离相似度 + 0.4 × 余弦相似度
```

## 扩展建议

1. **拼音转换**：集成拼音库，支持拼音到汉字的转换
2. **机器学习模型**：考虑使用轻量级NLP模型（如TFLite）
3. **在线词典更新**：支持从服务器动态更新词典
4. **多语言支持**：扩展到支持更多语言

## 性能优化

- 使用高效的字符串处理算法
- 词典可预加载到内存
- 支持多线程并发匹配
- C++核心确保高性能

## 许可证

本项目仅供学习和参考使用。
