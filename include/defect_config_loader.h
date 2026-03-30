#ifndef DEFECT_CONFIG_LOADER_H
#define DEFECT_CONFIG_LOADER_H

#include "embedding_matcher.h"
#include "json_parser.h"
#include <string>
#include <vector>

namespace ai_glasses {

struct DefectConfig {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> keywords;
    std::vector<std::string> categories;
    float threshold;
    bool enabled;
};

class DefectConfigLoader {
public:
    DefectConfigLoader();
    ~DefectConfigLoader();

    bool loadFromFile(const std::string& file_path);
    
    bool loadFromString(const std::string& json_string);
    
    std::vector<DefectConfig> getConfigs() const;
    
    bool applyToMatcher(EmbeddingMatcher& matcher) const;
    
    bool applyToMatcher(EmbeddingMatcher& matcher, 
                       const std::vector<std::string>& enabled_ids) const;
    
    size_t getConfigCount() const;
    
    size_t getEnabledConfigCount() const;

private:
    std::vector<DefectConfig> configs_;
    
    DefectConfig parseDefectConfig(const JsonValue& json) const;
};

}

#endif
