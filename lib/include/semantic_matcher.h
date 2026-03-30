#ifndef SEMANTIC_MATCHER_H
#define SEMANTIC_MATCHER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace ai_glasses {

struct SemanticMatchResult {
    std::string original_text;
    std::string matched_term;
    float similarity;
    bool is_match;
};

class SemanticMatcher {
public:
    SemanticMatcher();
    ~SemanticMatcher();

    void addTermDictionary(const std::string& standard_term, 
                          const std::vector<std::string>& synonyms);
    
    SemanticMatchResult findBestMatch(const std::string& input_text);
    
    std::vector<SemanticMatchResult> findAllMatches(const std::string& input_text, 
                                                      int top_k = 5);
    
    void setSimilarityThreshold(float threshold);
    
    float getSimilarityThreshold() const;

private:
    float calculateEditDistance(const std::string& str1, const std::string& str2);
    float calculateCosineSimilarity(const std::string& str1, const std::string& str2);
    float calculateCombinedSimilarity(const std::string& str1, const std::string& str2);
    
    std::vector<std::string> tokenize(const std::string& text);
    std::string normalizeText(const std::string& text);

    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}

#endif
