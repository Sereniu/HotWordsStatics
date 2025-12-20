#include "HotWordSystem.h"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

//日志系统头文件
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"

using namespace std;

int main(int argc, char* argv[]){

    //初始化异步日志系统
    try {
        // 创建日志目录
        system("mkdir -p ../logs");
        
        // 初始化异步线程池
        spdlog::init_thread_pool(8192, 1); // 队列8192，1个后台线程
        
        // 创建系统日志 (文件 + 控制台)
        auto system_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "../logs/system.log", 1024*1024*5, 3); // 5MB滚动，保留3个备份
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        
        std::vector<spdlog::sink_ptr> system_sinks{system_file_sink, console_sink};
        auto system_logger = std::make_shared<spdlog::async_logger>(
            "system", 
            system_sinks.begin(), 
            system_sinks.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block // 队列满时阻塞，不丢日志
        );
        
        // 创建性能日志 (仅文件，CSV格式)
        auto perf_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "../logs/performance.log", 1024*1024*10, 5);
        auto perf_logger = std::make_shared<spdlog::async_logger>(
            "perf",
            perf_file_sink,
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );
        perf_logger->set_pattern("%v"); // CSV格式，不加时间戳前缀
        
        // 创建操作日志 (用户查询等)
        auto op_file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
            "../logs/operation.log", 0, 0);
        auto op_logger = std::make_shared<spdlog::async_logger>(
            "operation",
            op_file_sink,
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block
        );
        
        // 设置日志级别
        #ifdef DEBUG_MODE  // 调试模式：开启所有日志
        system_logger->set_level(spdlog::level::trace);//trace
        #else  // 正式模式：屏蔽 trace/debug
        system_logger->set_level(spdlog::level::info);//info
        #endif
        
        system_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        
        op_logger->set_level(spdlog::level::info);//info
        op_logger->set_pattern("[%Y-%m-%d %H:%M:%S] [OPERATION] %v");
        
        perf_logger->set_level(spdlog::level::info);//info
        
        // 注册到全局
        spdlog::register_logger(system_logger);
        spdlog::register_logger(perf_logger);
        spdlog::register_logger(op_logger);
        spdlog::set_default_logger(system_logger); // 默认使用system_logger
        
        // 每秒自动刷新
        spdlog::flush_every(std::chrono::seconds(1));
        spdlog::flush_on(spdlog::level::err); // ERROR级别立即刷新
        system_logger->flush_on(spdlog::level::trace);
        
    } catch (const spdlog::spdlog_ex& ex) {
        cerr << "日志初始化失败: " << ex.what() << endl;
        return 1;
    }
    

    spdlog::info("=================================================");
    spdlog::info("Hot Word Statistics System Starting");

    string input_file="../data/";
    string output_file="../data/";

     // 检查参数数量
    if (argc < 3) {
        spdlog::error("参数不足！需要: <input_file> <output_file>");
        spdlog::shutdown(); // 关键：退出前关闭日志
        return 1;
    }

    input_file += argv[1];
    output_file += argv[2];

    spdlog::info("Input file: {}", input_file);
    spdlog::info("Output file: {}", output_file);

    try{
        auto start_time=chrono::high_resolution_clock::now();

        spdlog::info("Creating HotWordSystem instance...");
        spdlog::info("Parameters: buffer_capacity_=300s, low_watermark_=60s, window_size_=600, running_=1");
        
        //创建热词统计系统
        HotWordSystem system(input_file,output_file,300, 60,600,1);

        spdlog::info("HotWordSystem created successfully");

        system.start();
        system.join();

        // 记录结束时间
        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
        
        // 记录性能指标
        auto perf_logger = spdlog::get("perf");
        if (perf_logger) {
            perf_logger->info("timestamp,metric_name,value");
            perf_logger->info("{},total_duration_ms,{}", 
                chrono::system_clock::to_time_t(chrono::system_clock::now()), 
                duration.count());
            if (duration.count() > 0) {
                perf_logger->info("{},processing_rate_per_sec,{:.2f}",
                    chrono::system_clock::to_time_t(chrono::system_clock::now()),
                    1000.0 / duration.count());
            }
        }
        
        spdlog::info("=================================================");
        spdlog::info("Processing completed successfully");
        spdlog::info("Total duration: {} ms", duration.count());
        spdlog::info("Output file: {}", output_file);
        spdlog::info("=================================================");
        
        spdlog::info("Shutting down logger system...");
      //  spdlog::shutdown(); // 等待异步队列清空，关闭后台线程
        
        return 0;
    }catch(const exception& e){
        // 记录异常
        spdlog::critical("System crashed with exception: {}", e.what());
        spdlog::shutdown(); // 异常退出也要关闭
    }
}