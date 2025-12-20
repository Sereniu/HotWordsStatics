#include "HotWordSystem.h"
#include "spdlog/spdlog.h"

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
    // 【业务流程】系统初始化开始
    spdlog::info("=================================================");
    spdlog::info("===     HotWordSystem Initializing            ===");
    spdlog::info("=================================================");
    spdlog::info("Configuration:");
    spdlog::info("  Input file:       {}", input_file_);
    spdlog::info("  Output file:      {}", output_file_);
    spdlog::info("  Buffer capacity:  {}", buffer_capacity_);
    spdlog::info("  Low watermark:    {}", low_watermark_);
    spdlog::info("  Window size:      {}s ({}min)", window_size_, window_size_ / 60);
    spdlog::info("  Stat threads:     {}", num_stat_threads_);

    // 1. 创建输入线程对象
    spdlog::info("Creating InputThread...");
    input_thread_ = std::make_unique<InputThread>(
        input_file_,
        buffer_,
        query_queue_,
        query_mutex_,
        running_,
        100  // batch_size
    );
    spdlog::info("InputThread created successfully");
    
    // 2. 创建统计线程对象
    spdlog::info("Creating {} StatisticsThreads...", num_stat_threads_);
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
    spdlog::info("{} StatisticsThreads created successfully", stat_threads_.size());

    spdlog::info(">>> HotWordSystem Initialized Successfully <<<");
    spdlog::info("=================================================");
}

HotWordSystem::~HotWordSystem()=default;

void HotWordSystem::start()
{   
    spdlog::info("=================================================");
    spdlog::info("===        HotWordSystem Starting             ===");
    spdlog::info("=================================================");

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

    spdlog::info(">>> All Threads Started Successfully <<<");
    spdlog::info("System is now running...");
    spdlog::info("=================================================");
}

void HotWordSystem::stop()
{   
    spdlog::info("=================================================");
    spdlog::info("===       HotWordSystem Stopping              ===");
    spdlog::info("=================================================");

    running_.store(false);  // 设置系统停止标志

    buffer_.close();         // 关闭缓冲区，唤醒所有等待的线程

    // 清空查询队列，避免查询线程阻塞
    {
        std::lock_guard<std::mutex> lock(query_mutex_);
        while (!query_queue_.empty()) query_queue_.pop();
    }
    spdlog::info("System stop signal sent");
}

void HotWordSystem::join()
{
    spdlog::info("=================================================");
    spdlog::info("===    Waiting for Threads to Terminate       ===");
    spdlog::info("=================================================");

    // 等待输入线程退出
    if (input_thread_handle_.joinable()) {
        spdlog::info("Waiting for InputThread to finish...");
        input_thread_handle_.join();
        spdlog::info("InputThread terminated");
    }

    // 等待所有统计线程退出
    spdlog::info("Waiting for {} StatisticsThreads to finish...", 
                stat_thread_handles_.size());
    for (auto& t : stat_thread_handles_) {
        if (t.joinable()) {
            t.join();
        }
    }
    spdlog::debug("StatisticsThread [{}] terminated",stat_thread_handles_.size() );

    spdlog::info(">>> All Threads Terminated Successfully <<<");
    spdlog::info("=================================================");
}
