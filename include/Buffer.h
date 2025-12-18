// 循环缓冲区（模板类）
#ifndef BUFFER_H
#define BUFFER_H

#include "Common.h"
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>


template<typename T>
class Buffer {
private:
    std::vector<T> buffer_;//缓存区
    size_t capacity_;//最大容量
    size_t low_watermark_;//剩余数据量阈值
    
    size_t read_pos_;//读位置（consumer）
    size_t write_pos_;//写位置（生产者）
    size_t count_;//buffer中当前数据量

    bool allow_write_;
    
    mutable std::mutex mutex_;
    std::condition_variable refill_cv_;
    std::condition_variable not_empty_;
    
    std::atomic<bool> closed_;
    std::atomic<bool> input_finished_;
    
public:
    Buffer(size_t capacity, size_t low_watermark)
        : buffer_(capacity), capacity_(capacity), 
          low_watermark_(low_watermark),
          read_pos_(0), write_pos_(0), count_(0),allow_write_(true),
          closed_(false), input_finished_(false) {}
    
    bool push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        refill_cv_.wait(lock, [this] {
            return allow_write_ || closed_.load();
        });
        
        if (closed_.load()) return false;
        
        buffer_[write_pos_] = std::move(item);
        write_pos_ = (write_pos_ + 1) % capacity_;
        count_++;

        if(count_==capacity_){
            allow_write_=false;
        }
        
        not_empty_.notify_one();
        
        return true;
    }
    
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        not_empty_.wait(lock, [this] {
            return count_ > 0 || closed_.load() || input_finished_.load();
        });
        
        if (count_ == 0 && (input_finished_.load()|| closed_.load())) {
            return false;
        }
        
        item = std::move(buffer_[read_pos_]);
        read_pos_ = (read_pos_ + 1) % capacity_;
        count_--;
        
        if(!allow_write_ && count_<=low_watermark_){
            allow_write_=true;
            refill_cv_.notify_all();//生产者
        }
        
        return true;
    }
    
    void markInputFinished() {
        input_finished_.store(true);
        not_empty_.notify_all();
    }
    
    void close() {
        closed_.store(true);
        refill_cv_.notify_all();
        not_empty_.notify_all();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }
    
    size_t available() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return capacity_ - count_;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_ == 0;
    }
};

#endif // BUFFER_H