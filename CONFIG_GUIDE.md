# 配置指南 - 灵活的缺陷枚举管理

## 📋 概述

本指南介绍如何灵活配置缺陷枚举数据和本地 Embedding 模型资源管理。

## 🎯 核心特性

✅ **JSON 配置文件**: 灵活的缺陷枚举配置格式  
✅ **资源管理器**: 统一管理本地模型和配置文件  
✅ **动态加载**: 支持运行时加载和更新配置  
✅ **分类管理**: 支持按类别筛选缺陷  
✅ **独立阈值**: 每个缺陷类型可独立设置匹配阈值  
✅ **启用/禁用**: 动态控制缺陷类型的启用状态  

---

## 📁 配置文件格式

### 完整示例

```json
{
  "version": "1.0",
  "description": "缺陷枚举配置",
  "metadata": {
    "author": "Your Team",
    "embedding_dim": 128,
    "default_threshold": 0.5
  },
  "defects": [
    {
      "id": "DEFECT_001",
      "name": "墙皮脱落",
      "description": "墙面涂层或抹灰层脱落现象",
      "keywords": [
        "墙皮脱落",
        "墙面脱落",
        "墙留坠",
        "墙壁掉皮"
      ],
      "categories": ["墙面", "装饰层"],
      "threshold": 0.5,
      "enabled": true
    }
  ]
}
```

### 字段说明

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `id` | string | ✅ | 缺陷唯一标识符 |
| `name` | string | ✅ | 缺陷标准名称 |
| `description` | string | ❌ | 缺陷描述 |
| `keywords` | array | ❌ | 关键词/同义词列表 |
| `categories` | array | ❌ | 分类标签（用于筛选） |
| `threshold` | float | ❌ | 匹配阈值（默认 0.5） |
| `enabled` | boolean | ❌ | 是否启用（默认 true） |

---

## 💻 使用方式

### 方式一：从配置文件加载

#### C++ 示例

```cpp
#include "embedding_matcher.h"
#include "resource_manager.h"

using namespace ai_glasses;

int main() {
    // 1. 初始化资源管理器
    ResourceManager& rm = ResourceManager::getInstance();
    
    ResourceConfig config;
    config.embedding_model_path = "models/embedding.txt";
    config.jieba_dict_path = "dict/jieba.dict";
    config.defect_config_path = "config/defects.json";
    config.cache_dir = "cache";
    config.embedding_dim = 128;
    config.default_threshold = 0.5f;
    
    rm.initialize(config);
    
    // 2. 创建匹配器并从配置加载
    EmbeddingMatcher matcher;
    matcher.loadDefectConfig("config/defects.json");
    
    std::cout << "已加载 " << matcher.getEnumItemCount() 
              << " 个缺陷类型" << std::endl;
    
    // 3. 执行匹配
    MatchResult result = matcher.findBestMatch("墙留坠很严重");
    
    if (result.is_match) {
        std::cout << "匹配结果：" << result.matched_text << std::endl;
        std::cout << "相似度：" << result.similarity << std::endl;
    }
    
    return 0;
}
```

#### Android/Java 示例

```java
public class DefectMatcherApp {
    private EmbeddingMatcher matcher;
    private ResourceManager resourceManager;
    
    public DefectMatcherApp(Context context) {
        // 1. 初始化资源管理器
        resourceManager = new ResourceManager(context);
        resourceManager.initialize();
        
        // 2. 创建匹配器
        matcher = new EmbeddingMatcher();
        
        // 3. 从配置文件加载
        String configPath = resourceManager.getConfigPath("defects.json");
        String modelPath = resourceManager.getModelPath("embedding.txt");
        String dictPath = resourceManager.getDictPath("jieba.dict");
        
        // 加载配置（通过 JNI 调用）
        loadConfig(configPath, modelPath, dictPath);
    }
    
    private native void loadConfig(String configPath, String modelPath, String dictPath);
    
    public MatchResult match(String text) {
        return matcher.findBestMatch(text);
    }
}
```

### 方式二：从字符串加载（动态配置）

