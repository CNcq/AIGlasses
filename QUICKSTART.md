# 快速开始 - 5 分钟上手

## 📦 项目资源概览

### ✅ 已包含的资源文件

```
resources/
├── config/
│   └── defects_example.json      # 8 种缺陷类型的配置示例
└── models/
    └── embedding.txt             # 预生成的 Embedding 模型（2000 词，128 维）
```

### 📋 还需要准备的文件

```
resources/
├── config/
│   └── defects.json              # 你的实际缺陷配置（复制 defects_example.json 修改）
├── models/
│   └── embedding.txt             # ✅ 已包含
└── dict/
    └── jieba.dict                # 结巴分词词典（可选，使用默认词典）
```

---

## 🚀 快速开始（C++）

### 步骤 1: 包含头文件

```cpp
#include "embedding_matcher.h"
#include "resource_manager.h"

using namespace ai_glasses;
```

### 步骤 2: 初始化

```cpp
int main() {
    // 创建匹配器
    EmbeddingMatcher matcher;
    
    // 初始化（空路径使用默认设置）
    matcher.initialize("", "");
    
    // 从配置加载缺陷枚举
    matcher.loadDefectConfig("resources/config/defects_example.json");
    
    std::cout << "已加载 " << matcher.getEnumItemCount() 
              << " 个缺陷类型" << std::endl;
    
    return 0;
}
```

### 步骤 3: 执行匹配

```cpp
// 输入用户描述（可能包含谐音错误）
std::string input = "墙留坠很严重";

// 查找最匹配的缺陷
auto result = matcher.findBestMatch(input);

if (result.is_match) {
    std::cout << "输入：" << input << std::endl;
    std::cout << "匹配：" << result.matched_text << std::endl;
    std::cout << "相似度：" << result.similarity << std::endl;
} else {
    std::cout << "未找到匹配的缺陷类型" << std::endl;
}
```

### 完整示例

```cpp
#include "embedding_matcher.h"
#include <iostream>

using namespace ai_glasses;

int main() {
    // 1. 创建并初始化匹配器
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    // 2. 加载缺陷配置
    matcher.loadDefectConfig("resources/config/defects_example.json");
    
    // 3. 测试匹配
    std::vector<std::string> test_inputs = {
        "墙留坠很严重",
        "墙面有裂缝",
        "敲起来空空的",
        "卫生间渗水"
    };
    
    for (const auto& input : test_inputs) {
        auto result = matcher.findBestMatch(input);
        
        std::cout << "输入：" << input << std::endl;
        
        if (result.is_match) {
            std::cout << "  → 匹配：" << result.matched_text 
                      << " (相似度：" << result.similarity << ")" << std::endl;
        } else {
            std::cout << "  → 未匹配" << std::endl;
        }
    }
    
    return 0;
}
```

---

## 🚀 快速开始（Android）

### 步骤 1: 初始化资源管理器

```java
public class MainActivity extends AppCompatActivity {
    private EmbeddingMatcher matcher;
    private ResourceManager resourceManager;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // 1. 初始化资源管理器
        resourceManager = new ResourceManager(this);
        resourceManager.initialize();
        
        // 2. 创建匹配器
        matcher = new EmbeddingMatcher();
        
        // 3. 获取资源路径
        String modelPath = resourceManager.getModelPath("embedding.txt");
        String configPath = resourceManager.getConfigPath("defects_example.json");
        String dictPath = resourceManager.getDictPath("jieba.dict");
        
        // 4. 初始化匹配器
        matcher.initialize(dictPath, modelPath);
        
        // 5. 加载配置
        matcher.loadDefectConfig(configPath);
    }
}
```

### 步骤 2: 使用匹配器

```java
// 执行匹配
MatchResult result = matcher.findBestMatch("墙留坠很严重");

if (result.isMatch()) {
    System.out.println("匹配：" + result.getMatchedText());
    System.out.println("相似度：" + result.getSimilarity());
}
```

### 完整示例

```java
public class DefectMatcherDemo {
    private EmbeddingMatcher matcher;
    
    public void init(Context context) {
        // 初始化
        ResourceManager rm = new ResourceManager(context);
        rm.initialize();
        
        matcher = new EmbeddingMatcher();
        matcher.initialize(
            rm.getDictPath("jieba.dict"),
            rm.getModelPath("embedding.txt")
        );
        matcher.loadDefectConfig(rm.getConfigPath("defects_example.json"));
    }
    
    public void testMatching() {
        String[] testInputs = {
            "墙留坠很严重",
            "墙面有裂缝",
            "敲起来空空的",
            "卫生间渗水"
        };
        
        for (String input : testInputs) {
            MatchResult result = matcher.findBestMatch(input);
            
            Log.d("DefectMatcher", "输入：" + input);
            
            if (result.isMatch()) {
                Log.d("DefectMatcher", "  → 匹配：" + result.getMatchedText() 
                      + " (相似度：" + result.getSimilarity() + ")");
            } else {
                Log.d("DefectMatcher", "  → 未匹配");
            }
        }
    }
}
```

