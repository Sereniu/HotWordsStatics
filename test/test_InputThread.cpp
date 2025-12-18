#include "InputThread.h"
#include "Buffer.h"
#include "Common.h"
#include <iostream>
#include <thread>
#include <queue>
#include <atomic>

int main() {
    std::cout << "========== 测试 InputThread ==========" << std::endl;
    
    // 创建 Buffer
    Buffer<TimeSlot> buffer(100, 20);
    
    // 创建查询队列
    std::queue<QueryCommand> query_queue;
    std::mutex query_mutex;
    
    // 运行标志
    std::atomic<bool> running(true);
    
    // 创建输入线程
    InputThread input_thread(
        "../data/test_input.txt",  
        buffer,
        query_queue,
        query_mutex,
        running,
        10  //批量大小
    );
    
    // 启动输入线程
    std::thread input_handle([&]() {
        input_thread.run();
    });
    
    // 模拟统计线程读取数据
    std::thread consumer([&]() {
        TimeSlot slot;
        int count = 0;
        
        while (true) {
            if (buffer.pop(slot)) {
                count++;
                std::cout << "[消费者] 读取数据 [时间=" << slot.timestamp 
                          << ", 词数=" << slot.words.size() << "]" << std::endl;
                
                // 显示前 3 个词
                for (size_t i = 0; i < std::min(size_t(3), slot.words.size()); ++i) {
                    std::cout << "  - " << slot.words[i] << std::endl;
                }
            } else {
                std::cout << "[消费者] Buffer 为空且输入已完成，退出" << std::endl;
                break;
            }
        }
        
        std::cout << "[消费者] 总共读取 " << count << " 条数据" << std::endl;
    });
    
    // 等待线程结束
    input_handle.join();
    consumer.join();
    
    // 检查查询队列
    std::cout << "========== 查询队列 ==========" << std::endl;
    std::cout << "查询数量: " << query_queue.size() << std::endl;
    
    while (!query_queue.empty()) {
        auto cmd = query_queue.front();
        query_queue.pop();
        std::cout << "查询 [时间=" << cmd.timestamp << ", K=" << cmd.k << "]" << std::endl;
    }
    
    std::cout << "测试完成！" << std::endl;
    
    return 0;
}

/**
 * 编译运行:
 * cd HotWordsStatics/src
 * g++ -std=c++17 test_InputThread.cpp InputThread.cpp InputHandler.cpp TextProcessor.cpp -pthread -o test_InputThread -I../include -I../cppjieba/include
 * ./test_InputThread
 */