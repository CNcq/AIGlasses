# Embedding 模型使用指南

## 📦 模型文件说明

### 文件位置

项目已包含一个预生成的示例 Embedding 模型：

```
resources/models/embedding.txt          # 主要资源目录
scripts/models/embedding.txt            # 脚本生成目录（原始文件）
```

### 模型规格

- **词汇表大小**: 2000 词
- **向量维度**: 128 维
- **文件大小**: ~2.4 MB
- **格式**: 文本格式（UTF-8 编码）

### 文件格式

```
第一行：词汇表大小 向量维度
后续行：词语 向量值（空格分隔）
```

示例：
```
2000 128
墙皮 0.123456 -0.234567 0.345678 ... (128 个浮点数)
脱落 -0.456789 0.567890 -0.678901 ... (128 个浮点数)
```

---

## 🛠️ 生成/下载模型

### 方法一：创建示例模型（推荐用于测试）

```bash
# 在项目根目录执行
python scripts/download_embedding_model.py --action create
```

**参数说明**：
- `--vocab-size`: 词汇表大小（默认 1000）
- `--dim`: 向量维度（默认 128）
- `--output`: 输出文件路径（默认 `resources/models/embedding.txt`）

**示例**：
```bash
# 创建 2000 词、128 维的模型
python scripts/download_embedding_model.py --action create --vocab-size 2000 --dim 128

# 自定义输出路径
python scripts/download_embedding_model.py --action create --output my_models/my_embedding.txt
```

### 方法二：转换现有 Word2Vec 模型

如果你有现成的 Word2Vec 模型文件：

```bash
# 转换文本格式
python scripts/download_embedding_model.py --action convert \
  --input path/to/your/model.txt \
  --output resources/models/embedding.txt

# 转换二进制格式（如 Google News 模型）
python scripts/download_embedding_model.py --action convert \
  --input path/to/your/model.bin \
  --output resources/models/embedding.txt \
  --binary
```

### 方法三：下载预训练模型（需要网络）

```bash
# 下载中文预训练模型
python scripts/download_embedding_model.py --action download \
  --model-name zh \
  --output resources/models/embedding.txt
```

**支持的模型名称**：
- `zh`: 通用中文模型
- `zh-news`: 新闻语料训练
- `zh-web`: 网页语料训练
- `zh-wiki`: 维基百科语料训练

**注意**：下载需要网络连接和较长时间，且需要安装 `gensim` 库。

---

## ✅ 验证模型

验证模型文件格式是否正确：

```bash
python scripts/download_embedding_model.py --action verify \
  --output resources/models/embedding.txt
```

**输出示例**：
```
验证模型：resources/models/embedding.txt
  词汇表大小：2000
  向量维度：128
  实际词汇数：2000
✓ 模型验证通过
  文件大小：2397.70 KB
```

---

## 📋 使用模型

### C++ 中使用

```cpp
#include "embedding_model.h"
#include "embedding_matcher.h"

using namespace ai_glasses;

int main() {
    // 1. 加载 Embedding 模型
    EmbeddingModel model;
    model.loadFromFile("resources/models/embedding.txt");
    
    std::cout << "词汇表大小：" << model.getVocabSize() << std::endl;
    std::cout << "向量维度：" << model.getDimension() << std::endl;
    
    // 2. 获取词语向量
    EmbeddingVector vec = model.getEmbedding("墙皮");
    
    if (!vec.empty()) {
        std::cout << "成功获取'墙皮'的向量" << std::endl;
    }
    
    // 3. 使用 EmbeddingMatcher
    EmbeddingMatcher matcher;
    matcher.initialize(
        "resources/dict/jieba.dict",
        "resources/models/embedding.txt"
    );
    
    // 4. 加载缺陷配置
    matcher.loadDefectConfig("resources/config/defects.json");
    
    // 5. 执行匹配
    auto result = matcher.findBestMatch("墙留坠很严重");
    
    if (result.is_match) {
        std::cout << "匹配结果：" << result.matched_text << std::endl;
        std::cout << "相似度：" << result.similarity << std::endl;
    }
    
    return 0;
}
```

### Android 中使用

```java
public class MainActivity {
    private EmbeddingMatcher matcher;
    private ResourceManager resourceManager;
    
    public void init() {
        // 1. 初始化资源管理器
        resourceManager = new ResourceManager(context);
        resourceManager.initialize();
        
        // 2. 创建匹配器
        matcher = new EmbeddingMatcher();
        
        // 3. 获取资源路径
        String modelPath = resourceManager.getModelPath("embedding.txt");
        String dictPath = resourceManager.getDictPath("jieba.dict");
        String configPath = resourceManager.getConfigPath("defects.json");
        
        // 4. 初始化
        matcher.initialize(dictPath, modelPath);
        
        // 5. 加载配置
        matcher.loadDefectConfig(configPath);
        
        // 6. 使用
        MatchResult result = matcher.findBestMatch("墙留坠");
    }
}
```

