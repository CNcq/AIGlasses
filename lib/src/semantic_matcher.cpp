#include "semantic_matcher.h"
#include <algorithm>
#include <cmath>
#include <cctype>
#include <numeric>

namespace ai_glasses {

struct SemanticMatcher::Impl {
    std::unordered_map<std::string, std::vector<std::string>> term_dictionary;
    std::vector<std::string> all_terms;
    float similarity_threshold = 0.6f;
};

SemanticMatcher::SemanticMatcher() 
    : impl_(std::make_unique<Impl>()) {
}

SemanticMatcher::~SemanticMatcher() = default;

void SemanticMatcher::addTermDictionary(const std::string& standard_term,
                                         const std::vector<std::string>& synonyms) {
    impl_->term_dictionary[standard_term] = synonyms;
    impl_->all_terms.push_back(standard_term);
    for (const auto& syn : synonyms) {
        impl_->all_terms.push_back(syn);
    }
}

std::string SemanticMatcher::normalizeText(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    
    for (char c : text) {
        if (std::isalnum(static_cast<unsigned char>(c)) || 
            (static_cast<unsigned char>(c) > 127)) {
            result += std::tolower(static_cast<unsigned char>(c));
        }
    }
    return result;
}

std::vector<std::string> SemanticMatcher::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string token;
    
    for (size_t i = 0; i < text.size(); ) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        
        if (c > 127) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            if (i + 2 < text.size()) {
                tokens.push_back(text.substr(i, 3));
                i += 3;
            } else {
                i++;
            }
        } else if (std::isspace(c)) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
            i++;
        } else {
            token += c;
            i++;
        }
    }
    
    if (!token.empty()) {
        tokens.push_back(token);
    }
    
    return tokens;
}

float SemanticMatcher::calculateEditDistance(const std::string& str1, const std::string& str2) {
    const size_t m = str1.size();
    const size_t n = str2.size();
    
    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));
    
    for (size_t i = 0; i <= m; ++i) {
        dp[i][0] = i;
    }
    for (size_t j = 0; j <= n; ++j) {
        dp[0][j] = j;
    }
    
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (str1[i - 1] == str2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = std::min({
                    dp[i - 1][j] + 1,
                    dp[i][j - 1] + 1,
                    dp[i - 1][j - 1] + 1
                });
            }
        }
    }
    
    size_t max_len = std::max(m, n);
    if (max_len == 0) return 1.0f;
    
    return 1.0f - static_cast<float>(dp[m][n]) / static_cast<float>(max_len);
}

float SemanticMatcher::calculateCosineSimilarity(const std::string& str1, const std::string& str2) {
    auto tokens1 = tokenize(str1);
    auto tokens2 = tokenize(str2);
    
    std::unordered_map<std::string, int> freq1, freq2;
    
    for (const auto& token : tokens1) {
        freq1[token]++;
    }
    for (const auto& token : tokens2) {
        freq2[token]++;
    }
    
    float dot_product = 0.0f;
    float norm1 = 0.0f, norm2 = 0.0f;
    
    for (const auto& pair : freq1) {
        const auto& token = pair.first;
        const auto& count = pair.second;
        norm1 += static_cast<float>(count * count);
        auto it = freq2.find(token);
        if (it != freq2.end()) {
            dot_product += static_cast<float>(count * it->second);
        }
    }
    
    for (const auto& pair : freq2) {
        const auto& token = pair.first;
        const auto& count = pair.second;
        norm2 += static_cast<float>(count * count);
    }
    
    if (norm1 == 0.0f || norm2 == 0.0f) {
        return 0.0f;
    }
    
    return dot_product / (std::sqrt(norm1) * std::sqrt(norm2));
}

float SemanticMatcher::calculateCombinedSimilarity(const std::string& str1, const std::string& str2) {
    std::string norm1 = normalizeText(str1);
    std::string norm2 = normalizeText(str2);
    
    float edit_sim = calculateEditDistance(norm1, norm2);
    float cosine_sim = calculateCosineSimilarity(norm1, norm2);
    
    return 0.6f * edit_sim + 0.4f * cosine_sim;
}

SemanticMatchResult SemanticMatcher::findBestMatch(const std::string& input_text) {
    SemanticMatchResult result;
    result.original_text = input_text;
    result.similarity = 0.0f;
    result.is_match = false;
    result.matched_term = "";
    
    for (const auto& pair : impl_->term_dictionary) {
        const auto& standard_term = pair.first;
        const auto& synonyms = pair.second;
        float best_sim = calculateCombinedSimilarity(input_text, standard_term);
        std::string best_term = standard_term;
        
        for (const auto& syn : synonyms) {
            float sim = calculateCombinedSimilarity(input_text, syn);
            if (sim > best_sim) {
                best_sim = sim;
                best_term = standard_term;
            }
        }
        
        if (best_sim > result.similarity) {
            result.similarity = best_sim;
            result.matched_term = best_term;
        }
    }
    
    result.is_match = result.similarity >= impl_->similarity_threshold;
    return result;
}

std::vector<SemanticMatchResult> SemanticMatcher::findAllMatches(const std::string& input_text, int top_k) {
    std::vector<SemanticMatchResult> results;
    
    for (const auto& pair : impl_->term_dictionary) {
        const auto& standard_term = pair.first;
        const auto& synonyms = pair.second;
        float best_sim = calculateCombinedSimilarity(input_text, standard_term);
        
        for (const auto& syn : synonyms) {
            float sim = calculateCombinedSimilarity(input_text, syn);
            if (sim > best_sim) {
                best_sim = sim;
            }
        }
        
        SemanticMatchResult result;
        result.original_text = input_text;
        result.matched_term = standard_term;
        result.similarity = best_sim;
        result.is_match = best_sim >= impl_->similarity_threshold;
        results.push_back(result);
    }
    
    std::sort(results.begin(), results.end(), 
              [](const SemanticMatchResult& a, const SemanticMatchResult& b) {
                  return a.similarity > b.similarity;
              });
    
    if (results.size() > static_cast<size_t>(top_k)) {
        results.resize(top_k);
    }
    
    return results;
}

void SemanticMatcher::setSimilarityThreshold(float threshold) {
    impl_->similarity_threshold = std::max(0.0f, std::min(1.0f, threshold));
}

float SemanticMatcher::getSimilarityThreshold() const {
    return impl_->similarity_threshold;
}

}
