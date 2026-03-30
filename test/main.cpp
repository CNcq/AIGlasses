#include <iostream>
#include "semantic_matcher.h"

using namespace ai_glasses;

int main() {
    std::cout << "=== AI Glasses 语义匹配测试 ===" << std::endl << std::endl;

    SemanticMatcher matcher;
    matcher.setSimilarityThreshold(0.5f);

    matcher.addTermDictionary("墙皮脱落", {
        "墙留坠", "墙皮掉了", "墙面脱落", "墙皮剥落", "墙壁掉皮",
        "墙体脱落", "墙面掉皮", "墙皮损坏", "墙皮缺陷"
    });

    matcher.addTermDictionary("裂缝", {
        "裂痕", "开裂", "缝隙", "裂纹", "开裂缝", "墙面裂缝"
    });

    matcher.addTermDictionary("空鼓", {
        "空洞", "墙面空鼓", "墙壁空鼓", "墙体空鼓"
    });

    std::cout << "词典已加载，包含以下标准术语:" << std::endl;
    std::cout << "- 墙皮脱落 (9个同义词)" << std::endl;
    std::cout << "- 裂缝 (6个同义词)" << std::endl;
    std::cout << "- 空鼓 (4个同义词)" << std::endl << std::endl;

    std::vector<std::string> testCases = {
        "我发现在xx地方墙壁出现墙皮脱落，存在缺陷",
        "墙留坠很严重",
        "墙面有裂痕",
        "墙壁出现空鼓现象",
        "墙体开裂需要修复",
        "墙皮掉下来了"
    };

    for (const auto& testInput : testCases) {
        std::cout << "测试输入: \"" << testInput << "\"" << std::endl;
        
        auto bestMatch = matcher.findBestMatch(testInput);
        std::cout << "最佳匹配: " << bestMatch.matched_term 
                  << " (相似度: " << bestMatch.similarity 
                  << ", 匹配成功: " << (bestMatch.is_match ? "是" : "否") << ")" 
                  << std::endl;

        auto allMatches = matcher.findAllMatches(testInput, 3);
        std::cout << "前3名匹配:" << std::endl;
        for (size_t i = 0; i < allMatches.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << allMatches[i].matched_term 
                      << " (相似度: " << allMatches[i].similarity << ")" << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== 测试完成 ===" << std::endl;
    return 0;
}