---

## 🔧 自定义模型

### 训练自己的 Embedding 模型

#### 1. 准备语料

```python
# 准备建筑缺陷领域的语料
corpus = [
    "墙皮脱落是常见的建筑缺陷",
    "墙面出现裂缝需要及时处理",
    "空鼓现象可以通过敲击检查",
    # ... 更多领域相关文本
]

# 保存语料
with open('corpus.txt', 'w', encoding='utf-8') as f:
    for line in corpus:
        f.write(line + '\n')
```

#### 2. 使用 Gensim 训练

```python
from gensim.models import Word2Vec
import jieba

# 读取语料
sentences = []
with open('corpus.txt', 'r', encoding='utf-8') as f:
    for line in f:
        # 使用结巴分词
        words = list(jieba.cut(line.strip()))
        sentences.append(words)

# 训练 Word2Vec
model = Word2Vec(
    sentences,
    vector_size=128,
    window=5,
    min_count=1,
    workers=4
)

# 保存模型
model.wv.save_word2vec_format('my_embedding.txt', binary=False)
```

#### 3. 转换格式

```bash
python scripts/download_embedding_model.py --action verify \
  --output my_embedding.txt
```

---

## 📊 模型优化建议

### 1. 词汇表大小

- **小型**: 500-1000 词（适合移动端，快速）
- **中型**: 2000-5000 词（平衡性能和精度）
- **大型**: 10000+ 词（精度高，但文件大）

### 2. 向量维度

- **64 维**: 最小，速度快，精度一般
- **128 维**: 推荐，平衡性能和精度
- **256 维**: 高精度，但计算量大
- **300 维**: 标准 Word2Vec 维度

### 3. 领域适配

建议针对建筑缺陷领域：
- 收集领域相关文本（1000+ 条）
- 包含所有缺陷类型的描述
- 包含同义词、谐音词
- 使用领域语料训练或微调

### 4. 性能优化

```cpp
// 预计算向量并缓存
matcher.savePrecomputedEmbeddings("cache/embeddings.bin");

// 下次启动时加载缓存
matcher.loadPrecomputedEmbeddings("cache/embeddings.bin");
```

---

## ⚠️ 注意事项

### 1. 文件编码

- 必须使用 **UTF-8** 编码
- 避免使用 BOM 头

### 2. 向量归一化

建议对向量进行 L2 归一化，这样可以使用余弦相似度：

```python
import numpy as np

# L2 归一化
vector = vector / np.linalg.norm(vector)
```

### 3. 内存使用

模型文件大小与内存占用关系：
- 2000 词 × 128 维 ≈ 1MB 文件 ≈ 1MB 内存
- 10000 词 × 128 维 ≈ 5MB 文件 ≈ 5MB 内存

### 4. 加载时间

- 小模型（<1MB）: <100ms
- 中模型（1-5MB）: 100-500ms
- 大模型（>5MB）: >500ms

建议使用预计算缓存加速启动。

---

## 🔍 故障排查

### 问题 1: 模型加载失败

**检查**：
1. 文件路径是否正确
2. 文件编码是否为 UTF-8
3. 第一行格式是否为 `vocab_size dim`
4. 向量维度是否一致

**解决**：
```bash
# 验证模型
python scripts/download_embedding_model.py --action verify
```

### 问题 2: 词语向量找不到

**原因**：词语不在词汇表中

**解决**：
1. 检查词语是否在词汇表中
2. 使用更大的词汇表
3. 使用同义词映射

### 问题 3: 内存不足

**解决**：
1. 减小词汇表大小
2. 减小向量维度
3. 使用预计算缓存

---

## 📝 总结

### 快速开始

```bash
# 1. 生成示例模型
python scripts/download_embedding_model.py --action create

# 2. 验证模型
python scripts/download_embedding_model.py --action verify

# 3. 在代码中使用
# 参见上面的 C++/Java 示例
```

### 推荐配置

对于移动端应用：
- **词汇表**: 2000-3000 词
- **维度**: 128 维
- **格式**: 文本格式（便于调试）
- **缓存**: 启用预计算缓存

### 进阶使用

- 使用领域语料训练
- 定期更新词汇表
- 监控匹配精度
- 优化模型大小

---

**模型文件已生成在**: `resources/models/embedding.txt`

可以直接使用！🎉
