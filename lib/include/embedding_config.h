#ifndef EMBEDDING_CONFIG_H
#define EMBEDDING_CONFIG_H

#include <string>
#include <vector>
#include <unordered_map>

namespace ai_glasses {

struct EmbeddingConfig {
    // 词向量配置
    struct WordVector {
        std::string word;
        std::vector<float> vector;
    };
    
    // 配置项
    std::vector<std::string> words;  // 只存储词语，向量自动生成
    size_t embedding_dim = 128;
    bool auto_initialize = true;
    
    // 加载词向量
    bool loadWordVectors(const std::string& file_path);
    
    // 保存词向量
    bool saveWordVectors(const std::string& file_path) const;
    
    // 从 JSON 字符串加载
    bool loadFromJson(const std::string& json_string);
    
    // 转换为 JSON 字符串
    std::string toJson() const;
    
    // 生成随机向量（用于没有提供向量的词）
    void generateRandomVectors();
    
    // 获取词向量
    const std::vector<float>& getVector(const std::string& word) const;
    
    // 检查词是否存在
    bool hasWord(const std::string& word) const;
};

// 词向量配置管理器
class EmbeddingConfigManager {
public:
    EmbeddingConfigManager();
    ~EmbeddingConfigManager();
    
    // 加载配置
    bool loadFromFile(const std::string& config_path);
    bool loadFromJson(const std::string& json_string);
    
    // 获取配置
    const EmbeddingConfig& getConfig() const;
    
    // 应用配置到 EmbeddingMatcher
    bool applyToMatcher(void* matcher_ptr);  // void* 是为了避免依赖 embedding_matcher.h
    
    // 添加词
    bool addWord(const std::string& word);
    
    // 批量添加词
    bool addWords(const std::vector<std::string>& words);
    
    // 清空配置
    void clear();
    
private:
    EmbeddingConfig config_;
};

}

#endif
