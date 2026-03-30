#ifndef JIEBA_SEGMENTER_H
#define JIEBA_SEGMENTER_H

#include <string>
#include <vector>
#include <memory>

namespace ai_glasses {

class JiebaSegmenter {
public:
    JiebaSegmenter();
    ~JiebaSegmenter();

    bool initialize(const std::string& dict_path);
    
    std::vector<std::string> segment(const std::string& text);
    
    std::string segmentToString(const std::string& text, const std::string& delimiter = " ");
    
    bool isInitialized() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}

#endif
