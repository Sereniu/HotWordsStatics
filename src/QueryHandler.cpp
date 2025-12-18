#include "QueryHandler.h"
#include <iostream>

QueryHandler::QueryHandler(const std::string &output_file):output_file_(output_file)
{
}

QueryHandler::~QueryHandler()
{
    close();
}

bool QueryHandler::open()
{
    file_stream_.open(output_file_,std::ios::out | std::ios::trunc);

    if(!file_stream_.is_open()){
        std::cerr<<"[QueryHandler] [错误] 无法打开输出文件："<<output_file_<<std::endl;
        return false;
    }
    std::cout<<"[QueryHandler] [信息] 输出文件打开成功："<<output_file_<<std::endl;
    return true;
}

void QueryHandler::close()
{
    if(file_stream_.is_open()){
        file_stream_.close();
        std::cout<<"[QueryHandler] [信息] 输出文件已关闭"<<std::endl;
    }
}

void QueryHandler::outputTopK(unsigned int timestamp, const std::vector<std::pair<std::string, int>> &topk)
{
    std::lock_guard<std::mutex> lock(output_mutex_);

    if(!file_stream_.is_open())open();
    
    std::string time_str=formatTimestamp(timestamp);

    file_stream_<<time_str<<" Top-"<<topk.size()<<":"<<std::endl;

    for(size_t i=0;i<topk.size();i++){
        file_stream_<<(i+1)<<". "<<topk[i].first<<" (出现"<<topk[i].second<<"次)"<<std::endl;
    }

    file_stream_<<std::endl;
    file_stream_.flush();//刷新磁盘，强制写入磁盘
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
