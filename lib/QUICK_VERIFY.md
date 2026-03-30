# AI Glasses C++ 库 - 快速验证指南

## 📋 验证库是否正确生成

### 方法 1: 使用验证脚本（推荐）

```bash
cd /workspace/Project/AIGlasses/lib

# 赋予执行权限
chmod +x verify_library.sh

# 运行验证
./verify_library.sh
```

**预期输出：**
```
========================================
AI Glasses C++ Library Verification
========================================

[STEP] 检查库文件...
[INFO] 库文件大小：7.5M
[INFO] 文件类型：ELF 64-bit LSB shared object, x86-64
[INFO] 导出符号数：828
[SUCCESS] 库文件检查通过

[STEP] 验证 JNI 导出符号...
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeCreate
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeDestroy
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeAddTermDictionary
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeFindBestMatch
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeFindAllMatches
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeSetSimilarityThreshold
[INFO] ✓ Java_com_aiglasses_SemanticMatcher_nativeGetSimilarityThreshold
[SUCCESS] 所有 JNI 符号都已导出

[STEP] 验证核心功能符号...
[INFO] ✓ _ZN11ai_glasses16EmbeddingMatcher
[INFO] ✓ _ZN11ai_glasses16JiebaSegmenter
[INFO] ✓ _ZN11ai_glasses10JsonParser
[INFO] ✓ _ZN11ai_glasses15SemanticMatcher
[SUCCESS] 核心功能符号检查完成

========================================
[SUCCESS] 库验证完成！
========================================

库文件已正确生成并包含所有必需的符号
可以在 Android 项目中使用
```

---

### 方法 2: 手动验证

```bash
# 1. 检查库文件存在
ls -lh build_android/x86_64/lib/libai_glasses.so

# 2. 检查文件大小（应该 > 1MB）
du -h build_android/x86_64/lib/libai_glasses.so

# 3. 检查文件类型
file build_android/x86_64/lib/libai_glasses.so

# 4. 统计导出符号数量
nm -D build_android/x86_64/lib/libai_glasses.so | grep " T " | wc -l

# 5. 检查 JNI 函数
nm -D build_android/x86_64/lib/libai_glasses.so | grep "Java_"
```

---

## ✅ 验证清单

库正确生成的标志：

- [ ] 库文件存在：`build_android/x86_64/lib/libai_glasses.so`
- [ ] 文件大小合理：> 1MB（通常 5-10MB）
- [ ] 文件类型正确：ELF 64-bit LSB shared object
- [ ] 导出符号充足：> 500 个符号
- [ ] JNI 函数存在：包含所有 `Java_com_aiglasses_*` 函数
- [ ] 核心类存在：EmbeddingMatcher, JiebaSegmenter, JsonParser, SemanticMatcher

---

## 📊 库文件信息

根据实际构建结果：

| 属性 | 值 |
|------|-----|
| 文件路径 | `build_android/x86_64/lib/libai_glasses.so` |
| 文件大小 | 7.5 MB |
| 文件类型 | ELF 64-bit LSB shared object, x86-64 |
| 导出符号 | 828 个 |
| 包含 JNI | 是 |
| 状态 | ✅ 已验证可用 |

---

## 🚀 在 Android 项目中使用

### 1. 复制库文件

```bash
# 复制所有架构的库（如果构建了多个架构）
cp -r build_android/libs/* /path/to/your/android/app/src/main/jniLibs/
```

### 2. 在 Android 项目中配置

**app/build.gradle:**
```gradle
android {
    defaultConfig {
        ndk {
            abiFilters 'arm64-v8a', 'armeabi-v7a', 'x86_64'
        }
    }
    
    sourceSets {
        main {
            jniLibs.srcDirs = ['src/main/jniLibs']
        }
    }
}
```

### 3. 使用 Java 接口

```java
import com.aiglasses.SemanticMatcher;

public class MainActivity extends AppCompatActivity {
    private SemanticMatcher matcher;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // 创建匹配器
        matcher = new SemanticMatcher();
        
        // 添加词典
        matcher.addTermDictionary("墙壁", 
            Arrays.asList("墙面", "墙体", "墙"));
        
        // 查找最佳匹配
        MatchResult result = matcher.findBestMatch("墙皮掉落");
        Log.d("Match", "匹配结果：" + result.matchedTerm);
    }
}
```

---

## 🔍 故障排查

### 问题：验证脚本显示缺少符号

**解决方案：**
```bash
# 清理并重新构建
rm -rf build_android
./build_android.sh

# 重新验证
./verify_library.sh
```

### 问题：库文件太小（< 1MB）

**可能原因：**
- 缺少源文件
- 编译选项问题

**解决方案：**
```bash
# 检查 CMakeLists.txt 中的源文件列表
cat CMakeLists.txt | grep -A 10 "LIB_SOURCES"

# 确保包含所有 .cpp 文件
```

### 问题：没有 JNI 符号

**可能原因：**
- BUILD_ANDROID_JNI 选项未启用

**解决方案：**
```bash
# 检查构建配置
cat CMakeLists.txt | grep "BUILD_ANDROID_JNI"

# 重新构建（确保启用 JNI）
./build_android.sh
```

---

## 📚 相关文档

- [README.md](README.md) - 项目说明
- [BUILD_GUIDE.md](BUILD_GUIDE.md) - 构建指南
- [run_tests.sh](run_tests.sh) - 测试脚本

---

## ✅ 成功标志

当验证脚本显示：

```
========================================
[SUCCESS] 库验证完成！
========================================

库文件已正确生成并包含所有必需的符号
可以在 Android 项目中使用
```

**说明库已经完全可用！** 🎉