```cpp
#include "embedding_matcher.h"

EmbeddingMatcher matcher;
matcher.initialize("", "");

std::string configJson = R"({
  "defects": [
    {
      "id": "DEFECT_001",
      "name": "墙皮脱落",
      "keywords": ["墙皮脱落", "墙面脱落", "墙留坠"],
      "threshold": 0.5,
      "enabled": true
    }
  ]
})";

matcher.loadDefectConfigFromString(configJson);
```

### 方式三：动态添加缺陷

```cpp
EmbeddingMatcher matcher;
matcher.initialize("", "");

// 添加单个缺陷
matcher.addEnumItem("DEFECT_001", "墙皮脱落", "墙面涂层脱落", {
    "墙皮脱落", "墙面脱落", "墙留坠"
}, 0.5f);

// 添加带分类的缺陷（通过配置）
std::string configJson = R"({
  "defects": [
    {
      "id": "DEFECT_002",
      "name": "裂缝",
      "keywords": ["裂缝", "裂痕", "开裂"],
      "categories": ["结构", "墙面"],
      "threshold": 0.55,
      "enabled": true
    }
  ]
})";

matcher.loadDefectConfigFromString(configJson);
```

---

## 🔧 资源管理

### 目录结构

```
app_files/
├── models/
│   └── embedding.txt          # Embedding 模型文件
├── config/
│   └── defects.json           # 缺陷配置文件
├── dict/
│   └── jieba.dict             # 结巴分词词典
└── cache/
    └── embeddings.bin         # 预计算的向量缓存
```

### Android Asset 管理

将资源文件放在 `assets/aiglasses/` 目录下：

```
app/src/main/assets/aiglasses/
├── models/
│   └── embedding.txt
├── config/
│   └── defects.json
└── dict/
    └── jieba.dict
```

应用首次启动时会自动复制到内部存储。

---

## 🎨 高级功能

### 1. 按类别筛选

```cpp
// 只匹配"墙面"类别的缺陷
auto results = matcher.findByCategory("墙面", "墙留坠很严重");

for (const auto& result : results) {
    std::cout << result.matched_text << " (" 
              << result.similarity << ")" << std::endl;
}
```

### 2. 动态启用/禁用

```cpp
// 禁用某个缺陷类型
matcher.disableEnumItem("DEFECT_001");

// 启用某个缺陷类型
matcher.enableEnumItem("DEFECT_001");

// 检查是否启用
bool enabled = matcher.isEnumItemEnabled("DEFECT_001");

// 获取启用的缺陷数量
size_t enabled_count = matcher.getEnabledEnumItemCount();
```

### 3. 预计算向量缓存

```cpp
// 保存预计算的向量
matcher.savePrecomputedEmbeddings("cache/embeddings.bin");

// 加载预计算的向量（加速启动）
matcher.loadPrecomputedEmbeddings("cache/embeddings.bin");
```

### 4. 批量加载配置

```cpp
// 从多个配置文件加载
matcher.loadDefectConfig("config/defects_wall.json");
matcher.loadDefectConfig("config/defects_floor.json");
matcher.loadDefectConfig("config/defects_roof.json");

// 总共加载的缺陷数量
std::cout << "Total defects: " << matcher.getEnumItemCount() << std::endl;
```

---

## 📊 配置管理最佳实践

### 1. 模块化配置

将不同类型的缺陷分开配置：

```
config/
├── defects_wall.json      # 墙面缺陷
├── defects_floor.json     # 地面缺陷
├── defects_roof.json      # 屋面缺陷
└── defects_structure.json # 结构缺陷
```

### 2. 版本控制

在配置中包含版本信息：

```json
{
  "version": "1.0.2",
  "updated_at": "2024-01-15",
  "change_log": "新增钢筋外露缺陷类型",
  "defects": [...]
}
```

### 3. 环境区分

为不同环境准备不同配置：

```
config/
├── defects_dev.json       # 开发环境（包含测试数据）
├── defects_test.json      # 测试环境
└── defects_prod.json      # 生产环境（精简配置）
```

### 4. 热更新配置

