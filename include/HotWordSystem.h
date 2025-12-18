#ifndef HOTWORDSYSTEM_H
#define HOTWORDSYSTEM_H

#include "Common.h"
#include "Buffer.h"
#include "SlidingWindow.h"
#include "QueryHandler.h"
#include "InputThread.h"
#include "StatisticsThread.h"
#include <string>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

/**
 * @class HotWordSystem
 * @brief 热词统计系统总控类，负责管理输入线程、统计线程及缓冲区。
 *
 * 系统功能：
 * 1. 从文件读取文本数据和查询指令。
 * 2. 将文本数据放入缓冲区。
 * 3. 多线程统计热词信息。
 * 4. 响应查询请求并输出结果。
 */
class HotWordSystem {
private:
    std::string input_file_;//输入文件路径
    std::string output_file_;//输出文件路径
    size_t buffer_capacity_;//循环缓冲区容量
    size_t low_watermark_;//剩余数据量阈值
    uint32_t window_size_;//滑动窗口大小（秒）
    size_t num_stat_threads_;//统计线程数量
    
    Buffer<TimeSlot> buffer_;//循环缓冲区（生产消费）
    SlidingWindow sliding_window_;//滑动窗口
    QueryHandler query_handler_;//查询处理器
    std::queue<QueryCommand> query_queue_;//查询指令队列
    std::mutex query_mutex_;
    std::atomic<bool> running_;//线程进行标志
    
    std::unique_ptr<InputThread> input_thread_;//输入线程
    std::vector<std::unique_ptr<StatisticsThread>> stat_threads_;//统计线程对象队列
    std::thread input_thread_handle_;//输入线程（实例）
    std::vector<std::thread> stat_thread_handles_;//统计线程实例列表
    
public:
    HotWordSystem(const std::string& input_file,
                  const std::string& output_file = "output.txt",
                  size_t buffer_capacity = 500,
                  size_t low_watermark = 100,
                  uint32_t window_size = 600,
                  size_t num_stat_threads = 2);
    
    ~HotWordSystem();
    
    /**
     * @brief 启动系统，包括输入线程和统计线程
     */
    void start();

    /**
     * @brief 停止系统，通知所有线程安全退出
     */
    void stop();

    /**
     * @brief 等待所有线程退出（join）
     */
    void join();
};

#endif
