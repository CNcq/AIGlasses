#ifndef EMBEDDING_MODEL_H
#define EMBEDDING_MODEL_H

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <unordered_map>

namespace ai_glasses {

constexpr size_t DEFAULT_EMBEDDING_DIM = 128;

template<size_t Dim = DEFAULT_EMBEDDING_DIM>
using EmbeddingVector = std::array<float, Dim>;

class EmbeddingModel {
public:
    EmbeddingModel();
    ~EmbeddingModel();

    bool initialize(const std::string& model_path, size_t embedding_dim = DEFAULT_EMBEDDING_DIM);
    
    template<size_t Dim = DEFAULT_EMBEDDING_DIM>
    EmbeddingVector<Dim> computeEmbedding(const std::string& text);
    
    template<size_t Dim = DEFAULT_EMBEDDING_DIM>
    EmbeddingVector<Dim> computeEmbeddingFromTokens(const std::vector<std::string>& tokens);
    
    bool loadWordVectors(const std::string& word_vec_path);
    
    void addWordVector(const std::string& word, const std::vector<float>& vector);
    
    size_t getEmbeddingDim() const;
    
    size_t getVocabSize() const;
    
    bool hasWord(const std::string& word) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

template<size_t Dim>
EmbeddingVector<Dim> EmbeddingModel::computeEmbedding(const std::string& text) {
    EmbeddingVector<Dim> result{};
    result.fill(0.0f);
    
    if (!impl_ || impl_->word_vectors.empty()) {
        return result;
    }
    
    std::vector<std::string> words;
    for (char c : text) {
        if (c > 0x80) {
            words.emplace_back(1, c);
        } else if (!words.empty() && words.back().size() == 1 && words.back()[0] <= 0x80) {
            words.back() += c;
        } else {
            words.emplace_back(1, c);
        }
    }
    
    return computeEmbeddingFromTokens<Dim>(words);
}

template<size_t Dim>
EmbeddingVector<Dim> EmbeddingModel::computeEmbeddingFromTokens(const std::vector<std::string>& tokens) {
    EmbeddingVector<Dim> result{};
    result.fill(0.0f);
    
    if (tokens.empty() || impl_->word_vectors.empty()) {
        return result;
    }
    
    size_t count = 0;
    for (const auto& token : tokens) {
        auto it = impl_->word_vectors.find(token);
        if (it != impl_->word_vectors.end()) {
            const auto& vec = it->second;
            for (size_t i = 0; i < std::min(Dim, vec.size()); ++i) {
                result[i] += vec[i];
            }
            count++;
        }
    }
    
    if (count > 0) {
        for (size_t i = 0; i < Dim; ++i) {
            result[i] /= static_cast<float>(count);
        }
    }
    
    return result;
}

}

#endif
