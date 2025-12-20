#include "SlidingWindow.h"
#include <algorithm>
#include <cmath>
#include "spdlog/spdlog.h"

SlidingWindow::SlidingWindow(unsigned int window_size):window_size_(window_size){
    spdlog::info("=== SlidingWindow Initialized ===");
    spdlog::info("Window size: {} seconds ({} minutes)", 
                 window_size_, window_size_ / 60);
}

void SlidingWindow::addData(const TimeSlot &data)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    std::lock_guard<std::mutex> lock(mutex_);

    curtime = std::max(curtime, data.timestamp);
    unsigned int ts=data.timestamp;

    if (ts < curtime) {
        spdlog::warn("Late data detected! data_time={}, current_time={}, lag={}s", 
                     ts, curtime, curtime - ts);
    }
    // 累加当前时间槽中的词频
    for (const auto& word : data.words) {
    ++word_count_[word];
    }
    // 建立时间索引（同一时间戳的词一起处理）
    if (time_index_.find(ts) == time_index_.end()) {
        time_index_[ts] = data.words;
    } else {
        // 相同时间戳的词追加到已有列表
        time_index_[ts].insert(time_index_[ts].end(),data.words.begin(),data.words.end());
    }

    // 淘汰过期数据（以 curtime 为基准）
    evictExpiredData(curtime);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},window_adddata_ms,{:.3f}", 
                         std::time(nullptr), duration_ms);
    }
    
    // 如果耗时过长，发出警告
    if (duration_ms > 50.0) {
        spdlog::warn("Slow window update: {:.2f}ms (threshold: 50ms)", duration_ms);
    }
    
}

//查询时要先同步一下窗口
std::vector<std::pair<std::string, int>> SlidingWindow::getTopK(int k) const
{
    auto start_time = std::chrono::high_resolution_clock::now();

    std::lock_guard<std::mutex> lock(mutex_);
    // 【异常处理】检查 K 值合法性
    if (k <= 0) {
        spdlog::warn("Invalid K value: {}, reset to default K=10", k);
        k = 1;
    }

    std::vector<std::pair<std::string, int>> result;
    result.reserve(word_count_.size());

    for (const auto& kv : word_count_) {
    result.emplace_back(kv.first, kv.second);
    }

    std::sort(result.begin(), result.end(),
    [](const auto& a, const auto& b) {
    return a.second > b.second;
    });


    if (static_cast<int>(result.size()) > k) {
    result.resize(k);
    }

     auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    auto perf_logger = spdlog::get("perf");
    if (perf_logger) {
        perf_logger->info("{},topk_query_ms,{:.3f}", std::time(nullptr), duration_ms);
        perf_logger->info("{},topk_k_value,{}", std::time(nullptr), k);
    }
    
    spdlog::debug("Top-K query completed in {:.3f}ms", duration_ms);

    return result;
}

int SlidingWindow::getWordCount(const std::string &word) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = word_count_.find(word);
    return (it != word_count_.end()) ? it->second : 0;
}

size_t SlidingWindow::getTotalWords() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = 0;
    for (const auto& kv : word_count_) {
        total += kv.second;
    }
    return total;
}

size_t SlidingWindow::getUniqueWords() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return word_count_.size();return size_t();
}

unsigned int SlidingWindow::currentTime() const
{
    return curtime;
}

void SlidingWindow::evictExpiredData(unsigned int curtime)
{
    unsigned int expire_time = (curtime > window_size_)? (curtime - window_size_): 0;


    // map 是按时间戳升序排列的，方便从最早时间开始淘汰
    auto it = time_index_.begin();
    while (it != time_index_.end() && it->first < expire_time) {
        // 对该时间槽内的每一个词进行词频递减
        for (const auto& word : it->second) {
            decrementWord(word);
        }
        it = time_index_.erase(it);
    }
}

void SlidingWindow::decrementWord(const std::string &word)
{
    auto it = word_count_.find(word);
    if (it != word_count_.end()) {
        if (it->second > 50) {
            spdlog::debug("Evicting word: '{}' (frequency was: {})", word, it->second);
        }
        if (--(it->second) == 0) {
            word_count_.erase(it);
        }
    }
}

size_t SlidingWindow::estimateMemoryUsage() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t memory = 0;
    
    // word_count_ 的内存
    memory += word_count_.size() * (50 + sizeof(int));
    
    // time_index_ 的内存
    for (const auto& kv : time_index_) {
        memory += sizeof(unsigned int);
        memory += kv.second.size() * 50;
    }
    
    return memory;
}