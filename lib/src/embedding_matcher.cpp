#include "embedding_matcher.h"
#include "jieba_segmenter.h"
#include "embedding_model.h"
#include "json_parser.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace ai_glasses {

struct EmbeddingMatcher::Impl {
    std::unique_ptr<JiebaSegmenter> segmenter;
    std::unique_ptr<EmbeddingModel> embedding_model;
    std::vector<EnumItem> enum_items;
    std::unordered_map<std::string, size_t> enum_item_map;
    float similarity_threshold = 0.5f;
    bool initialized = false;
};

EmbeddingMatcher::EmbeddingMatcher() 
    : impl_(std::make_unique<Impl>()) {
    impl_->segmenter = std::make_unique<JiebaSegmenter>();
    impl_->embedding_model = std::make_unique<EmbeddingModel>();
}

EmbeddingMatcher::~EmbeddingMatcher() = default;

bool EmbeddingMatcher::initialize(const std::string& jieba_dict_path,
                                   const std::string& embedding_model_path) {
    bool success = true;
    
    if (!jieba_dict_path.empty()) {
        success &= impl_->segmenter->initialize(jieba_dict_path);
    } else {
        success &= impl_->segmenter->initialize("");
    }
    
    if (!embedding_model_path.empty()) {
        success &= impl_->embedding_model->initialize(embedding_model_path, EMBEDDING_DIM);
    } else {
        success &= impl_->embedding_model->initialize("", EMBEDDING_DIM);
    }
    
    impl_->initialized = success;
    return success;
}

void EmbeddingMatcher::addEnumItem(const EnumItem& item) {
    size_t index = impl_->enum_items.size();
    impl_->enum_items.push_back(item);
    impl_->enum_item_map[item.id] = index;
    
    if (!item.is_precomputed && item.enabled) {
        computeEnumItemEmbedding(impl_->enum_items.back());
    }
}

void EmbeddingMatcher::addEnumItem(const std::string& id, const std::string& text,
                                    const std::vector<std::string>& keywords) {
    EnumItem item;
    item.id = id;
    item.text = text;
    item.description = "";
    item.keywords = keywords;
    item.threshold = impl_->similarity_threshold;
    item.enabled = true;
    item.is_precomputed = false;
    item.embedding.fill(0.0f);
    
    addEnumItem(item);
}

void EmbeddingMatcher::addEnumItem(const std::string& id, const std::string& text,
                                    const std::string& description,
                                    const std::vector<std::string>& keywords,
                                    float threshold) {
    EnumItem item;
    item.id = id;
    item.text = text;
    item.description = description;
    item.keywords = keywords;
    item.threshold = threshold;
    item.enabled = true;
    item.is_precomputed = false;
    item.embedding.fill(0.0f);
    
    addEnumItem(item);
}

void EmbeddingMatcher::addDefectsFromList(const std::vector<std::string>& defect_list) {
    for (size_t i = 0; i < defect_list.size(); ++i) {
        std::string id = "defect_" + std::to_string(i);
        std::string text = defect_list[i];
        std::vector<std::string> keywords = {text};
        
        addEnumItem(id, text, keywords);
    }
}

MatchResult EmbeddingMatcher::findBestMatch(const std::string& input_text) {
    MatchResult result;
    result.input_text = input_text;
    result.similarity = 0.0f;
    result.rank = 1;
    result.is_match = false;
    
    if (impl_->enum_items.empty()) {
        return result;
    }
    
    auto input_embedding = computeEmbedding(input_text);
    
    for (size_t i = 0; i < impl_->enum_items.size(); ++i) {
        const auto& enum_item = impl_->enum_items[i];
        if (!enum_item.enabled || !enum_item.is_precomputed) {
            continue;
        }
        
        float sim = cosineSimilarity(input_embedding, enum_item.embedding);
        
        if (sim > result.similarity) {
            result.similarity = sim;
            result.matched_id = enum_item.id;
            result.matched_text = enum_item.text;
            result.is_match = sim >= enum_item.threshold;
        }
    }
    
    return result;
}

EmbeddingVector EmbeddingMatcher::computeEmbedding(const std::string& text) {
    auto tokens = segmentText(text);
    return computeTextEmbedding(tokens);
}

void EmbeddingMatcher::setSimilarityThreshold(float threshold) {
    impl_->similarity_threshold = std::max(0.0f, std::min(1.0f, threshold));
}

float EmbeddingMatcher::getSimilarityThreshold() const {
    return impl_->similarity_threshold;
}

size_t EmbeddingMatcher::getEnumItemCount() const {
    return impl_->enum_items.size();
}

float EmbeddingMatcher::cosineSimilarity(const EmbeddingVector& v1, const EmbeddingVector& v2) {
    float dot_product = 0.0f;
    float norm1 = 0.0f, norm2 = 0.0f;
    
    for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
        dot_product += v1[i] * v2[i];
        norm1 += v1[i] * v1[i];
        norm2 += v2[i] * v2[i];
    }
    
    if (norm1 < 1e-8f || norm2 < 1e-8f) {
        return 0.0f;
    }
    
    return dot_product / (std::sqrt(norm1) * std::sqrt(norm2));
}

std::vector<std::string> EmbeddingMatcher::segmentText(const std::string& text) {
    return impl_->segmenter->segment(text);
}

EmbeddingVector EmbeddingMatcher::computeTextEmbedding(const std::vector<std::string>& tokens) {
    EmbeddingVector embedding{};
    embedding.fill(0.0f);
    
    if (tokens.empty()) {
        return embedding;
    }
    
    size_t count = 0;
    for (const auto& token : tokens) {
        auto word_embedding = impl_->embedding_model->computeEmbedding(token);
        for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
            embedding[i] += word_embedding[i];
        }
        count++;
    }
    
    if (count > 0) {
        for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
            embedding[i] /= static_cast<float>(count);
        }
    }
    
    return embedding;
}

void EmbeddingMatcher::computeEnumItemEmbedding(EnumItem& item) {
    std::vector<std::string> all_tokens;
    
    auto text_tokens = segmentText(item.text);
    all_tokens.insert(all_tokens.end(), text_tokens.begin(), text_tokens.end());
    
    for (const auto& kw : item.keywords) {
        auto kw_tokens = segmentText(kw);
        all_tokens.insert(all_tokens.end(), kw_tokens.begin(), kw_tokens.end());
    }
    
    if (!item.description.empty()) {
        auto desc_tokens = segmentText(item.description);
        all_tokens.insert(all_tokens.end(), desc_tokens.begin(), desc_tokens.end());
    }
    
    item.embedding = computeTextEmbedding(all_tokens);
    item.is_precomputed = true;
}

bool EmbeddingMatcher::addWordVector(const std::string& word, const std::vector<float>& vector) {
    if (vector.size() != EMBEDDING_DIM) {
        return false;
    }
    
    EmbeddingVector emb;
    for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
        emb[i] = vector[i];
    }
    
    impl_->embedding_model->addWordVector(word, emb);
    return true;
}

}
