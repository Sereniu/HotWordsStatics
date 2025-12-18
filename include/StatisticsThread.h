#ifndef STATISTICSTHREAD_H
#define STATISTICSTHREAD_H

#include "Common.h"
#include "Buffer.h"
#include "SlidingWindow.h"
#include "QueryHandler.h"
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>

/**
 * get TimeSlot from buffer
 * call addData to add TimeSlot to SlidingWindow
 * check QueryCommand and cout TopK
 */
class StatisticsThread {
private:
    int thread_id_;//线程编号
    Buffer<TimeSlot>& buffer_;//缓冲区
    SlidingWindow& sliding_window_;//时间窗口
    QueryHandler& query_handler_;//输出结果
    
    std::queue<QueryCommand>& query_queue_;//保存查询请求
    std::mutex& query_mutex_;
    
public:
    //线程初始化
    StatisticsThread(int thread_id,
                     Buffer<TimeSlot>& buffer,
                     SlidingWindow& sliding_window,
                     QueryHandler& query_handler,
                     std::queue<QueryCommand>& query_queue,
                     std::mutex& query_mutex);
    
    /**
     * 核心主循环
     * 从buffer_在获取TimeSlot
     * 更新SlidingWindow中的词频统计
     * 根据当前时间处理查询TopK
     */
    void run();
    
private:
    //执行查询（工具函数）
    void processQueries();
};

#endif
