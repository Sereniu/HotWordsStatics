#ifndef TEST_INPUT_BUFFER_H
#define TEST_INPUT_BUFFER_H

//InputThread+Buffer
#include "IntegrationTestBase.h"
#include "Buffer.h"
#include "InputThread.h"
#include "Common.h"
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

class test_Input_Buffer : public IntegrationTestBase {
private:
    string input_file_ = "../../data/test_input.txt";

protected:
    void SetUp() override {
        LOG("[Input_Buffer] 输入文件"+input_file_);
    }
    
    void TearDown() override {}
    
    void RunTest() override {
        {
            Buffer<TimeSlot> buffer(100, 20);
            queue<QueryCommand> query_queue;
            mutex query_mutex;
            atomic<bool> running(true);
            atomic<int> consumed(0);
            
            LOG("=== 测试1: 单统计线程并发处理 ===");
            
            thread producer([&]() {
                InputThread input(input_file_, buffer, query_queue, query_mutex, running, 20);
                input.run();
            });
            
             thread consumer([&]() {
                TimeSlot slot;
                while (buffer.pop(slot)) {
                    consumed++;
                    ASSERT_GT(slot.words.size(), 0, "TimeSlot词列表为空");
                }
            });
            
            producer.join();
            consumer.join();
            
            LOG("  消费了 " + to_string(consumed.load()) + " 个TimeSlot");
            
            lock_guard<mutex> lock(query_mutex);
            LOG("  查询队列包含 " + to_string(query_queue.size()) + " 条命令");
        }
        
        // 测试2: 多线程并发
        // {
        //     Buffer<TimeSlot> buffer(100, 20);
        //     queue<QueryCommand> query_queue;
        //     mutex query_mutex;
        //     atomic<bool> running(true);
        //     atomic<int> total_consumed(0);
            
        //     LOG("=== 测试2: 多统计线程并发处理 ===");
            
        //     thread producer([&]() {
        //         InputThread input(input_file_, buffer, query_queue, query_mutex, running, 20);
        //         input.run();
        //     });
            
        //     vector<thread> consumers;
        //     for (int i = 0; i < 3; i++) {
        //         consumers.emplace_back([&]() {
        //             TimeSlot slot;
        //             int local_count = 0;
        //             while (buffer.pop(slot)) {
        //                 local_count++;
        //             }
        //             total_consumed += local_count;
        //         });
        //     }
            
        //     producer.join();
        //     for (auto& t : consumers) {
        //         t.join();
        //     }
            
        //     LOG("  3个消费者共消费 " + to_string(total_consumed.load()) + " 个TimeSlot");
        // }
        
    }
    
public:
    test_Input_Buffer() 
        : IntegrationTestBase("Input + Buffer 集成（流控测试）") {}
};

#endif