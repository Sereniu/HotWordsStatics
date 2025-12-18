// 公共数据结构定义（纯结构体，无类）
#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * 带时间戳的时间槽
 */
struct TimeSlot {
    unsigned int timestamp;             // 时间戳（秒）
    std::vector<std::string> words={}; // 该时刻的所有词
    
    TimeSlot(unsigned int ts = 0) : timestamp(ts) {}
};

/**
 * 查询命令
 */
struct QueryCommand {
    unsigned int timestamp;  // 查询时刻的时间戳
    int k;               // Top-K 的 K 值
    
    QueryCommand(unsigned int ts = 0, int k_val = 10) 
        : timestamp(ts), k(k_val) {}
};

#endif // COMMON_H