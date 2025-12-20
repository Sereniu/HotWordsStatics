#include "InputThread.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include <chrono>


InputThread::InputThread(const std::string &input_file, Buffer<TimeSlot> &buffer, std::queue<QueryCommand> &query_queue, std::mutex &query_mutex, std::atomic<bool> &running, size_t batch_size):
    buffer_(buffer),
    query_queue_(query_queue),
    query_mutex_(query_mutex),
    running_(running),
    batch_size_(batch_size)
{
    spdlog::info("=== InputThread Initializing ===");
    spdlog::info("Input file: {}", input_file);
    spdlog::info("Batch size: {}", batch_size_);

    input_handler_=std::make_unique<InputHandler>(input_file);
    text_processor_ = std::make_unique<TextProcessor>("../dict/", true);//如果是为了测试可以临时改为../../dict,正式运行为../dict

    spdlog::info(">>> InputThread Initialized Successfully <<<");
}

void InputThread::run()
{
    spdlog::info(">>> InputThread Started <<<");

    // 性能统计变量
    auto thread_start_time = std::chrono::high_resolution_clock::now();
    auto last_report_time = thread_start_time;

    if (!input_handler_->open()) {
        spdlog::critical("InputThread: Failed to open input file");
        buffer_.markInputFinished(); // 缓冲区结束输入
        running_.store(false); // 结束进程
        return;
    }

    // 批次缓存,提高吞吐量
    std::vector<TimeSlot> batch;
    batch.reserve(batch_size_);

    // 统计信息，便于调试（日志）
    size_t total_lines = 0;
    size_t text_lines = 0;
    size_t query_lines = 0;
    size_t total_words = 0;

    // 性能统计
    size_t processed_since_last_report = 0;
    double total_preprocess_time_ms = 0.0;

    // 数据的变量
    unsigned int timestamp;
    std::string text;
    bool is_query;
    int k;

    while (running_.load() && !input_handler_->eof()) {
        // 读取一行
        if (!input_handler_->readLine(timestamp, text, is_query, k)) {
            continue;
        }

        total_lines++;
        processed_since_last_report++;

        if (is_query) {
            query_lines++;

            // 在锁中调用而后释放
            {
                std::lock_guard<std::mutex> lock(query_mutex_);
                query_queue_.push(QueryCommand(timestamp, k));
            }

            spdlog::info("Query command received: timestamp={}, K={}", timestamp, k);

            auto op_logger = spdlog::get("operation");
            if (op_logger) {
                op_logger->info("Query enqueued: timestamp={}, K={}", timestamp, k);
            }

            continue;
        }

        text_lines++;

        // 文本预处理时间计时
        auto preprocess_start = std::chrono::high_resolution_clock::now();

        TimeSlot slot(timestamp);
        slot.words = text_processor_->processWithPOS(text);

        auto preprocess_end = std::chrono::high_resolution_clock::now();
        auto preprocess_ms = std::chrono::duration<double, std::milli>(
            preprocess_end - preprocess_start).count();
        total_preprocess_time_ms += preprocess_ms;

        total_words += slot.words.size();

        if (!slot.words.empty()) {
            batch.push_back(std::move(slot));
            spdlog::trace("Text processed: timestamp={}, words={}, time={:.2f}ms", 
                         timestamp, slot.words.size(), preprocess_ms);
        }

        if (batch.size() >= batch_size_) {
            // 批次提交时间计时
            auto batch_start = std::chrono::high_resolution_clock::now();

            bool success = true;
            int pushed_count = 0;
            for (auto& item : batch) {
                if (!buffer_.push(std::move(item))) {
                    spdlog::warn("Buffer closed, stopping input. Pushed {}/{} items", 
                               pushed_count, batch.size());
                    success = false;
                    break;
                }
                pushed_count++;
            }
            
            auto batch_end = std::chrono::high_resolution_clock::now();
            auto batch_ms = std::chrono::duration<double, std::milli>(
                batch_end - batch_start).count();
            
            spdlog::debug("Batch submitted: size={}, time={:.2f}ms", batch.size(), batch_ms);

            batch.clear();

            if (!success) {
                break;
            }
        }

        // 定期报告吞吐量（每5秒）
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(now - last_report_time).count();

        if (elapsed >= 5.0) {
            double throughput = processed_since_last_report / elapsed;
            double avg_preprocess_ms = processed_since_last_report > 0 ? 
                total_preprocess_time_ms / processed_since_last_report : 0.0;
            
            auto perf_logger = spdlog::get("perf");
            if (perf_logger) {
                perf_logger->info("{},input_throughput_per_sec,{:.2f}", 
                                std::time(nullptr), throughput);
                perf_logger->info("{},avg_preprocess_ms,{:.3f}", 
                                std::time(nullptr), avg_preprocess_ms);
            }
            
            spdlog::info("--- InputThread Performance ---");
            spdlog::info("Throughput: {:.2f} lines/sec", throughput);
            spdlog::info("Avg preprocess: {:.3f}ms/line", avg_preprocess_ms);
            spdlog::info("Processed: {} lines, {} words", 
                        processed_since_last_report, total_words);
            
            // 重置计数器
            processed_since_last_report = 0;
            total_preprocess_time_ms = 0.0;
            last_report_time = now;
        }
    } 

    // 清理+日志
    spdlog::info("InputThread: Submitting remaining {} items in batch", batch.size());
   
    for (auto& item : batch) {
        if (!buffer_.push(std::move(item))) {
            spdlog::warn("Buffer closed while submitting remaining items");
            break;
        }   
    }
    
    buffer_.markInputFinished();
    input_handler_->close();
    
    //计算总时长
    auto thread_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration<double>(
        thread_end_time - thread_start_time).count();
    
    spdlog::info("=================================================");
    spdlog::info("===       InputThread Statistics              ===");
    spdlog::info("=================================================");
    spdlog::info("Total lines:        {}", total_lines);
    spdlog::info("Text lines:         {}", text_lines);
    spdlog::info("Query lines:        {}", query_lines);
    spdlog::info("Total words:        {}", total_words);
    spdlog::info("Avg words/text:     {:.2f}", 
                text_lines > 0 ? (double)total_words / text_lines : 0.0);
    spdlog::info("Total duration:     {:.2f}s", total_duration);
    spdlog::info("Overall throughput: {:.2f} lines/sec", 
                total_duration > 0 ? total_lines / total_duration : 0.0);
    spdlog::info("=================================================");
    
    // 最终性能汇总
    auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},input_total_lines,{}", std::time(nullptr), total_lines);
        perf_logger->info("{},input_text_lines,{}", std::time(nullptr), text_lines);
        perf_logger->info("{},input_total_words,{}", std::time(nullptr), total_words);
        perf_logger->info("{},input_duration_sec,{:.2f}", std::time(nullptr), total_duration);
        perf_logger->info("{},input_overall_throughput,{:.2f}", 
                        std::time(nullptr), 
                        total_duration > 0 ? total_lines / total_duration : 0.0);
    }

    spdlog::info("<<< InputThread Terminated <<<");
}