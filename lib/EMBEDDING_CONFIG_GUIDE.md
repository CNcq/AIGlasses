# 词向量配置系统使用指南

## 概述

词向量配置系统采用动态导入方式，用户可以在代码中直接输入缺陷数组，系统会自动为每个词生成随机向量，使词向量管理更加灵活和可配置。

## 使用流程

### 动态缺陷数组配置

```cpp
#include "embedding_matcher.h"

using namespace ai_glasses;

int main() {
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    // 用户输入的缺陷数组
    std::vector<std::string> defect_list = {"铜质", "铁质", "铝合金质"};
    
    // 动态添加缺陷
    matcher.addDefectsFromList(defect_list);
    
    // 匹配文本
    MatchResult result = matcher.findBestMatch("这个变电箱材质为铜");
    
    std::cout << "Matched: " << result.matched_text << std::endl;
    std::cout << "Similarity: " << result.similarity << std::endl;
    
    return 0;
}
```

## 核心 API

### addDefectsFromList

动态添加缺陷数组。

```cpp
void addDefectsFromList(const std::vector<std::string>& defect_list);
```

**参数**:
- `defect_list`: 缺陷名称数组

**示例**:
```cpp
std::vector<std::string> defects = {"铜质", "铁质", "铝合金质"};
matcher.addDefectsFromList(defects);
```

## 故障排查

### 问题 1: 缺陷未匹配

```
Input: 这个变电箱材质为铜
Matched: (empty)
Similarity: 0.0
```

**解决方案**:
- 检查输入文本是否包含缺陷关键词
- 调整匹配阈值
- 确保缺陷数组已正确添加

### 问题 2: 匹配效果不佳

```
匹配结果不准确
```

**解决方案**:
- 检查缺陷关键词是否覆盖全面
- 调整 threshold 阈值
- 增加同义词和近义词

## 总结

词向量配置系统提供了灵活的词向量管理方式，支持动态导入缺陷数组，系统自动为每个词生成随机向量，使词向量的维护和更新更加方便。

**核心要点**:
1. ✅ 动态添加缺陷: 使用 `addDefectsFromList` 方法
2. ✅ 缺陷名称: 完整、有意义
3. ✅ 阈值设置: 合理范围 0.5-0.7
4. ✅ 测试验证: 运行测试验证效果

## 核心 API

### addDefectsFromList

动态添加缺陷数组。

```cpp
void addDefectsFromList(const std::vector<std::string>& defect_list);
```

**参数**:
- `defect_list`: 缺陷名称数组

**示例**:
```cpp
std::vector<std::string> defects = {"铜质", "铁质", "铝合金质"};
matcher.addDefectsFromList(defects);
```

## 故障排查

### 问题 1: 缺陷未匹配

```
Input: 这个变电箱材质为铜
Matched: (empty)
Similarity: 0.0
```

**解决方案**:
- 检查输入文本是否包含缺陷关键词
- 调整匹配阈值
- 确保缺陷数组已正确添加

### 问题 2: 匹配效果不佳

```
匹配结果不准确
```

**解决方案**:
- 检查缺陷关键词是否覆盖全面
- 调整 threshold 阈值
- 增加同义词和近义词

## 总结

词向量配置系统提供了灵活的词向量管理方式，支持动态导入缺陷数组，系统自动为每个词生成随机向量，使词向量的维护和更新更加方便。

**核心要点**:
1. ✅ 动态添加缺陷: 使用 `addDefectsFromList` 方法
2. ✅ 缺陷名称: 完整、有意义
3. ✅ 阈值设置: 合理范围 0.5-0.7
4. ✅ 测试验证: 运行测试验证效果

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

### 动态添加缺陷

```cpp
// 一次性添加所有缺陷
std::vector<std::string> defects = {"铜质", "铁质", "铝合金质"};
matcher.addDefectsFromList(defects);
```

## 故障排查

### 问题 1: 缺陷未匹配

```
Input: 这个变电箱材质为铜
Matched: (empty)
Similarity: 0.0
```

**解决方案**:
- 检查输入文本是否包含缺陷关键词
- 调整匹配阈值
- 确保缺陷数组已正确添加

### 问题 2: 匹配效果不佳

```
匹配结果不准确
```

**解决方案**:
- 检查缺陷关键词是否覆盖全面
- 调整 threshold 阈值
- 增加同义词和近义词

## 总结

词向量配置系统提供了灵活的词向量管理方式，支持动态导入缺陷数组，系统自动为每个词生成随机向量，使词向量的维护和更新更加方便。

**核心要点**:
1. ✅ 动态添加缺陷: 使用 `addDefectsFromList` 方法
2. ✅ 缺陷名称: 完整、有意义
3. ✅ 阈值设置: 合理范围 0.5-0.7
4. ✅ 测试验证: 运行测试验证效果
