#include "embedding_matcher.h"
#include "jieba_segmenter.h"
#include "json_parser.h"
#include "semantic_matcher.h"
#include "defect_config_loader.h"
#include <iostream>
#include <cassert>

using namespace ai_glasses;

// 测试 JSON 解析器
void test_json_parser() {
    std::cout << "Testing JSON Parser..." << std::endl;
    
    std::string json = R"({
        "name": "test",
        "value": 123,
        "enabled": true,
        "items": [1, 2, 3]
    })";
    
    JsonValue root = JsonParser::parse(json);
    
    assert(root.isObject());
    assert(root.has("name"));
    assert(root["name"].isString());
    assert(root["name"].asString() == "test");
    assert(root["value"].isNumber());
    assert(root["value"].asNumber() == 123);
    assert(root["enabled"].isBool());
    assert(root["enabled"].asBool() == true);
    assert(root["items"].isArray());
    assert(root["items"].asArray().size() == 3);
    
    std::cout << "✓ JSON Parser test passed!" << std::endl;
}

// 测试结巴分词
void test_jieba_segmenter() {
    std::cout << "Testing Jieba Segmenter..." << std::endl;
    
    JiebaSegmenter segmenter;
    segmenter.initialize("");  // 使用内置词典
    
    std::vector<std::string> tokens = segmenter.segment("墙壁出现脱落");
    
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
    
    assert(!tokens.empty());
    
    std::cout << "✓ Jieba Segmenter test passed!" << std::endl;
}

// 测试 EmbeddingMatcher
void test_embedding_matcher() {
    std::cout << "Testing Embedding Matcher..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");  // 使用空路径测试
    
    // 添加测试数据 - 使用明确的参数类型避免歧义
    std::vector<std::string> keywords1 = {"墙壁", "脱落", "墙皮"};
    matcher.addEnumItem("wall_peel", "墙皮脱落", keywords1);
    
    std::vector<std::string> keywords2 = {"裂缝", "裂痕"};
    matcher.addEnumItem("crack", "裂缝", keywords2);
    
    std::vector<std::string> keywords3 = {"锈蚀", "腐蚀"};
    matcher.addEnumItem("corrosion", "锈蚀", keywords3);
    
    // 测试匹配
    MatchResult result = matcher.findBestMatch("墙壁掉皮");
    
    std::cout << "Input: 墙壁掉皮" << std::endl;
    std::cout << "Matched: " << result.matched_text << std::endl;
    std::cout << "Similarity: " << result.similarity << std::endl;
    
    assert(!result.matched_id.empty());
    
    std::cout << "✓ Embedding Matcher test passed!" << std::endl;
}

// 测试 SemanticMatcher
void test_semantic_matcher() {
    std::cout << "Testing Semantic Matcher..." << std::endl;
    
    SemanticMatcher matcher;
    
    // 添加同义词词典
    std::vector<std::string> wall_synonyms = {"墙壁", "墙面", "墙体", "墙"};
    matcher.addTermDictionary("墙壁", wall_synonyms);
    
    std::vector<std::string> peel_synonyms = {"脱落", "掉落", "剥离", "掉皮"};
    matcher.addTermDictionary("脱落", peel_synonyms);
    
    // 测试匹配
    SemanticMatchResult result = matcher.findBestMatch("墙皮掉落");
    
    std::cout << "Input: 墙皮掉落" << std::endl;
    std::cout << "Matched: " << result.matched_term << std::endl;
    std::cout << "Similarity: " << result.similarity << std::endl;
    
    std::cout << "✓ Semantic Matcher test passed!" << std::endl;
}

// 测试配置加载
void test_config_loader() {
    std::cout << "Testing Config Loader..." << std::endl;
    
    std::string json = R"({
        "defects": [
            {
                "id": "defect_001",
                "name": "墙皮脱落",
                "description": "墙壁表面材料脱落",
                "keywords": ["墙壁", "脱落", "墙皮"],
                "threshold": 0.5,
                "enabled": true
            },
            {
                "id": "defect_002",
                "name": "裂缝",
                "description": "结构出现裂缝",
                "keywords": ["裂缝", "裂痕"],
                "threshold": 0.6,
                "enabled": true
            }
        ]
    })";
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    DefectConfigLoader loader;
    loader.loadFromString(json);
    loader.applyToMatcher(matcher);
    
    assert(loader.getConfigCount() == 2);
    
    std::cout << "✓ Config Loader test passed!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "AI Glasses C++ Library Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        test_json_parser();
        test_jieba_segmenter();
        test_embedding_matcher();
        test_semantic_matcher();
        test_config_loader();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "✓ ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
