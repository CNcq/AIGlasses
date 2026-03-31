#include "jieba_segmenter.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include "cppjieba/Jieba.hpp"

namespace ai_glasses {

struct JiebaSegmenter::Impl {
    std::string dict_path;
    std::unique_ptr<cppjieba::Jieba> jieba;
    std::unordered_set<std::string> dict_words;
    std::unordered_map<std::string, std::vector<std::string>> trie;
    bool initialized = false;

    bool loadDictionary(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                size_t pos = line.find('\t');
                if (pos != std::string::npos) {
                    std::string word = line.substr(0, pos);
                    dict_words.insert(word);
                } else {
                    dict_words.insert(line);
                }
            }
        }
        return true;
    }

    std::vector<std::string> maxMatch(const std::string& text, size_t max_word_len = 10) {
        std::vector<std::string> result;

        size_t i = 0;
        while (i < text.size()) {
            size_t matched_len = 0;
            std::string matched_word;

            for (size_t len = std::min(max_word_len, text.size() - i); len > 0; --len) {
                std::string word = text.substr(i, len);

                bool is_ascii_word = true;
                for (size_t j = 0; j < len; ++j) {
                    unsigned char c = static_cast<unsigned char>(word[j]);
                    if (c > 127) {
                        is_ascii_word = false;
                        break;
                    }
                }

                if (is_ascii_word) {
                    if (dict_words.find(word) != dict_words.end()) {
                        matched_word = word;
                        matched_len = len;
                        break;
                    }
                } else {
                    std::string sub_word;
                    for (size_t k = 0; k < len && (i + k) < text.size(); ++k) {
                        if (static_cast<unsigned char>(text[i + k]) > 127) {
                            sub_word += text.substr(i + k, 3);
                            if (dict_words.find(sub_word) != dict_words.end() || 
                                sub_word.length() >= 3) {
                                matched_word = sub_word;
                                matched_len = k > 0 ? k : 3;
                            }
                            break;
                        }
                    }
                    if (!matched_word.empty() && matched_len > 0) {
                        break;
                    }
                }
            }

            if (matched_len > 0) {
                result.push_back(matched_word);
                i += matched_len;
            } else {
                unsigned char c = static_cast<unsigned char>(text[i]);
                if (c > 127 && i + 2 < text.size()) {
                    result.push_back(text.substr(i, 3));
                    i += 3;
                } else if (!std::isspace(c)) {
                    result.push_back(text.substr(i, 1));
                    i += 1;
                } else {
                    i += 1;
                }
            }
        }

        return result;
    }

    std::vector<std::string> basicTokenize(const std::string& text) {
        std::vector<std::string> result;
        std::string current;

        for (size_t i = 0; i < text.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(text[i]);

            if (c > 127) {
                if (!current.empty()) {
                    result.push_back(current);
                    current.clear();
                }
                if (i + 2 < text.size()) {
                    result.push_back(text.substr(i, 3));
                    i += 2;
                }
            } else if (std::isspace(c) || std::ispunct(c)) {
                if (!current.empty()) {
                    result.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }

        if (!current.empty()) {
            result.push_back(current);
        }

        return result;
    }
};

JiebaSegmenter::JiebaSegmenter() 
    : impl_(std::make_unique<Impl>()) {
}

JiebaSegmenter::~JiebaSegmenter() = default;

bool JiebaSegmenter::initialize(const std::string& dict_path) {
    impl_->dict_path = dict_path;
    
    if (!dict_path.empty()) {
        impl_->initialized = impl_->loadDictionary(dict_path);
    }

    std::vector<std::string> default_words = {
        "墙皮脱落", "墙面", "墙壁", "裂缝", "空鼓", "脱落", "剥落", "掉皮",
        "发现", "地方", "存在", "缺陷", "问题", "损坏", "破损", "维修",
        "严重", "轻微", "需要", "处理", "修复", "检查"
    };
    
    for (const auto& word : default_words) {
        impl_->dict_words.insert(word);
    }
    
    impl_->initialized = true;
    
    if (dict_path.empty()) {
        impl_->jieba = std::make_unique<cppjieba::Jieba>();
    } else {
        impl_->jieba = std::make_unique<cppjieba::Jieba>(dict_path);
    }
    
    return true;
}

std::vector<std::string> JiebaSegmenter::segment(const std::string& text) {
    if (!impl_->initialized) {
        return impl_->basicTokenize(text);
    }

    if (impl_->jieba) {
        std::vector<std::string> words;
        impl_->jieba->Cut(text, words);
        return words;
    }

    return impl_->basicTokenize(text);
}

std::string JiebaSegmenter::segmentToString(const std::string& text, const std::string& delimiter) {
    auto words = segment(text);
    std::string result;
    
    for (size_t i = 0; i < words.size(); ++i) {
        result += words[i];
        if (i < words.size() - 1) {
            result += delimiter;
        }
    }
    
    return result;
}

bool JiebaSegmenter::isInitialized() const {
    return impl_->initialized;
}

}
