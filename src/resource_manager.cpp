#include "resource_manager.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>

namespace ai_glasses {

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define FILE_EXISTS(path) (_access(path, 0) == 0)
#else
#include <sys/stat.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define FILE_EXISTS(path) (access(path, F_OK) == 0)
#endif

ResourceManager::ResourceManager() : initialized_(false) {
    embedding_model_ = std::make_shared<EmbeddingModel>();
    jieba_segmenter_ = std::make_shared<JiebaSegmenter>();
}

ResourceManager::~ResourceManager() = default;

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

bool ResourceManager::initialize(const ResourceConfig& config) {
    config_ = config;
    
    if (!config_.cache_dir.empty()) {
        createDirectory(config_.cache_dir);
    }
    
    if (!config_.embedding_model_path.empty() && 
        FILE_EXISTS(config_.embedding_model_path.c_str())) {
        if (!embedding_model_->initialize(config_.embedding_model_path, config_.embedding_dim)) {
            return false;
        }
    } else {
        embedding_model_->initialize("", config_.embedding_dim);
    }
    
    if (!config_.jieba_dict_path.empty() && 
        FILE_EXISTS(config_.jieba_dict_path.c_str())) {
        if (!jieba_segmenter_->initialize(config_.jieba_dict_path)) {
            return false;
        }
    } else {
        jieba_segmenter_->initialize("");
    }
    
    initialized_ = true;
    return true;
}

bool ResourceManager::initialize(const std::string& base_path) {
    ResourceConfig config;
    
    config.embedding_model_path = base_path + "/models/embedding.txt";
    config.jieba_dict_path = base_path + "/dict/jieba.dict";
    config.defect_config_path = base_path + "/config/defects.json";
    config.cache_dir = base_path + "/cache";
    config.embedding_dim = 128;
    config.default_threshold = 0.5f;
    
    return initialize(config);
}

bool ResourceManager::isInitialized() const {
    return initialized_;
}

std::shared_ptr<EmbeddingModel> ResourceManager::getEmbeddingModel() const {
    return embedding_model_;
}

std::shared_ptr<JiebaSegmenter> ResourceManager::getJiebaSegmenter() const {
    return jieba_segmenter_;
}

const ResourceConfig& ResourceManager::getConfig() const {
    return config_;
}

std::string ResourceManager::getEmbeddingModelPath() const {
    return config_.embedding_model_path;
}

std::string ResourceManager::getJiebaDictPath() const {
    return config_.jieba_dict_path;
}

std::string ResourceManager::getDefectConfigPath() const {
    return config_.defect_config_path;
}

std::string ResourceManager::getCacheDir() const {
    return config_.cache_dir;
}

bool ResourceManager::createDirectory(const std::string& path) {
    return MKDIR(path.c_str()) == 0 || FILE_EXISTS(path.c_str());
}

bool ResourceManager::fileExists(const std::string& path) {
    return FILE_EXISTS(path.c_str());
}

bool ResourceManager::copyFile(const std::string& src, const std::string& dst) {
    std::ifstream src_file(src, std::ios::binary);
    if (!src_file.is_open()) {
        return false;
    }
    
    std::ofstream dst_file(dst, std::ios::binary);
    if (!dst_file.is_open()) {
        return false;
    }
    
    dst_file << src_file.rdbuf();
    return true;
}

bool ResourceManager::copyModelToCache(const std::string& source_path, 
                                        const std::string& cache_name) {
    if (!fileExists(source_path)) {
        return false;
    }
    
    std::string cache_path = config_.cache_dir + "/" + cache_name;
    createDirectory(config_.cache_dir);
    
    return copyFile(source_path, cache_path);
}

bool ResourceManager::loadEmbeddingModel(const std::string& model_path) {
    if (!fileExists(model_path)) {
        return false;
    }
    
    return embedding_model_->initialize(model_path, config_.embedding_dim);
}

bool ResourceManager::loadJiebaDictionary(const std::string& dict_path) {
    if (!fileExists(dict_path)) {
        return false;
    }
    
    return jieba_segmenter_->initialize(dict_path);
}

bool ResourceManager::exportDefaultConfig(const std::string& output_path) {
    std::ofstream file(output_path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"version\": \"1.0\",\n";
    file << "  \"description\": \"缺陷枚举配置\",\n";
    file << "  \"defects\": [\n";
    
    std::vector<std::pair<std::string, std::string>> default_defects = {
        {"DEFECT_001", "墙皮脱落"},
        {"DEFECT_002", "裂缝"},
        {"DEFECT_003", "空鼓"},
        {"DEFECT_004", "渗水"},
        {"DEFECT_005", "发霉"},
        {"DEFECT_006", "脱落"},
        {"DEFECT_007", "钢筋外露"},
        {"DEFECT_008", "混凝土破损"}
    };
    
    for (size_t i = 0; i < default_defects.size(); ++i) {
        file << "    {\n";
        file << "      \"id\": \"" << default_defects[i].first << "\",\n";
        file << "      \"name\": \"" << default_defects[i].second << "\",\n";
        file << "      \"description\": \"" << default_defects[i].second << "缺陷\",\n";
        file << "      \"keywords\": [],\n";
        file << "      \"threshold\": 0.5,\n";
        file << "      \"enabled\": true\n";
        file << "    }";
        
        if (i < default_defects.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    return true;
}

}