---

## 🎯 自定义配置

### 修改缺陷配置

复制示例配置并修改：

```bash
# 复制示例配置
cp resources/config/defects_example.json resources/config/defects.json
```

然后编辑 `resources/config/defects.json`：

```json
{
  "defects": [
    {
      "id": "MY_DEFECT_001",
      "name": "你的缺陷名称",
      "keywords": [
        "关键词 1",
        "关键词 2",
        "同义词",
        "谐音词"
      ],
      "threshold": 0.5,
      "enabled": true
    }
  ]
}
```

### 添加新缺陷类型

```json
{
  "defects": [
    {
      "id": "DEFECT_009",
      "name": "瓷砖空鼓",
      "keywords": [
        "瓷砖空鼓",
        "地砖空鼓",
        "瓷砖松动",
        "敲起来咚咚响"
      ],
      "categories": ["地面", "墙面"],
      "threshold": 0.55,
      "enabled": true
    }
  ]
}
```

---

## 📊 测试你的配置

### 测试脚本

创建一个简单的测试程序：

```cpp
#include "embedding_matcher.h"
#include <iostream>

using namespace ai_glasses;

void test_config(const std::string& config_path) {
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    std::cout << "加载配置：" << config_path << std::endl;
    
    if (!matcher.loadDefectConfig(config_path)) {
        std::cerr << "配置加载失败！" << std::endl;
        return;
    }
    
    std::cout << "✓ 配置加载成功" << std::endl;
    std::cout << "  缺陷数量：" << matcher.getEnumItemCount() << std::endl;
    std::cout << "  启用数量：" << matcher.getEnabledEnumItemCount() << std::endl;
    
    // 交互式测试
    std::cout << "\n输入测试文本（输入 'quit' 退出）：" << std::endl;
    
    while (true) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "quit") break;
        
        auto result = matcher.findBestMatch(input);
        
        if (result.is_match) {
            std::cout << "  匹配：" << result.matched_text 
                      << " (相似度：" << result.similarity << ")" << std::endl;
        } else {
            std::cout << "  未找到匹配" << std::endl;
        }
    }
}

int main() {
    test_config("resources/config/defects.json");
    return 0;
}
```

---

## 🔧 常见问题

### Q1: 如何添加更多缺陷类型？

**A**: 在配置文件中添加新的缺陷对象：

```json
{
  "defects": [
    {
      "id": "DEFECT_001",
      "name": "墙皮脱落",
      "keywords": ["墙皮脱落", "墙留坠"]
    },
    {
      "id": "DEFECT_002",
      "name": "新增缺陷",
      "keywords": ["关键词 1", "关键词 2"]
    }
  ]
}
```

### Q2: 如何调整匹配阈值？

**A**: 修改缺陷配置中的 `threshold` 字段：

```json
{
  "defects": [
    {
      "id": "DEFECT_001",
      "name": "墙皮脱落",
      "threshold": 0.6  // 提高阈值，匹配更严格
    }
  ]
}
```

### Q3: 如何优化匹配精度？

**A**: 
1. 添加更多关键词（包括同义词、谐音词）
2. 调整阈值
3. 使用领域特定的 Embedding 模型
4. 收集更多测试用例进行验证

---

## 📝 下一步

### 1. 阅读详细文档

- [`CONFIG_GUIDE.md`](CONFIG_GUIDE.md) - 配置指南
- [`EMBEDDING_MODEL_GUIDE.md`](EMBEDDING_MODEL_GUIDE.md) - 模型使用指南
- [`CHANGELOG_CONFIG.md`](CHANGELOG_CONFIG.md) - 更新说明

### 2. 自定义配置

- 复制 `defects_example.json` 为 `defects.json`
- 根据实际需求修改配置
- 添加你的缺陷类型

### 3. 集成到项目

- C++ 项目：包含头文件，链接库
- Android 项目：使用 Java 包装类
- 其他平台：使用 JNI 接口

### 4. 性能优化

- 使用预计算缓存
- 优化词汇表大小
- 调整向量维度

---

## ✅ 检查清单

开始前确保：

- [ ] `resources/models/embedding.txt` 存在
- [ ] `resources/config/defects_example.json` 存在
- [ ] 编译环境配置正确
- [ ] 包含必要的头文件
- [ ] 链接必要的库

---

## 🎉 开始使用

现在你可以：

1. 运行上面的示例代码
2. 修改配置添加自己的缺陷类型
3. 测试不同的输入文本
4. 集成到你的项目中

**祝你使用愉快！** 🚀
