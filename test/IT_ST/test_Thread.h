#ifndef TEST_THREAD_H
#define TEST_THREAD_H

// StatisticsThread + InputThread + Buffer 集成测试
#include "IntegrationTestBase.h"
#include "Buffer.h"
#include "InputThread.h"
#include "StatisticsThread.h"
#include "SlidingWindow.h"
#include "QueryHandler.h"
#include "Common.h"
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

class test_Thread : public IntegrationTestBase {
private:
    string input_file_ = "../../data/test_input.txt";
    string output_file_ = "../../data/test_thread_output1.txt";

protected:
    void SetUp() override {
        LOG("使用输入文件: " + input_file_);
        LOG("输出文件: " + output_file_);
    }
    
    void TearDown() override {
        LOG("测试完成，输出文件已保留: " + output_file_);
    }
    
    void RunTest() override {
        // 测试1: 单统计线程完整流程
        {
            Buffer<TimeSlot> buffer(100, 20);
            SlidingWindow window(10);
            QueryHandler handler(output_file_);
            queue<QueryCommand> query_queue;
            mutex query_mutex;
            atomic<bool> running(true);

            
            
            LOG("=== 测试1: 单统计线程完整流程 ===");
            
            // 输入线程（生产者）
            thread input_thread([&]() {
                InputThread input(input_file_, buffer, query_queue, 
                                query_mutex, running, 20);
                input.run();
            });
            
            // 统计线程（消费者）
            thread stat_thread([&]() {
                StatisticsThread stat(1, buffer, window, handler, 
                                    query_queue, query_mutex);
                stat.run();
            });
            
            input_thread.join();
            stat_thread.join();
            handler.close();
            
            LOG("  完整数据流处理成功");
            LOG("  窗口统计: 总词数=" + to_string(window.getTotalWords()) + 
                ", 唯一词数=" + to_string(window.getUniqueWords()));
        }
        
        // // 测试2: 多统计线程并发协作
        // {
        //     Buffer<TimeSlot> buffer(100, 20);
        //     SlidingWindow window(600);
        //     string multi_output = "../../data/test_thread_output_multi.txt";
        //     QueryHandler handler(multi_output);
        //     queue<QueryCommand> query_queue;
        //     mutex query_mutex;
        //     atomic<bool> running(true);
            
        //     LOG("=== 测试2: 多统计线程并发协作 ===");
            
        //     // 输入线程（生产者）
        //     thread input_thread([&]() {
        //         InputThread input(input_file_, buffer, query_queue, 
        //                         query_mutex, running, 20);
        //         input.run();
        //     });
            
        //     // 3个统计线程（消费者）
        //     vector<thread> stat_threads;
        //     for (int i = 0; i < 3; i++) {
        //         stat_threads.emplace_back([&, i]() {
        //             StatisticsThread stat(i, buffer, window, handler, 
        //                                 query_queue, query_mutex);
        //             stat.run();
        //         });
        //     }
            
        //     input_thread.join();
        //     for (auto& t : stat_threads) {
        //         t.join();
        //     }
        //     handler.close();
            
        //     // 验证结果
        //     ASSERT_GT(window.getTotalWords(), 0, "窗口未统计到任何词");
        //     ASSERT_GT(window.getUniqueWords(), 0, "窗口未统计到唯一词");
            
        //     LOG("  3个统计线程协作完成");
        //     LOG("  窗口统计: 总词数=" + to_string(window.getTotalWords()) + 
        //         ", 唯一词数=" + to_string(window.getUniqueWords()));
        // }
    }
public:
    test_Thread() 
        : IntegrationTestBase("StatisticsThread + InputThread + Buffer 集成测试") {}
};

#endif // TEST_THREAD_H