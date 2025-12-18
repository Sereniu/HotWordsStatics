#ifndef INPUTTHREAD_H
#define INPUTTHREAD_H

#include "Common.h"
#include "Buffer.h"
#include "TextProcessor.h"
#include "InputHandler.h"
#include <atomic>
#include <memory>
#include <queue>
#include <mutex>

/**
 * 输入线程，负责读取、分词、写入 Buffer
 */
class InputThread {
private:
    std::unique_ptr<InputHandler> input_handler_;//输入解析
    std::unique_ptr<TextProcessor> text_processor_;//分词过滤
    Buffer<TimeSlot>& buffer_;//循环缓冲区的引用（外部创建，线程贡献）
    
    std::queue<QueryCommand>& query_queue_;//外部创建，输入线程和统计线程共享
    std::mutex& query_mutex_;
    
    std::atomic<bool>& running_;//线程进行的标志
    size_t batch_size_;//批量写入大小
    
public:
    InputThread(const std::string& input_file,
                Buffer<TimeSlot>& buffer,
                std::queue<QueryCommand>& query_queue,
                std::mutex& query_mutex,
                std::atomic<bool>& running,
                size_t batch_size = 50);
    
    /**
     * 线程主函数
     * 打开输入文件
     * 逐行读取
     * 处理查询命令
     * 处理文本数据（解析+分词）
     * 收尾
     */
    void run();
};

#endif 
