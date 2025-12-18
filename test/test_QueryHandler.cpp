#include "QueryHandler.h"
#include <iostream>
#include <vector>
#include <iomanip>

int main(){
    QueryHandler handler("../data/test_output.txt");

    std::vector<std::pair<std::string, int>> topk1 = {
        {"中山大学", 15},
        {"计算机", 12},
        {"学习", 10},
        {"深度", 8},
        {"Python", 6}
    };

    std::cout<<"第一次查询结果(300秒)"<<std::endl;
    handler.outputTopK(300,topk1);// 时间戳 300 秒 = [00:05:00]

    std::vector<std::pair<std::string, int>> topk2 = {
        {"人工智能", 20},
        {"机器学习", 18},
        {"神经网络", 15}
    };

    handler.outputTopK(720, topk2);  // 时间戳 720 秒 = [00:12:00]
    std::cout << "第二次查询结果 (720秒)" << std::endl;

       
    std::cout << "所有测试完成！请检查 test_output.txt 文件内容。" << std::endl;

    return 0;
}


/**
 * cd HotWordsStatics/src
 * g++ -std=c++17 test_QueryHandler.cpp QueryHandler.cpp -pthread -o test_QueryHandler
 * ./test_QueryHandler
 */
