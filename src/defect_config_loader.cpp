#include "defect_config_loader.h"
#include <fstream>
#include <sstream>

namespace ai_glasses {

DefectConfigLoader::DefectConfigLoader() = default;
DefectConfigLoader::~DefectConfigLoader() = default;

bool DefectConfigLoader::loadFromFile(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    return loadFromString(content);
}

bool DefectConfigLoader::loadFromString(const std::string& json_string) {
    try {
        JsonValue root = JsonParser::parse(json_string);
        
        if (!root.isObject()) {
            return false;
        }
        
        configs_.clear();
        
        if (root.has("defects") && root["defects"].isArray()) {
            const auto& defects = root["defects"].asArray();
            for (size_t i = 0; i < defects.size(); ++i) {
                if (defects[i].isObject()) {
                    configs_.push_back(parseDefectConfig(defects[i]));
                }
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

DefectConfig DefectConfigLoader::parseDefectConfig(const JsonValue& json) const {
    DefectConfig config;
    
    config.id = json.has("id") ? json["id"].asString() : "";
    config.name = json.has("name") ? json["name"].asString() : "";
    config.description = json.has("description") ? json["description"].asString() : "";
    config.threshold = json.has("threshold") ? static_cast<float>(json["threshold"].asNumber()) : 0.5f;
    config.enabled = json.has("enabled") ? json["enabled"].asBool() : true;
    
    if (json.has("keywords") && json["keywords"].isArray()) {
        const auto& keywords = json["keywords"].asArray();
        for (size_t i = 0; i < keywords.size(); ++i) {
            if (keywords[i].isString()) {
                config.keywords.push_back(keywords[i].asString());
            }
        }
    }
    
    if (json.has("categories") && json["categories"].isArray()) {
        const auto& categories = json["categories"].asArray();
        for (size_t i = 0; i < categories.size(); ++i) {
            if (categories[i].isString()) {
                config.categories.push_back(categories[i].asString());
            }
        }
    }
    
    return config;
}

std::vector<DefectConfig> DefectConfigLoader::getConfigs() const {
    return configs_;
}

bool DefectConfigLoader::applyToMatcher(EmbeddingMatcher& matcher) const {
    for (const auto& config : configs_) {
        if (!config.enabled) {
            continue;
        }
        
        matcher.addEnumItem(config.id, config.name, config.keywords);
    }
    
    return true;
}

bool DefectConfigLoader::applyToMatcher(EmbeddingMatcher& matcher, 
                                        const std::string& enabled_ids) const {
    std::vector<std::string> enabled_list;
    std::string current;
    
    for (char c : enabled_ids) {
        if (c == ',' || c == ';') {
            if (!current.empty()) {
                enabled_list.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        enabled_list.push_back(current);
    }
    
    return applyToMatcher(matcher, enabled_list);
}

bool DefectConfigLoader::applyToMatcher(EmbeddingMatcher& matcher, 
                                        const std::vector<std::string>& enabled_ids) const {
    std::vector<bool> enabled(configs_.size(), false);
    
    for (const auto& id : enabled_ids) {
        for (size_t i = 0; i < configs_.size(); ++i) {
            if (configs_[i].id == id) {
                enabled[i] = true;
                break;
            }
        }
    }
    
    for (size_t i = 0; i < configs_.size(); ++i) {
        if (enabled[i]) {
            matcher.addEnumItem(configs_[i].id, configs_[i].name, configs_[i].keywords);
        }
    }
    
    return true;
}

size_t DefectConfigLoader::getConfigCount() const {
    return configs_.size();
}

size_t DefectConfigLoader::getEnabledConfigCount() const {
    size_t count = 0;
    for (const auto& config : configs_) {
        if (config.enabled) {
            count++;
        }
    }
    return count;
}

}
