#include "SlidingWindow.h"
#include <algorithm>
#include <cmath>

SlidingWindow::SlidingWindow(unsigned int window_size):window_size_(window_size){}

void SlidingWindow::addData(const TimeSlot &data)
{
    std::lock_guard<std::mutex> lock(mutex_);

    curtime = std::max(curtime, data.timestamp);
    unsigned int ts=data.timestamp;

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
    
}

//查询时要先同步一下窗口
std::vector<std::pair<std::string, int>> SlidingWindow::getTopK(int k) const
{
    std::lock_guard<std::mutex> lock(mutex_);

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
        if (--(it->second) == 0) {
            word_count_.erase(it);
        }
    }
}
