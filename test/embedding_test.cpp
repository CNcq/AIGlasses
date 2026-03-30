#include <iostream>
#include <iomanip>
#include "embedding_matcher.h"

using namespace ai_glasses;

int main() {
    std::cout << "=== AI Glasses Embedding 匹配测试 ===" << std::endl << std::endl;

    EmbeddingMatcher matcher;
    
    bool initialized = matcher.initialize("", "");
    if (!initialized) {
        std::cerr << "初始化失败!" << std::endl;
        return 1;
    }
    
    matcher.setSimilarityThreshold(0.5f);

    std::cout << "添加枚举项..." << std::endl;
    
    matcher.addEnumItem("DEFECT_001", "墙皮脱落", {
        "墙皮脱落", "墙面脱落", "墙皮剥落", "墙壁掉皮", "墙体脱落", 
        "墙面掉皮", "墙皮损坏", "墙皮缺陷", "墙留坠"
    });

    matcher.addEnumItem("DEFECT_002", "裂缝", {
        "裂缝", "裂痕", "开裂", "缝隙", "裂纹", "开裂缝", "墙面裂缝"
    });

    matcher.addEnumItem("DEFECT_003", "空鼓", {
        "空鼓", "空洞", "墙面空鼓", "墙壁空鼓", "墙体空鼓"
    });

    matcher.addEnumItem("DEFECT_004", "渗水", {
        "渗水", "漏水", "渗水", "潮湿", "水渍", "浸水"
    });

    matcher.addEnumItem("DEFECT_005", "发霉", {
        "发霉", "霉菌", "霉斑", "长霉", "霉变"
    });

    std::cout << "已加载 " << matcher.getEnumItemCount() << " 个枚举项" << std::endl << std::endl;

    std::vector<std::string> testCases = {
        "我发现在 xx 地方墙壁出现墙皮脱落，存在缺陷",
        "墙留坠很严重",
        "墙面有裂痕",
        "墙壁出现空鼓现象",
        "墙体开裂需要修复",
        "墙皮掉下来了",
        "卫生间渗水了",
        "天花板发霉长霉斑"
    };

    for (const auto& testInput : testCases) {
        std::cout << "测试输入: \"" << testInput << "\"" << std::endl;
        
        auto bestMatch = matcher.findBestMatch(testInput);
        std::cout << "最佳匹配:" << std::endl;
        std::cout << "  ID: " << bestMatch.matched_id << std::endl;
        std::cout << "  文本：" << bestMatch.matched_text << std::endl;
        std::cout << "  相似度：" << std::fixed << std::setprecision(4) << bestMatch.similarity << std::endl;
        std::cout << "  匹配成功：" << (bestMatch.is_match ? "是" : "否") << std::endl;

        auto allMatches = matcher.findAllMatches(testInput, 3);
        std::cout << "前 3 名匹配:" << std::endl;
        for (size_t i = 0; i < allMatches.size(); ++i) {
            std::cout << "  " << (i + 1) << ". [" << allMatches[i].matched_id << "] " 
                      << allMatches[i].matched_text 
                      << " (相似度：" << std::fixed << std::setprecision(4) 
                      << allMatches[i].similarity << ")" << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "=== 测试完成 ===" << std::endl;
    return 0;
}
