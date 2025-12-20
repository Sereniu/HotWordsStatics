#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include "TextProcessor.h"

int main(){
    std::cout << "测试 1: 基础分词对比" << std::endl;
    
    // 不带词性
    TextProcessor processor("../dict/", false);
    auto words1_a = processor.process("我在中山大学的计算机学院学习深度学习，感觉非常好");
    std::cout << "不带词性: ";
    for (const auto& w : words1_a) std::cout << w << " ";
    std::cout << std::endl;
    
    // 带词性
    TextProcessor processorPOS("../dict/", true);
    auto words1_b = processorPOS.processWithPOS("我在中山大学的计算机学院学习深度学习，感觉非常好");
    std::cout << "带词性:   ";
    for (const auto& w : words1_b) std::cout << w << " ";
    std::cout << std::endl;

    std::cout << "测试 2: 数字过滤对比" << std::endl;
    
    auto words2_a = processor.process("今天是2024年1月1日，我在中山大学学习，有500多名学生");
    std::cout << "不带词性: ";
    for (const auto& w : words2_a) std::cout << w << " ";
    std::cout << std::endl;
    
    auto words2_b = processorPOS.processWithPOS("今天是2024年1月1日，我在中山大学学习，有500多名学生");
    std::cout << "带词性:   ";
    for (const auto& w : words2_b) std::cout << w << " ";
    std::cout << std::endl;

    std::cout << "测试 3: 符号过滤对比" << std::endl;
    
    auto words3_a = processor.process("你好！！！这是\"测试\"，，，你知道吗？？？——【重要】");
    std::cout << "不带词性: ";
    for (const auto& w : words3_a) std::cout << w << " ";
    std::cout << std::endl;
    
    auto words3_b = processorPOS.processWithPOS("你好！！！这是\"测试\"，，，你知道吗？？？——【重要】");
    std::cout << "带词性:   ";
    for (const auto& w : words3_b) std::cout << w << " ";
    std::cout << std::endl;

    std::cout << "测试 5: 敏感词过滤对比" << std::endl;
    
    auto words5_a = processor.process("我真的不想骂你，但你是智障吗？垃圾广告真多");
    std::cout << "不带词性: ";
    for (const auto& w : words5_a) std::cout << w << " ";
    std::cout << std::endl;
    
    auto words5_b = processorPOS.processWithPOS("我真的不想骂你，但你是智障吗？垃圾广告真多");
    std::cout << "带词性:   ";
    for (const auto& w : words5_b) std::cout << w << " ";
    std::cout << std::endl;
}

/**
 * cd HotWordsStatics/src
 * g++ -std=c++17 test_TextProcessor.cpp ../src/TextProcessor.cpp -pthread -o test_TextProcessor -I ../include
 * ./test_TextProcessor
 */
