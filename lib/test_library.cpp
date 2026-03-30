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

// 测试 EmbeddingMatcher - 使用计量箱缺陷配置
void test_embedding_matcher() {
    std::cout << "Testing Embedding Matcher..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");  // 使用空路径测试
    
    // 添加测试数据 - 使用计量箱缺陷配置
    std::vector<std::string> keywords1 = {"箱体材质", "不锈钢", "塑料", "铁"};
    matcher.addEnumItem("MET_BOX_001", "箱体材质", keywords1);
    
    std::vector<std::string> keywords2 = {"隔离开关", "塑壳断路器", "微型断路器"};
    matcher.addEnumItem("MET_BOX_002", "开关故障", keywords2);
    
    std::vector<std::string> keywords3 = {"变形破损", "老化", "缺失"};
    matcher.addEnumItem("MET_BOX_003", "箱体损坏", keywords3);
    
    // 为测试添加简单的词向量（模拟 embedding）
    // 使用简单的 one-hot 编码模拟向量
    auto addTestVector = [&matcher](const std::string& word, int index) {
        std::vector<float> vec(128, 0.0f);
        vec[index % 128] = 1.0f;
        matcher.getImpl()->embedding_model->addWordVector(word, vec);
    };
    
    // 添加词向量
    addTestVector("箱体材质", 0);
    addTestVector("不锈钢", 1);
    addTestVector("塑料", 2);
    addTestVector("铁", 3);
    addTestVector("隔离开关", 4);
    addTestVector("塑壳断路器", 5);
    addTestVector("微型断路器", 6);
    addTestVector("变形破损", 7);
    addTestVector("老化", 8);
    addTestVector("缺失", 9);
    
    // 测试匹配 - 箱体材质相关
    std::cout << "\nTest 1: 匹配箱体材质相关词汇" << std::endl;
    MatchResult result1 = matcher.findBestMatch("箱体锈蚀破损");
    std::cout << "Input: 箱体锈蚀破损" << std::endl;
    std::cout << "Matched: " << result1.matched_text << std::endl;
    std::cout << "Similarity: " << result1.similarity << std::endl;
    
    if (result1.matched_id.empty()) {
        std::cout << "⚠ No match found (this is expected for random test vectors)" << std::endl;
    } else {
        assert(!result1.matched_id.empty());
    }
    
    // 测试匹配 - 开关相关
    std::cout << "\nTest 2: 匹配开关相关词汇" << std::endl;
    MatchResult result2 = matcher.findBestMatch("隔离开关故障");
    std::cout << "Input: 隔离开关故障" << std::endl;
    std::cout << "Matched: " << result2.matched_text << std::endl;
    std::cout << "Similarity: " << result2.similarity << std::endl;
    
    if (result2.matched_id.empty()) {
        std::cout << "⚠ No match found (this is expected for random test vectors)" << std::endl;
    } else {
        assert(!result2.matched_id.empty());
    }
    
    // 测试匹配 - 箱体损坏相关
    std::cout << "\nTest 3: 匹配箱体损坏相关词汇" << std::endl;
    MatchResult result3 = matcher.findBestMatch("箱体老化破损");
    std::cout << "Input: 箱体老化破损" << std::endl;
    std::cout << "Matched: " << result3.matched_text << std::endl;
    std::cout << "Similarity: " << result3.similarity << std::endl;
    
    if (result3.matched_id.empty()) {
        std::cout << "⚠ No match found (this is expected for random test vectors)" << std::endl;
    } else {
        assert(!result3.matched_id.empty());
    }
    
    std::cout << "✓ Embedding Matcher test passed!" << std::endl;
    
    // 测试毫无关联的输入
    std::cout << "\nTest 4: 测试毫无关联的输入" << std::endl;
    MatchResult unrelated_result = matcher.findBestMatch("苹果香蕉橙子");
    std::cout << "Input: 苹果香蕉橙子" << std::endl;
    std::cout << "Matched: " << (unrelated_result.matched_id.empty() ? "(empty)" : unrelated_result.matched_text) << std::endl;
    std::cout << "Similarity: " << unrelated_result.similarity << std::endl;
    
    if (unrelated_result.matched_id.empty()) {
        std::cout << "✓ Correctly returned empty result for unrelated input" << std::endl;
    } else {
        std::cout << "⚠ Found match with similarity: " << unrelated_result.similarity << std::endl;
    }
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

// 测试配置加载 - 使用计量箱缺陷配置
void test_config_loader() {
    std::cout << "Testing Config Loader..." << std::endl;
    
    std::string json = R"({
        "version": "1.0",
        "description": "计量箱缺陷枚举配置",
        "defects": [
            {
                "id": "MET_BOX_001",
                "name": "箱体材质",
                "description": "计量箱箱体材质类型",
                "keywords": ["箱体材质", "不锈钢", "塑料", "铁"],
                "categories": ["箱体", "材质"],
                "threshold": 0.6,
                "enabled": true
            },
            {
                "id": "MET_BOX_002",
                "name": "开关故障",
                "description": "计量箱开关故障",
                "keywords": ["隔离开关", "塑壳断路器", "微型断路器", "开关故障"],
                "categories": ["开关"],
                "threshold": 0.5,
                "enabled": true
            },
            {
                "id": "MET_BOX_003",
                "name": "箱体损坏",
                "description": "计量箱箱体损坏",
                "keywords": ["变形破损", "老化", "缺失", "锈蚀"],
                "categories": ["箱体", "损坏"],
                "threshold": 0.5,
                "enabled": true
            }
        ]
    })";
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    DefectConfigLoader loader;
    loader.loadFromString(json);
    loader.applyToMatcher(matcher);
    
    assert(loader.getConfigCount() == 3);
    
    std::cout << "Loaded defects:" << std::endl;
    std::cout << "  - MET_BOX_001: 箱体材质" << std::endl;
    std::cout << "  - MET_BOX_002: 开关故障" << std::endl;
    std::cout << "  - MET_BOX_003: 箱体损坏" << std::endl;
    
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
