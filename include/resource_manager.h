#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "embedding_model.h"
#include "jieba_segmenter.h"
#include <string>
#include <memory>

namespace ai_glasses {

struct ResourceConfig {
    std::string embedding_model_path;
    std::string jieba_dict_path;
    std::string defect_config_path;
    std::string cache_dir;
    size_t embedding_dim;
    float default_threshold;
};

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    static ResourceManager& getInstance();

    bool initialize(const ResourceConfig& config);
    
    bool initialize(const std::string& base_path);
    
    bool isInitialized() const;

    std::shared_ptr<EmbeddingModel> getEmbeddingModel() const;
    
    std::shared_ptr<JiebaSegmenter> getJiebaSegmenter() const;
    
    const ResourceConfig& getConfig() const;
    
    std::string getEmbeddingModelPath() const;
    
    std::string getJiebaDictPath() const;
    
    std::string getDefectConfigPath() const;
    
    std::string getCacheDir() const;

    bool copyModelToCache(const std::string& source_path, 
                         const std::string& cache_name);
    
    bool loadEmbeddingModel(const std::string& model_path);
    
    bool loadJiebaDictionary(const std::string& dict_path);
    
    bool exportDefaultConfig(const std::string& output_path);

private:
    ResourceConfig config_;
    std::shared_ptr<EmbeddingModel> embedding_model_;
    std::shared_ptr<JiebaSegmenter> jieba_segmenter_;
    bool initialized_;
    
    bool createDirectory(const std::string& path);
    bool fileExists(const std::string& path);
    bool copyFile(const std::string& src, const std::string& dst);
};

}

#endif
