#include "StatisticsThread.h"
#include "spdlog/spdlog.h"
#include <chrono>

void StatisticsThread::run()
{
    spdlog::info(">>> StatisticsThread [{}] Started <<<", thread_id_);

    // 性能统计
    auto thread_start_time = std::chrono::high_resolution_clock::now();
    auto last_report_time = thread_start_time;
    
    size_t processed_slots = 0;
    size_t processed_queries = 0;
    size_t processed_since_last_report = 0;
    double total_window_update_ms = 0.0;
    double total_query_process_ms = 0.0;

    while(true){
        TimeSlot slot;

        //Buffer pop 计时
        auto pop_start = std::chrono::high_resolution_clock::now();
                
        if (!buffer_.pop(slot)) {
            //Buffer 已空且输入结束
            spdlog::info("StatisticsThread [{}]: Buffer closed, exiting", thread_id_);
            break;
        }
        
        auto pop_end = std::chrono::high_resolution_clock::now();
        auto pop_ms = std::chrono::duration<double, std::milli>(pop_end - pop_start).count();
        
        if (pop_ms > 10.0) {
            spdlog::warn("StatisticsThread [{}]: Slow buffer pop: {:.2f}ms", 
                        thread_id_, pop_ms);
        }

        //更新滑动窗口
        auto window_start = std::chrono::high_resolution_clock::now();
        
        sliding_window_.addData(slot);
        
        auto window_end = std::chrono::high_resolution_clock::now();
        auto window_ms = std::chrono::duration<double, std::milli>(
            window_end - window_start).count();
        total_window_update_ms += window_ms;
        
        spdlog::debug("StatisticsThread [{}]: Window updated with timestamp={}, time={:.2f}ms", 
                     thread_id_, slot.timestamp, window_ms);

        //执行查询
        auto query_start = std::chrono::high_resolution_clock::now();
        
        processQueries();

        
        auto query_end = std::chrono::high_resolution_clock::now();
        auto query_ms = std::chrono::duration<double, std::milli>(
            query_end - query_start).count();
        total_query_process_ms += query_ms;

        processed_slots++;
        processed_since_last_report++;

        // 性能定期报告（每5秒）
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(now - last_report_time).count();
        
        if (elapsed >= 5.0) {
            double throughput = processed_since_last_report / elapsed;
            double avg_window_ms = total_window_update_ms / processed_since_last_report;
            double avg_query_ms = processed_queries > 0 ? 
                total_query_process_ms / processed_queries : 0.0;
            
            auto perf_logger = spdlog::get("perf");
            if (perf_logger) {
                perf_logger->info("{},stats_throughput_per_sec,{:.2f}", 
                                std::time(nullptr), throughput);
                perf_logger->info("{},stats_avg_window_update_ms,{:.3f}", 
                                std::time(nullptr), avg_window_ms);
                perf_logger->info("{},stats_avg_query_ms,{:.3f}", 
                                std::time(nullptr), avg_query_ms);
                
                //内存占用估算
                size_t memory_bytes = sliding_window_.estimateMemoryUsage();
                perf_logger->info("{},window_memory_kb,{:.2f}", 
                                std::time(nullptr), memory_bytes / 1024.0);
            }
            
            spdlog::info("--- StatisticsThread [{}] Performance ---", thread_id_);
            spdlog::info("Throughput: {:.2f} slots/sec", throughput);
            spdlog::info("Avg window update: {:.3f}ms", avg_window_ms);
            spdlog::info("Avg query process: {:.3f}ms", avg_query_ms);
            spdlog::info("Total processed: {} slots, {} queries", 
                        processed_slots, processed_queries);
            
            // 重置计数器
            processed_since_last_report = 0;
            total_window_update_ms = 0.0;
            total_query_process_ms = 0.0;
            last_report_time = now;
        }

    }

    // 线程结束统计
    auto thread_end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration<double>(
        thread_end_time - thread_start_time).count();
    
    spdlog::info("=================================================");
    spdlog::info("===  StatisticsThread [{}] Statistics         ===", thread_id_);
    spdlog::info("=================================================");
    spdlog::info("Processed slots:    {}", processed_slots);
    spdlog::info("Processed queries:  {}", processed_queries);
    spdlog::info("Total duration:     {:.2f}s", total_duration);
    spdlog::info("Overall throughput: {:.2f} slots/sec", 
                total_duration > 0 ? processed_slots / total_duration : 0.0);
    spdlog::info("=================================================");
    
    //性能最终统计
    auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},stats_total_slots,{}", std::time(nullptr), processed_slots);
        perf_logger->info("{},stats_duration_sec,{:.2f}", std::time(nullptr), total_duration);
    }
    
    spdlog::info("<<< StatisticsThread [{}] Terminated <<<", thread_id_);
}

void StatisticsThread::processQueries()
{
    std::lock_guard<std::mutex> lock(query_mutex_);

    unsigned int ts = sliding_window_.currentTime();
    int executed_count = 0;

    while(!query_queue_.empty()){
        QueryCommand query = query_queue_.front();

        if(query.timestamp <= ts){
            auto topk = sliding_window_.getTopK(query.k);

            // 使用滑动窗口时间
            query_handler_.outputTopK(ts, topk);
            
            // 【操作日志】记录查询执行
            auto op_logger = spdlog::get("operation");
            if (op_logger) {
                std::string result_str;
                for (size_t i = 0; i < topk.size() && i < 5; ++i) {
                    result_str += topk[i].first + "(" + std::to_string(topk[i].second) + ")";
                    if (i < std::min(topk.size(), size_t(5)) - 1) result_str += ", ";
                }
                if (topk.size() > 5) result_str += "...";
                
                op_logger->info("Query executed: timestamp={}, K={}, results=[{}]", 
                              ts, query.k, result_str);
            }
            
            spdlog::debug("Query executed: timestamp={}, K={}, results_count={}", 
                         ts, query.k, topk.size());

            query_queue_.pop();   // 只执行一次
            executed_count++;
        } else {
            spdlog::trace("Query waiting: query_timestamp={}, current={}", 
                query.timestamp, ts);
            break;
        }
    }
}

StatisticsThread::StatisticsThread(int thread_id, Buffer<TimeSlot> &buffer, SlidingWindow &sliding_window, QueryHandler &query_handler, std::queue<QueryCommand> &query_queue, std::mutex &query_mutex)
: thread_id_(thread_id),
      buffer_(buffer),
      sliding_window_(sliding_window),
      query_handler_(query_handler),
      query_queue_(query_queue),
      query_mutex_(query_mutex)
{
    spdlog::info("=== StatisticsThread [{}] Initialized ===", thread_id_);
}
