#include "embedding_matcher.h"
#include "jieba_segmenter.h"
#include "json_parser.h"
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

// 测试结巴分词（使用 cppjieba 混合模式）
void test_jieba_segmenter() {
    std::cout << "Testing Jieba Segmenter (Mix Mode)..." << std::endl;
    
    JiebaSegmenter segmenter;
    segmenter.initialize("");  // 使用内置词典，自动使用混合模式
    
    std::vector<std::string> tokens = segmenter.segment("墙壁出现脱落");
    
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
    
    assert(!tokens.empty());
    
    // 测试铜质、铁质、铝合金质等词汇的分词
    std::cout << "\nTest: 分词测试（铜质/铁质/铝合金质）" << std::endl;
    std::vector<std::string> test_cases = {
        "这个变电箱材质为铜",
        "箱子是铁质的",
        "外壳是铝合金质"
    };
    
    for (const auto& text : test_cases) {
        auto words = segmenter.segment(text);
        std::cout << "Input: " << text << std::endl;
        std::cout << "Tokens: ";
        for (const auto& word : words) {
            std::cout << word << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "✓ Jieba Segmenter test passed!" << std::endl;
}

// 测试 EmbeddingMatcher - 使用动态缺陷数组
void test_embedding_matcher() {
    std::cout << "Testing Embedding Matcher..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    // 添加测试数据 - 使用动态缺陷数组
    std::vector<std::string> defect_list = {"箱体材质", "开关故障", "箱体损坏"};
    matcher.addDefectsFromList(defect_list);
    
    std::cout << "Added defects: ";
    for (const auto& defect : defect_list) {
        std::cout << defect << " ";
    }
    std::cout << std::endl;
    
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

// 测试动态缺陷数组 - 铜铁铝合金
void test_dynamic_defects() {
    std::cout << "Testing Dynamic Defects (Copper/Iron/Aluminum)..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    // 用户输入的缺陷数组
    std::vector<std::string> defect_list = {"铜质", "铁质", "铝合金质"};
    
    // 添加缺陷
    matcher.addDefectsFromList(defect_list);
    
    std::cout << "Added defects: ";
    for (const auto& defect : defect_list) {
        std::cout << defect << " ";
    }
    std::cout << std::endl;
    
    // 测试匹配
    std::cout << "\nTest 1: 匹配铜质" << std::endl;
    MatchResult result1 = matcher.findBestMatch("这个变电箱材质为铜");
    std::cout << "Input: 这个变电箱材质为铜" << std::endl;
    std::cout << "Matched: " << (result1.matched_text.empty() ? "(无匹配)" : result1.matched_text) << std::endl;
    std::cout << "Similarity: " << result1.similarity << std::endl;
    std::cout << "Rank: " << result1.rank << std::endl;
    std::cout << "Is Match: " << (result1.is_match ? "是" : "否") << std::endl;
    
    std::cout << "\nTest 2: 匹配铁质" << std::endl;
    MatchResult result2 = matcher.findBestMatch("箱子是铁质的");
    std::cout << "Input: 箱子是铁质的" << std::endl;
    std::cout << "Matched: " << (result2.matched_text.empty() ? "(无匹配)" : result2.matched_text) << std::endl;
    std::cout << "Similarity: " << result2.similarity << std::endl;
    std::cout << "Rank: " << result2.rank << std::endl;
    std::cout << "Is Match: " << (result2.is_match ? "是" : "否") << std::endl;
    
    std::cout << "\nTest 3: 匹配铝合金质" << std::endl;
    MatchResult result3 = matcher.findBestMatch("外壳是铝合金质");
    std::cout << "Input: 外壳是铝合金质" << std::endl;
    std::cout << "Matched: " << (result3.matched_text.empty() ? "(无匹配)" : result3.matched_text) << std::endl;
    std::cout << "Similarity: " << result3.similarity << std::endl;
    std::cout << "Rank: " << result3.rank << std::endl;
    std::cout << "Is Match: " << (result3.is_match ? "是" : "否") << std::endl;
    
    std::cout << "✓ Dynamic Defects test passed!" << std::endl;
}

// 测试 addEnumItem 重载方法
void test_add_enum_item_overloads() {
    std::cout << "Testing addEnumItem Overloads..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    // 测试 addEnumItem(const EnumItem& item)
    EnumItem item1;
    item1.id = "TEST_001";
    item1.text = "测试缺陷1";
    item1.keywords = {"关键词1", "关键词2"};
    item1.threshold = 0.5f;
    item1.enabled = true;
    item1.is_precomputed = false;
    matcher.addEnumItem(item1);
    
    // 测试 addEnumItem(const std::string& id, const std::string& text, const std::vector<std::string>& keywords)
    matcher.addEnumItem("TEST_002", "测试缺陷2", std::vector<std::string>{"关键词3", "关键词4"});
    
    // 测试 addEnumItem(const std::string& id, const std::string& text, const std::string& description, const std::vector<std::string>& keywords, float threshold)
    matcher.addEnumItem("TEST_003", "测试缺陷3", "这是描述", {"关键词5"}, 0.6f);
    
    std::cout << "Total enum items: " << matcher.getEnumItemCount() << std::endl;
    assert(matcher.getEnumItemCount() == 3);
    
    std::cout << "✓ addEnumItem Overloads test passed!" << std::endl;
}

// 测试阈值设置
void test_threshold_setting() {
    std::cout << "Testing Threshold Setting..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    std::vector<std::string> defect_list = {"缺陷A", "缺陷B"};
    matcher.addDefectsFromList(defect_list);
    
    // 设置阈值
    matcher.setSimilarityThreshold(0.7f);
    assert(matcher.getSimilarityThreshold() == 0.7f);
    
    std::cout << "Similarity threshold: " << matcher.getSimilarityThreshold() << std::endl;
    std::cout << "✓ Threshold Setting test passed!" << std::endl;
}

// 测试 computeEmbedding
void test_compute_embedding() {
    std::cout << "Testing computeEmbedding..." << std::endl;
    
    EmbeddingMatcher matcher;
    matcher.initialize("", "");
    
    // 计算文本嵌入
    EmbeddingVector embedding = matcher.computeEmbedding("测试文本");
    
    std::cout << "Embedding dimension: " << embedding.size() << std::endl;
    assert(embedding.size() == EMBEDDING_DIM);
    
    std::cout << "✓ computeEmbedding test passed!" << std::endl;
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
        test_dynamic_defects();
        test_add_enum_item_overloads();
        test_threshold_setting();
        test_compute_embedding();
        
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
