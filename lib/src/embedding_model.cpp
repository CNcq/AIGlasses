#include "embedding_model.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace ai_glasses {

struct EmbeddingModel::Impl {
    size_t embedding_dim = DEFAULT_EMBEDDING_DIM;
    std::unordered_map<std::string, std::vector<float>> word_vectors;
    bool initialized = false;

    bool loadFromTextFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        std::string header;
        if (!std::getline(file, header)) {
            return false;
        }

        std::istringstream header_stream(header);
        size_t vocab_size, dim;
        header_stream >> vocab_size >> dim;

        if (dim != embedding_dim) {
            embedding_dim = dim;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string word;
            iss >> word;

            std::vector<float> vector(dim);
            for (size_t i = 0; i < dim; ++i) {
                iss >> vector[i];
            }

            word_vectors[word] = vector;
        }

        initialized = true;
        return true;
    }

    bool loadFromBinaryFile(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        size_t vocab_size;
        file.read(reinterpret_cast<char*>(&vocab_size), sizeof(size_t));
        file.read(reinterpret_cast<char*>(&embedding_dim), sizeof(size_t));

        std::string line;
        while (std::getline(file, line)) {
        }
        file.clear();
        file.seekg(0, std::ios::beg);

        file.read(reinterpret_cast<char*>(&vocab_size), sizeof(size_t));
        file.read(reinterpret_cast<char*>(&embedding_dim), sizeof(size_t));

        for (size_t i = 0; i < vocab_size; ++i) {
            size_t word_len;
            file.read(reinterpret_cast<char*>(&word_len), sizeof(size_t));

            std::string word(word_len, '\0');
            file.read(&word[0], word_len);

            std::vector<float> vector(embedding_dim);
            file.read(reinterpret_cast<char*>(vector.data()), embedding_dim * sizeof(float));

            word_vectors[word] = vector;
        }

        initialized = true;
        return true;
    }
};

EmbeddingModel::EmbeddingModel() 
    : impl_(std::make_unique<Impl>()) {
}

EmbeddingModel::~EmbeddingModel() = default;

bool EmbeddingModel::initialize(const std::string& model_path, size_t embedding_dim) {
    impl_->embedding_dim = embedding_dim;

    if (model_path.empty()) {
        return true;
    }

    if (model_path.find(".bin") != std::string::npos) {
        return impl_->loadFromBinaryFile(model_path);
    } else {
        return impl_->loadFromTextFile(model_path);
    }
}

bool EmbeddingModel::loadWordVectors(const std::string& word_vec_path) {
    return impl_->loadFromTextFile(word_vec_path);
}

void EmbeddingModel::addWordVector(const std::string& word, const std::vector<float>& vector) {
    if (vector.size() == impl_->embedding_dim) {
        impl_->word_vectors[word] = vector;
    }
}

size_t EmbeddingModel::getEmbeddingDim() const {
    return impl_->embedding_dim;
}

size_t EmbeddingModel::getVocabSize() const {
    return impl_->word_vectors.size();
}

bool EmbeddingModel::hasWord(const std::string& word) const {
    return impl_->word_vectors.find(word) != impl_->word_vectors.end();
}

std::array<float, DEFAULT_EMBEDDING_DIM> EmbeddingModel::computeEmbedding(const std::string& text) {
    std::array<float, DEFAULT_EMBEDDING_DIM> result{};
    result.fill(0.0f);
    
    if (!impl_ || impl_->word_vectors.empty()) {
        return result;
    }
    
    // 简单实现：使用输入文本作为键查找向量
    auto it = impl_->word_vectors.find(text);
    if (it != impl_->word_vectors.end()) {
        for (size_t i = 0; i < DEFAULT_EMBEDDING_DIM && i < it->second.size(); ++i) {
            result[i] = it->second[i];
        }
    }
    
    return result;
}

std::array<float, DEFAULT_EMBEDDING_DIM> EmbeddingModel::computeEmbeddingFromTokens(const std::vector<std::string>& tokens) {
    std::array<float, DEFAULT_EMBEDDING_DIM> result{};
    result.fill(0.0f);
    
    if (tokens.empty() || !impl_ || impl_->word_vectors.empty()) {
        return result;
    }
    
    size_t count = 0;
    for (const auto& token : tokens) {
        auto it = impl_->word_vectors.find(token);
        if (it != impl_->word_vectors.end()) {
            for (size_t i = 0; i < DEFAULT_EMBEDDING_DIM && i < it->second.size(); ++i) {
                result[i] += it->second[i];
            }
            count++;
        }
    }
    
    if (count > 0) {
        for (size_t i = 0; i < DEFAULT_EMBEDDING_DIM; ++i) {
            result[i] /= static_cast<float>(count);
        }
    }
    
    return result;
}

}