```java
public class ConfigManager {
    private EmbeddingMatcher matcher;
    
    public boolean updateConfig(String newConfigUrl) {
        // 下载新配置
        String newConfig = downloadConfig(newConfigUrl);
        
        // 验证配置格式
        if (!validateConfig(newConfig)) {
            return false;
        }
        
        // 动态更新
        return matcher.loadDefectConfigFromString(newConfig);
    }
    
    public boolean rollbackConfig() {
        // 回滚到备份配置
        return matcher.loadDefectConfig("config/defects_backup.json");
    }
}
```

---

## 🔍 调试技巧

### 1. 查看加载的缺陷

```cpp
auto configs = loader.getConfigs();
for (const auto& config : configs) {
    std::cout << "ID: " << config.id << std::endl;
    std::cout << "  Name: " << config.name << std::endl;
    std::cout << "  Keywords: " << config.keywords.size() << std::endl;
    std::cout << "  Threshold: " << config.threshold << std::endl;
    std::cout << "  Enabled: " << (config.enabled ? "Yes" : "No") << std::endl;
}
```

### 2. 验证配置有效性

```cpp
bool validateConfig(const std::string& configPath) {
    EmbeddingMatcher matcher;
    
    if (!matcher.loadDefectConfig(configPath)) {
        std::cerr << "配置加载失败" << std::endl;
        return false;
    }
    
    if (matcher.getEnumItemCount() == 0) {
        std::cerr << "配置中没有缺陷定义" << std::endl;
        return false;
    }
    
    std::cout << "配置验证通过：" 
              << matcher.getEnumItemCount() << " 个缺陷" << std::endl;
    return true;
}
```

### 3. 性能测试

```cpp
auto start = std::chrono::high_resolution_clock::now();

for (int i = 0; i < 1000; ++i) {
    matcher.findBestMatch("墙留坠");
}

auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

std::cout << "1000 次匹配耗时：" << duration.count() << "ms" << std::endl;
std::cout << "平均每次匹配：" << duration.count() / 1000.0 << "ms" << std::endl;
```

---

## ⚠️ 常见问题

### Q1: 配置文件加载失败？

**A**: 检查以下几点：
1. JSON 格式是否正确（使用 JSON 验证工具）
2. 文件路径是否正确
3. 文件编码是否为 UTF-8
4. 是否有读取权限

### Q2: 如何更新线上配置？

**A**: 
1. 将新配置上传到服务器
2. 客户端下载新配置
3. 验证配置有效性
4. 调用 `loadDefectConfigFromString()` 动态更新
5. 保存备份配置

### Q3: 如何优化大配置文件？

**A**:
1. 使用模块化配置（按类别分开）
2. 预计算向量并缓存
3. 按需加载（只加载需要的类别）
4. 压缩配置文件（使用 gzip）

### Q4: Embedding 模型文件太大？

**A**:
1. 使用量化模型（float32 → float16）
2. 减少向量维度（128 → 64）
3. 使用模型压缩技术
4. 云端部署 + 本地缓存

---

## 📝 配置模板

### 最小配置

```json
{
  "defects": [
    {
      "id": "DEFECT_001",
      "name": "墙皮脱落",
      "keywords": ["墙皮脱落", "墙留坠"]
    }
  ]
}
```

### 完整配置

```json
{
  "version": "1.0",
  "description": "缺陷枚举配置",
  "metadata": {
    "author": "Team",
    "embedding_dim": 128,
    "default_threshold": 0.5
  },
  "defects": [
    {
      "id": "DEFECT_001",
      "name": "墙皮脱落",
      "description": "墙面涂层脱落",
      "keywords": [
        "墙皮脱落",
        "墙面脱落",
        "墙留坠"
      ],
      "categories": ["墙面", "装饰层"],
      "threshold": 0.5,
      "enabled": true
    }
  ]
}
```

---

## 🎯 总结

通过本配置系统，你可以：

✅ 灵活管理缺陷枚举数据  
✅ 支持本地 Embedding 模型资源管理  
✅ 动态加载和更新配置  
✅ 按类别筛选缺陷  
✅ 独立控制每个缺陷的阈值和启用状态  
✅ 支持配置热更新  

所有代码已实现完毕，可以直接使用！
