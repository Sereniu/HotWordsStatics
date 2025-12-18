#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include "Common.h"
#include <fstream>
#include <sstream>
#include <mutex>
#include <iomanip>

class QueryHandler {
private:
    std::string output_file_;
    std::ofstream file_stream_;
    mutable std::mutex output_mutex_;
    
public:
    QueryHandler(const std::string& output_file = "output.txt");
    ~QueryHandler();
    
    bool open();
    void close();

    /**
     * 输出 Top-K 结果到文件
     * @param timestamp 查询时刻的时间戳（秒）
     * @param topk Top-K 词频列表（词 + 频次）
     */
    void outputTopK(unsigned int timestamp, 
                    const std::vector<std::pair<std::string, int>>& topk);
    
private:
    //将时间戳（秒）格式化为 [HH:MM:SS]
    std::string formatTimestamp(unsigned int seconds);
};

#endif 