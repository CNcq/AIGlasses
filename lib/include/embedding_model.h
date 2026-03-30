#ifndef EMBEDDING_MODEL_H
#define EMBEDDING_MODEL_H

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_map>

namespace ai_glasses {

// 使用 embedding_matcher.h 中定义的常量
constexpr size_t DEFAULT_EMBEDDING_DIM = 128;

class EmbeddingModel {
public:
    EmbeddingModel();
    ~EmbeddingModel();

    bool initialize(const std::string& model_path, size_t embedding_dim = DEFAULT_EMBEDDING_DIM);
    
    // 使用固定的 128 维向量
    std::array<float, DEFAULT_EMBEDDING_DIM> computeEmbedding(const std::string& text);
    
    std::array<float, DEFAULT_EMBEDDING_DIM> computeEmbeddingFromTokens(const std::vector<std::string>& tokens);
    
    bool loadWordVectors(const std::string& word_vec_path);
    
    void addWordVector(const std::string& word, const std::vector<float>& vector);
    void addWordVector(const std::string& word, const std::array<float, 128>& vector);
    
    size_t getEmbeddingDim() const;
    
    size_t getVocabSize() const;
    
    bool hasWord(const std::string& word) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}

#endif
