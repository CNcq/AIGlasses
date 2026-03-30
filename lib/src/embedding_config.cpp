#include "embedding_config.h"
#include "json_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include <algorithm>

namespace ai_glasses {

// EmbeddingConfig 实现

bool EmbeddingConfig::loadWordVectors(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }
    
    std::string line;
    
    while (std::getline(file, line)) {
        // 跳过空行
        if (line.empty()) continue;
        
        // 只存储词语（向量会自动生成）
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    
    return true;
}

bool EmbeddingConfig::saveWordVectors(const std::string& file_path) const {
    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }
    
    // 写入头信息
    file << words.size() << " " << embedding_dim << std::endl;
    
    // 生成随机向量并写入
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0f, 1.0f);
    
    for (const auto& word : words) {
        file << word;
        for (size_t i = 0; i < embedding_dim; ++i) {
            file << " " << dis(gen);
        }
        file << std::endl;
    }
    
    return true;
}

bool EmbeddingConfig::loadFromJson(const std::string& json_string) {
    JsonValue root;
    try {
        root = JsonParser::parse(json_string);
    } catch (...) {
        std::cerr << "Failed to parse JSON" << std::endl;
        return false;
    }
    
    if (!root.isObject()) {
        std::cerr << "JSON root must be an object" << std::endl;
        return false;
    }
    
    // 解析 embedding_dim
    if (root.has("embedding_dim")) {
        embedding_dim = static_cast<size_t>(root["embedding_dim"].asNumber());
    }
    
    // 解析 words（只存储词语）
    if (root.has("words")) {
        JsonValue words_json = root["words"];
        if (words_json.isArray()) {
            for (size_t i = 0; i < words_json.asArray().size(); ++i) {
                const auto& item = words_json.asArray()[i];
                if (item.isString()) {
                    words.push_back(item.asString());
                }
            }
        }
    }
    
    // 兼容旧格式：也解析 word_vectors
    if (root.has("word_vectors")) {
        JsonValue word_vectors_json = root["word_vectors"];
        if (word_vectors_json.isArray()) {
            for (size_t i = 0; i < word_vectors_json.asArray().size(); ++i) {
                const auto& item = word_vectors_json.asArray()[i];
                if (item.isObject()) {
                    if (item.has("word")) {
                        words.push_back(item["word"].asString());
                    }
                }
            }
        }
    }
    
    return true;
}

std::string EmbeddingConfig::toJson() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"embedding_dim\": " << embedding_dim << ",\n";
    oss << "  \"words\": [\n";
    
    for (size_t i = 0; i < words.size(); ++i) {
        oss << "    \"" << words[i] << "\"";
        if (i < words.size() - 1) oss << ",";
        oss << "\n";
    }
    
    oss << "  ]\n";
    oss << "}\n";
    
    return oss.str();
}

void EmbeddingConfig::generateRandomVectors() {
    // 这个函数在 EmbeddingMatcher 中调用，用于生成随机向量
    // 这里只保留声明，实际实现由 EmbeddingMatcher 完成
}

const std::vector<float>& EmbeddingConfig::getVector(const std::string& word) const {
    // 这个函数在 EmbeddingMatcher 中调用，用于获取词向量
    // 这里只保留声明，实际实现由 EmbeddingMatcher 完成
    static std::vector<float> empty_vector;
    return empty_vector;
}

bool EmbeddingConfig::hasWord(const std::string& word) const {
    return std::find(words.begin(), words.end(), word) != words.end();
}

// EmbeddingConfigManager 实现

EmbeddingConfigManager::EmbeddingConfigManager() {}

EmbeddingConfigManager::~EmbeddingConfigManager() {}

bool EmbeddingConfigManager::loadFromFile(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open config file: " << config_path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    return loadFromJson(buffer.str());
}

bool EmbeddingConfigManager::loadFromJson(const std::string& json_string) {
    return config_.loadFromJson(json_string);
}

const EmbeddingConfig& EmbeddingConfigManager::getConfig() const {
    return config_;
}

bool EmbeddingConfigManager::addWord(const std::string& word) {
    if (!config_.hasWord(word)) {
        config_.words.push_back(word);
        return true;
    }
    return false;
}

bool EmbeddingConfigManager::addWords(const std::vector<std::string>& words) {
    for (const auto& word : words) {
        addWord(word);
    }
    return !config_.words.empty();
}

void EmbeddingConfigManager::clear() {
    config_.words.clear();
}

}
