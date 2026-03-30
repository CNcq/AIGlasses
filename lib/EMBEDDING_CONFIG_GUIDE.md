# 词向量配置系统使用指南

## 概述

词向量配置系统允许你从 JSON 配置文件加载词语，系统会自动为每个词生成随机向量，使词向量管理更加灵活和可配置。

## 文件结构

```
resources/
├── embedding_config.json      # 词向量配置文件（词语列表）
└── config/
    └── defects_example.json   # 缺陷枚举配置文件（缺陷定义）
```

## 配置文件说明

### 1. embedding_config.json - 词向量配置文件

**作用**: 定义所有需要计算向量的词语列表

**格式**:
```json
{
  "embedding_dim": 128,
  "words": [
    "箱体材质",
    "不锈钢",
    "塑料",
    "隔离开关",
    "变形破损",
    "老化"
  ]
}
```

**字段说明**:
- `embedding_dim`: 向量维度（默认 128）
- `words`: 词语列表（不需要提供向量，系统自动生成）

**注意事项**:
- ✅ 只包含有意义的完整词语（2个字或以上）
- ✅ 避免单字词（如"锈"、"蚀"、"破"）
- ✅ 避免停用词（如"或"、"不"、"有"、"无"）
- ✅ 避免重复词语
- ✅ 保持词语的语义完整性

### 2. defects_example.json - 缺陷枚举配置文件

**作用**: 定义具体的缺陷类型及其关键词

**格式**:
```json
{
  "version": "1.0",
  "description": "缺陷枚举配置",
  "metadata": {
    "embedding_dim": 128,
    "default_threshold": 0.5
  },
  "defects": [
    {
      "id": "MET_BOX_001",
      "name": "箱体材质",
      "description": "计量箱箱体材质类型",
      "keywords": [
        "箱体材质",
        "不锈钢",
        "塑料",
        "铁"
      ],
      "categories": ["箱体", "材质"],
      "threshold": 0.6,
      "enabled": true
    }
  ]
}
```

**字段说明**:

**根字段**:
- `version`: 配置文件版本
- `description`: 配置描述
- `metadata`: 元数据
  - `embedding_dim`: 向量维度
  - `default_threshold`: 默认匹配阈值

**defects 数组**:
- `id`: 缺陷唯一标识（如 "MET_BOX_001"）
- `name`: 缺陷名称
- `description`: 缺陷描述
- `keywords`: 关键词列表（与 embedding_config.json 中的词对应）
- `categories`: 分类标签
- `threshold`: 匹配阈值（0.0-1.0，越高要求越严格）
- `enabled`: 是否启用

## 使用流程

### 完整配置流程

#### 步骤 1: 准备词向量配置

1. 创建或更新 `resources/embedding_config.json`
2. 只包含需要计算向量的词语
3. 确保词语是完整的、有意义的

**示例**:
```json
{
  "embedding_dim": 128,
  "words": [
    "箱体材质",
    "不锈钢",
    "塑料",
    "隔离开关",
    "塑壳断路器",
    "微型断路器",
    "变形破损",
    "老化",
    "缺失",
    "不牢固"
  ]
}
```

#### 步骤 2: 准备缺陷配置

1. 创建或更新 `resources/config/defects_example.json`
2. 定义所有缺陷类型
3. 为每个缺陷配置关键词（从 embedding_config.json 中选择）

**示例**:
```json
{
  "defects": [
    {
      "id": "MET_BOX_001",
      "name": "箱体材质",
      "keywords": ["箱体材质", "不锈钢", "塑料", "铁"],
      "categories": ["箱体", "材质"],
      "threshold": 0.6
    },
    {
      "id": "MET_BOX_002",
      "name": "开关故障",
      "keywords": ["隔离开关", "塑壳断路器", "微型断路器", "开关故障"],
      "categories": ["开关"],
      "threshold": 0.5
    }
  ]
}
```

#### 步骤 3: 在代码中使用

```cpp
#include "embedding_matcher.h"
#include "defect_config_loader.h"

using namespace ai_glasses;

// 1. 创建匹配器
EmbeddingMatcher matcher;

// 2. 加载词向量配置
matcher.loadEmbeddingConfig("resources/embedding_config.json");

// 3. 加载缺陷配置
DefectConfigLoader loader;
loader.loadFromFile("resources/config/defects_example.json");
loader.applyToMatcher(matcher);

// 4. 初始化
matcher.initialize("", "");

// 5. 预计算向量（可选，但推荐）
matcher.precomputeAndSaveEnumVectors("cache/embedding_cache.bin");

// 6. 进行匹配
MatchResult result = matcher.findBestMatch("箱体锈蚀破损");
```

## 更新配置文件

### 场景 1: 添加新缺陷

1. **在 defects_example.json 中添加新缺陷**:
```json
{
  "id": "MET_BOX_003",
  "name": "新缺陷名称",
  "keywords": ["关键词1", "关键词2"],
  "categories": ["分类1"],
  "threshold": 0.5
}
```

2. **在 embedding_config.json 中添加关键词**（如果不存在）:
```json
{
  "words": [..., "新关键词"]
}
```

3. **重新编译并运行**

### 场景 2: 修改缺陷阈值

直接修改 defects_example.json 中的 threshold 值：
```json
{
  "id": "MET_BOX_001",
  "threshold": 0.7  // 提高阈值，要求更严格
}
```

### 场景 3: 删除缺陷

将 defects_example.json 中不需要的缺陷项删除：
```json
{
  "defects": [
    // 删除不需要的缺陷
  ]
}
```

### 场景 4: 批量更新

