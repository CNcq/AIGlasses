#ifndef EMBEDDING_MATCHER_H
#define EMBEDDING_MATCHER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <array>

namespace ai_glasses {

constexpr size_t EMBEDDING_DIM = 128;

using EmbeddingVector = std::array<float, EMBEDDING_DIM>;

struct EnumItem {
    std::string id;
    std::string text;
    std::string description;
    std::vector<std::string> keywords;
    EmbeddingVector embedding;
    float threshold;
    bool enabled;
    bool is_precomputed;
};

struct MatchResult {
    std::string input_text;
    std::string matched_id;
    std::string matched_text;
    float similarity;
    int rank;
    bool is_match;
};

class JiebaSegmenter;
class EmbeddingModel;

class EmbeddingMatcher {
public:
    EmbeddingMatcher();
    ~EmbeddingMatcher();

    bool initialize(const std::string& jieba_dict_path, 
                   const std::string& embedding_model_path);
    
    void addEnumItem(const EnumItem& item);
    
    void addEnumItem(const std::string& id, const std::string& text,
                    const std::vector<std::string>& keywords = {});
    
    void addEnumItem(const std::string& id, const std::string& text,
                    const std::string& description,
                    const std::vector<std::string>& keywords = {},
                    float threshold = 0.5f);
    
    // 动态添加缺陷（用户输入的缺陷数组）
    void addDefectsFromList(const std::vector<std::string>& defect_list, bool clear_existing = false);
    
    MatchResult findBestMatch(const std::string& input_text);
    
    EmbeddingVector computeEmbedding(const std::string& text);
    
    void setSimilarityThreshold(float threshold);
    
    float getSimilarityThreshold() const;
    
    size_t getEnumItemCount() const;
    
    // 测试用：添加词向量
    bool addWordVector(const std::string& word, const std::vector<float>& vector);
    
    // 测试用：获取内部实现（用于添加词向量）
    // 注意：此方法仅用于测试目的，实际使用中不推荐
    void* getImpl() { return impl_.get(); }
    const void* getImpl() const { return impl_.get(); }

private:
    float cosineSimilarity(const EmbeddingVector& v1, const EmbeddingVector& v2);
    std::vector<std::string> segmentText(const std::string& text);
    EmbeddingVector computeTextEmbedding(const std::vector<std::string>& tokens);
    void computeEnumItemEmbedding(EnumItem& item);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}

#endif
