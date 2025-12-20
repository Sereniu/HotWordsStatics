#include "QueryHandler.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include <chrono>

QueryHandler::QueryHandler(const std::string &output_file):output_file_(output_file)
{
    spdlog::info("QueryHandler initialized: output_file={}", output_file_);
}

QueryHandler::~QueryHandler()
{
    close();
}

bool QueryHandler::open()
{
    file_stream_.open(output_file_,std::ios::out | std::ios::trunc);

    if(!file_stream_.is_open()){
        spdlog::error("Failed to open output file: {}", output_file_);
        return false;
    }
    spdlog::info("Output file opened successfully: {}", output_file_);
    return true;
}

void QueryHandler::close()
{
    if(file_stream_.is_open()){
        file_stream_.close();
        spdlog::info("Output file closed");
    }
}

void QueryHandler::outputTopK(unsigned int timestamp, const std::vector<std::pair<std::string, int>> &topk)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(output_mutex_);

    if(!file_stream_.is_open())open();
    
    std::string time_str=formatTimestamp(timestamp);

    file_stream_<<time_str<<" Top-"<<topk.size()<<":"<<std::endl;

    for(size_t i=0;i<topk.size();i++){
        file_stream_<<(i+1)<<". "<<topk[i].first<<" (出现"<<topk[i].second<<"次)"<<std::endl;
    }

    file_stream_<<std::endl;
    file_stream_.flush();//刷新磁盘，强制写入磁盘

    //输出延迟
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration<double, std::milli>(
        end_time - start_time).count();
    
    auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},output_write_ms,{:.3f}", std::time(nullptr), duration_ms);
    }
    
    spdlog::debug("Output written in {:.3f}ms", duration_ms);
}

std::string QueryHandler::formatTimestamp(unsigned int seconds)
{
    unsigned int h=seconds/3600;
    unsigned int m=(seconds%3600)/60;
    unsigned int s=seconds%60;

    std::ostringstream oss;

    oss<<std::setfill('0')<<'['<<std::setw(2)<<h<<":"<<std::setw(2)<<m<<":"<<std::setw(2)<<s<<']';

    return oss.str();
}
