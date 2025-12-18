#ifndef SLIDINGWINDOW_H
#define SLIDINGWINDOW_H

#include "Common.h"
#include <unordered_map>
#include <map>
#include <mutex>


class SlidingWindow {
private:
    std::unordered_map<std::string, int> word_count_;
    std::map<unsigned int, std::vector<std::string>> time_index_;
    unsigned int window_size_;
    unsigned int curtime=0;
    mutable std::mutex mutex_;
    
public:

    /**
    * 构造函数
    * @param window_size 滑动窗口大小（秒），默认 600 秒（10 分钟）
    */
    explicit SlidingWindow(unsigned int window_size = 600);
    
    /**
    * 向滑动窗口中加入一个时间槽的数据
    *
    * 逻辑说明：
    * 1. 将当前 TimeSlot 中的所有词加入词频统计表 word_count_
    * 2. 在 time_index_ 中记录该时间戳对应的词列表
    * 3. 根据当前时间戳，淘汰窗口外（超过 window_size）的旧数据
    *
    * @param data 已完成分词的时间槽（时间戳 + 词列表）
    */
    void addData(const TimeSlot& data);

    /**
    * 获取当前窗口内 Top-K 高频词
    *
    * 实现方式：
    * - 将 unordered_map 转为 vector
    * - 按词频降序排序
    * - 取前 K 个
    *
    * @param k Top-K 中的 K 值
    * @return 词频对 (word, count) 的列表
    */
    std::vector<std::pair<std::string, int>> getTopK(int k) const;
    
    /**
    * 获取某个词在当前窗口内的出现次数
    */
    int getWordCount(const std::string& word) const;

    /**
    * 获取窗口内的总词数（含重复）
    */
    size_t getTotalWords() const;

    /**
    * 获取窗口内的不同词数量
    */
    size_t getUniqueWords() const;

    unsigned int currentTime() const;
    
private:

    /**
    * 淘汰滑动窗口外的过期数据
    *
    * 规则：
    * - 若某个时间戳 < current_time - window_size_
    * - 则该时间槽内的所有词都需要从 word_count_ 中减去
    *
    * @param curtime 当前最新数据的时间戳
    */
    void evictExpiredData(unsigned int curtime);

    /**
    * 对某个词进行词频递减，若减到 0 则删除
    */
    void decrementWord(const std::string& word);
};

#endif 