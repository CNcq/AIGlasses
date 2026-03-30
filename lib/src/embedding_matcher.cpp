#include "embedding_matcher.h"
#include "jieba_segmenter.h"
#include "embedding_model.h"
#include "json_parser.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <numeric>

namespace ai_glasses {

struct EmbeddingMatcher::Impl {
    std::unique_ptr<JiebaSegmenter> segmenter;
    std::unique_ptr<EmbeddingModel> embedding_model;
    std::vector<EnumItem> enum_items;
    std::unordered_map<std::string, size_t> enum_item_map;
    std::unordered_map<std::string, std::vector<std::string>> category_map;
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

bool EmbeddingMatcher::initializeFromConfig(const std::string& config_path) {
    return loadDefectConfig(config_path);
}

bool EmbeddingMatcher::loadDefectConfig(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    return loadDefectConfigFromString(content);
}

bool EmbeddingMatcher::loadDefectConfigFromString(const std::string& json_string) {
    try {
        JsonValue root = JsonParser::parse(json_string);
        
        if (!root.isObject() || !root.has("defects") || !root["defects"].isArray()) {
            return false;
        }
        
        const auto& defects = root["defects"].asArray();
        
        for (size_t i = 0; i < defects.size(); ++i) {
            if (!defects[i].isObject()) continue;
            
            const auto& defect = defects[i];
            
            EnumItem item;
            item.id = defect.has("id") ? defect["id"].asString() : "";
            item.text = defect.has("name") ? defect["name"].asString() : "";
            item.description = defect.has("description") ? defect["description"].asString() : "";
            item.threshold = defect.has("threshold") ? static_cast<float>(defect["threshold"].asNumber()) : 0.5f;
            item.enabled = defect.has("enabled") ? defect["enabled"].asBool() : true;
            item.is_precomputed = false;
            item.embedding.fill(0.0f);
            
            if (defect.has("keywords") && defect["keywords"].isArray()) {
                const auto& keywords = defect["keywords"].asArray();
                for (size_t j = 0; j < keywords.size(); ++j) {
                    if (keywords[j].isString()) {
                        item.keywords.push_back(keywords[j].asString());
                    }
                }
            }
            
            if (defect.has("categories") && defect["categories"].isArray()) {
                const auto& categories = defect["categories"].asArray();
                for (size_t j = 0; j < categories.size(); ++j) {
                    if (categories[j].isString()) {
                        std::string category = categories[j].asString();
                        impl_->category_map[category].push_back(item.id);
                    }
                }
            }
            
            addEnumItem(item);
        }
        
        return true;
    } catch (...) {
        return false;
    }
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

std::vector<MatchResult> EmbeddingMatcher::findAllMatches(const std::string& input_text, int top_k) {
    std::vector<MatchResult> results;
    
    if (impl_->enum_items.empty()) {
        return results;
    }
    
    auto input_embedding = computeEmbedding(input_text);
    
    for (size_t i = 0; i < impl_->enum_items.size(); ++i) {
        const auto& enum_item = impl_->enum_items[i];
        if (!enum_item.enabled || !enum_item.is_precomputed) {
            continue;
        }
        
        float sim = cosineSimilarity(input_embedding, enum_item.embedding);
        
        // 只添加有实际匹配的结果
        if (sim > 0.0f) {
            MatchResult result;
            result.input_text = input_text;
            result.matched_id = enum_item.id;
            result.matched_text = enum_item.text;
            result.similarity = sim;
            result.is_match = sim >= enum_item.threshold;
            results.push_back(result);
        }
    }
    
    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.similarity > b.similarity;
    });
    
    for (size_t i = 0; i < results.size(); ++i) {
        results[i].rank = i + 1;
    }
    
    if (results.size() > static_cast<size_t>(top_k)) {
        results.resize(top_k);
    }
    
    return results;
}

std::vector<MatchResult> EmbeddingMatcher::findByCategory(const std::string& category,
                                                           const std::string& input_text) {
    std::vector<MatchResult> results;
    
    auto it = impl_->category_map.find(category);
    if (it == impl_->category_map.end()) {
        return results;
    }
    
    const auto& item_ids = it->second;
    auto input_embedding = computeEmbedding(input_text);
    
    for (const auto& item_id : item_ids) {
        auto map_it = impl_->enum_item_map.find(item_id);
        if (map_it == impl_->enum_item_map.end()) {
            continue;
        }
        
        const auto& enum_item = impl_->enum_items[map_it->second];
        if (!enum_item.enabled || !enum_item.is_precomputed) {
            continue;
        }
        
        float sim = cosineSimilarity(input_embedding, enum_item.embedding);
        
        MatchResult result;
        result.input_text = input_text;
        result.matched_id = enum_item.id;
        result.matched_text = enum_item.text;
        result.similarity = sim;
        result.is_match = sim >= enum_item.threshold;
        results.push_back(result);
    }
    
    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.similarity > b.similarity;
    });
    
    for (size_t i = 0; i < results.size(); ++i) {
        results[i].rank = i + 1;
    }
    
    return results;
}

EmbeddingVector EmbeddingMatcher::computeEmbedding(const std::string& text) {
    auto tokens = segmentText(text);
    return computeTextEmbedding(tokens);
}

bool EmbeddingMatcher::loadPrecomputedEmbeddings(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    size_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
    
    for (size_t i = 0; i < count; ++i) {
        size_t id_len;
        file.read(reinterpret_cast<char*>(&id_len), sizeof(size_t));
        
        std::string id(id_len, '\0');
        file.read(&id[0], id_len);
        
        EmbeddingVector embedding;
        file.read(reinterpret_cast<char*>(embedding.data()), EMBEDDING_DIM * sizeof(float));
        
        auto it = impl_->enum_item_map.find(id);
        if (it != impl_->enum_item_map.end()) {
            impl_->enum_items[it->second].embedding = embedding;
            impl_->enum_items[it->second].is_precomputed = true;
        }
    }
    
    return true;
}

