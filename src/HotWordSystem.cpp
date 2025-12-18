#include "HotWordSystem.h"

HotWordSystem::HotWordSystem(const std::string &input_file, const std::string &output_file, size_t buffer_capacity, size_t low_watermark, uint32_t window_size, size_t num_stat_threads)
 :  input_file_(input_file),
    output_file_(output_file),
    buffer_capacity_(buffer_capacity),
    low_watermark_(low_watermark),
    window_size_(window_size),
    num_stat_threads_(num_stat_threads),
    buffer_(buffer_capacity_, low_watermark_),
    sliding_window_(window_size_),
    query_handler_(output_file_),
    running_(true) // 初始为运行状态
{
      // 1. 创建输入线程对象
    input_thread_ = std::make_unique<InputThread>(
        input_file_,
        buffer_,
        query_queue_,
        query_mutex_,
        running_,
        100  // batch_size
    );
    std::cout << "[HotWordSystem] InputThread 创建完成" << std::endl;
    
    // 2. 创建统计线程对象
    for (size_t i = 0; i < num_stat_threads_; i++) {
        stat_threads_.emplace_back(
            std::make_unique<StatisticsThread>(
                i,
                buffer_,
                sliding_window_,
                query_handler_,
                query_queue_,
                query_mutex_
            )
        );
    }
    std::cout << "[HotWordSystem] " << stat_threads_.size() << " 个 StatisticsThread 创建完成" << std::endl;
}

HotWordSystem::~HotWordSystem()=default;

void HotWordSystem::start()
{
    // 启动输入线程
    input_thread_handle_ = std::thread([this]() {
        input_thread_->run();
    });

    // 启动统计线程
    for (auto& stat_thread : stat_threads_) {
        stat_thread_handles_.emplace_back([&stat_thread]() {
            stat_thread->run();
        });
    }

    std::cout << "[HotWordSystem] 系统启动完成" << std::endl;
}

void HotWordSystem::stop()
{
    running_.store(false);  // 设置系统停止标志

    buffer_.close();         // 关闭缓冲区，唤醒所有等待的线程

    // 清空查询队列，避免查询线程阻塞
    {
        std::lock_guard<std::mutex> lock(query_mutex_);
        while (!query_queue_.empty()) query_queue_.pop();
    }

    std::cout << "[HotWordSystem] 系统停止信号发送" << std::endl;
}

void HotWordSystem::join()
{
    // 等待输入线程退出
    if (input_thread_handle_.joinable()) {
        input_thread_handle_.join();
    }

    // 等待所有统计线程退出
    for (auto& t : stat_thread_handles_) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "[HotWordSystem] 所有线程已退出" << std::endl;
}