1. 导出当前配置
2. 使用 Excel 或文本编辑器批量修改
3. 替换配置文件
4. 验证配置正确性

## 配置文件最佳实践

### 1. 词语选择原则

**✅ 推荐**:
- 完整的词语（2个字或以上）
- 有意义的词汇
- 覆盖所有可能的表达方式
- 包含同义词和近义词

**❌ 避免**:
- 单字词
- 停用词
- 过于泛化的词
- 重复的词

### 2. 缺陷配置原则

**✅ 推荐**:
- 每个缺陷有明确的定义
- 关键词覆盖所有可能的表达
- 合理设置阈值（0.5-0.7）
- 添加分类标签便于管理

**❌ 避免**:
- 关键词太少（<3个）
- 阈值过高（>0.8）或过低（<0.3）
- 缺陷定义模糊
- 不添加分类标签

### 3. 版本管理

**推荐做法**:
```bash
# 将配置文件纳入版本控制
git add resources/embedding_config.json
git add resources/config/defects_example.json
git commit -m "Update defect configuration"
```

### 4. 测试验证

每次修改配置后，运行测试验证：
```cpp
// 测试匹配效果
MatchResult result = matcher.findBestMatch("箱体锈蚀");
assert(!result.matched_id.empty());
```

## 生成配置文件

### 方法 1: 手动编写

按照上述格式手动编写 JSON 文件。

### 方法 2: 使用 Python 脚本生成

```python
import json

# 创建词向量配置
embedding_config = {
    "embedding_dim": 128,
    "words": [
        "箱体材质",
        "不锈钢",
        "塑料",
        "隔离开关"
    ]
}

# 创建缺陷配置
defects_config = {
    "version": "1.0",
    "defects": [
        {
            "id": "MET_BOX_001",
            "name": "箱体材质",
            "keywords": ["箱体材质", "不锈钢", "塑料"],
            "categories": ["箱体", "材质"],
            "threshold": 0.6
        }
    ]
}

# 保存到文件
with open("embedding_config.json", "w", encoding='utf-8') as f:
    json.dump(embedding_config, f, indent=2, ensure_ascii=False)

with open("defects_config.json", "w", encoding='utf-8') as f:
    json.dump(defects_config, f, indent=2, ensure_ascii=False)
```

### 方法 3: 从 Excel 生成

1. 在 Excel 中整理词语和缺陷
2. 使用 Python 脚本转换为 JSON

```python
import pandas as pd
import json

# 从 Excel 读取
df = pd.read_excel("config.xlsx")

# 转换为 JSON
words = df['词语'].tolist()
embedding_config = {
    "embedding_dim": 128,
    "words": words
}

with open("embedding_config.json", "w") as f:
    json.dump(embedding_config, f, indent=2, ensure_ascii=False)
```

## 配置文件对比

### 之前（需要提供向量）：
```json
{
  "embedding_dim": 128,
  "word_vectors": [
    {
      "word": "墙壁",
      "vector": [1.0, 0.0, 0.0, ...]  // 需要提供完整的向量
    }
  ]
}
```

### 现在（只需提供词语）：
```json
{
  "embedding_dim": 128,
  "words": ["墙壁", "脱落", "墙皮"]  // 只需要词语
}
```

## 自动向量生成

系统会在运行时自动为每个词生成随机向量，确保：
- 每个词都有唯一的向量表示
- 向量维度正确
- 向量值在合理范围内（-1.0 到 1.0）

## 性能优化

### 使用预计算缓存

```cpp
// 预计算并保存枚举项的向量
matcher.precomputeAndSaveEnumVectors("cache/embedding_cache.bin");

// 加载预计算的向量
matcher.loadPrecomputedEnumVectors("cache/embedding_cache.bin");
```

### 批量加载

```cpp
// 一次性加载所有词语
matcher.loadEmbeddingConfig("resources/embedding_config.json");

// 然后添加所有枚举项
matcher.addEnumItem("wall_peel", "墙皮脱落", {"墙壁", "脱落"});
matcher.addEnumItem("crack", "裂缝", {"裂缝", "裂痕"});
```

## 故障排查

### 问题 1: 找不到配置文件

```
Error: Failed to open config file
```

**解决方案**: 检查文件路径是否正确

### 问题 2: JSON 解析失败

```
Error: Failed to parse JSON
```

**解决方案**: 
- 检查 JSON 格式是否正确
- 使用在线 JSON 校验工具验证
- 确保文件编码为 UTF-8

### 问题 3: 向量维度不匹配

```
Error: Vector dimension mismatch
```

**解决方案**: 确保配置文件中的 `embedding_dim` 与实际向量维度一致

### 问题 4: 匹配效果不佳

```
匹配结果不准确
```

**解决方案**:
- 检查关键词是否覆盖全面
- 调整 threshold 阈值
- 增加同义词和近义词
- 检查输入文本是否正确

### 问题 5: 配置文件过大

```
配置文件包含大量无意义的词
```

**解决方案**:
- 移除单字词
- 移除停用词
- 合并重复的词
- 只保留有实际意义的词语

## 总结

词向量配置系统提供了灵活的词向量管理方式，支持从 JSON 配置文件加载词语，系统自动为每个词生成随机向量，使词向量的维护和更新更加方便。

**核心要点**:
1. ✅ embedding_config.json: 只包含需要计算向量的词语
2. ✅ defects_example.json: 定义缺陷类型及其关键词
3. ✅ 词语选择: 完整、有意义、避免重复
4. ✅ 阈值设置: 合理范围 0.5-0.7
5. ✅ 版本管理: 纳入 Git 等版本控制系统
6. ✅ 测试验证: 修改后运行测试验证效果