bool EmbeddingMatcher::savePrecomputedEmbeddings(const std::string& file_path) {
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    size_t count = impl_->enum_items.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    
    for (const auto& item : impl_->enum_items) {
        size_t id_len = item.id.size();
        file.write(reinterpret_cast<const char*>(&id_len), sizeof(size_t));
        file.write(item.id.c_str(), id_len);
        file.write(reinterpret_cast<const char*>(item.embedding.data()), EMBEDDING_DIM * sizeof(float));
    }
    
    return true;
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

size_t EmbeddingMatcher::getEnabledEnumItemCount() const {
    size_t count = 0;
    for (const auto& item : impl_->enum_items) {
        if (item.enabled) {
            count++;
        }
    }
    return count;
}

void EmbeddingMatcher::enableEnumItem(const std::string& id) {
    auto it = impl_->enum_item_map.find(id);
    if (it != impl_->enum_item_map.end()) {
        impl_->enum_items[it->second].enabled = true;
        if (!impl_->enum_items[it->second].is_precomputed) {
            computeEnumItemEmbedding(impl_->enum_items[it->second]);
        }
    }
}

void EmbeddingMatcher::disableEnumItem(const std::string& id) {
    auto it = impl_->enum_item_map.find(id);
    if (it != impl_->enum_item_map.end()) {
        impl_->enum_items[it->second].enabled = false;
    }
}

bool EmbeddingMatcher::isEnumItemEnabled(const std::string& id) const {
    auto it = impl_->enum_item_map.find(id);
    if (it != impl_->enum_item_map.end()) {
        return impl_->enum_items[it->second].enabled;
    }
    return false;
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

bool EmbeddingMatcher::precomputeAndSaveEnumVectors(const std::string& cache_path) {
    std::ofstream file(cache_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    size_t count = impl_->enum_items.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    
    for (const auto& item : impl_->enum_items) {
        size_t id_len = item.id.size();
        file.write(reinterpret_cast<const char*>(&id_len), sizeof(size_t));
        file.write(item.id.c_str(), id_len);
        
        size_t text_len = item.text.size();
        file.write(reinterpret_cast<const char*>(&text_len), sizeof(size_t));
        file.write(item.text.c_str(), text_len);
        
        EmbeddingVector embedding = item.embedding;
        if (!item.is_precomputed) {
            embedding = computeTextEmbedding(segmentText(item.text));
            for (const auto& kw : item.keywords) {
                auto kw_emb = computeTextEmbedding(segmentText(kw));
                for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
                    embedding[i] += kw_emb[i];
                }
            }
            float norm = 0.0f;
            for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
                norm += embedding[i] * embedding[i];
            }
            if (norm > 0) {
                norm = std::sqrt(norm);
                for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
                    embedding[i] /= norm;
                }
            }
        }
        
        file.write(reinterpret_cast<const char*>(embedding.data()), EMBEDDING_DIM * sizeof(float));
        
        bool enabled = item.enabled;
        file.write(reinterpret_cast<const char*>(&enabled), sizeof(bool));
        
        float threshold = item.threshold;
        file.write(reinterpret_cast<const char*>(&threshold), sizeof(float));
    }
    
    return true;
}

bool EmbeddingMatcher::loadPrecomputedEnumVectors(const std::string& cache_path) {
    std::ifstream file(cache_path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    size_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
    
    for (size_t i = 0; i < count; ++i) {
        size_t id_len;
        file.read(reinterpret_cast<char*>(&id_len), sizeof(size_t));
        
        std::string id(id_len, '\0');
        file.read(&id[0], id_len);
        
        size_t text_len;
        file.read(reinterpret_cast<char*>(&text_len), sizeof(size_t));
        
        std::string text(text_len, '\0');
        file.read(&text[0], text_len);
        
        EmbeddingVector embedding;
        file.read(reinterpret_cast<char*>(embedding.data()), EMBEDDING_DIM * sizeof(float));
        
        bool enabled;
        file.read(reinterpret_cast<char*>(&enabled), sizeof(bool));
        
        float threshold;
        file.read(reinterpret_cast<char*>(&threshold), sizeof(float));
        
        auto it = impl_->enum_item_map.find(id);
        if (it != impl_->enum_item_map.end()) {
            impl_->enum_items[it->second].embedding = embedding;
            impl_->enum_items[it->second].is_precomputed = true;
            impl_->enum_items[it->second].enabled = enabled;
            impl_->enum_items[it->second].threshold = threshold;
        } else {
            EnumItem item;
            item.id = id;
            item.text = text;
            item.embedding = embedding;
            item.enabled = enabled;
            item.threshold = threshold;
            item.is_precomputed = true;
            item.description = "";
            
            size_t index = impl_->enum_items.size();
            impl_->enum_items.push_back(item);
            impl_->enum_item_map[id] = index;
        }
    }
    
    return true;
}

bool EmbeddingMatcher::addWordVector(const std::string& word, const std::vector<float>& vector) {
    if (vector.size() != EMBEDDING_DIM) {
        return false;
    }
    
    EmbeddingVector emb;
    for (size_t i = 0; i < EMBEDDING_DIM; ++i) {
        emb[i] = vector[i];
    }
    
    return impl_->embedding_model->addWordVector(word, emb);
}

}
