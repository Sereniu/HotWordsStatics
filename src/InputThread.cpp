#include "InputThread.h"
#include <iostream>

InputThread::InputThread(const std::string &input_file, Buffer<TimeSlot> &buffer, std::queue<QueryCommand> &query_queue, std::mutex &query_mutex, std::atomic<bool> &running, size_t batch_size):
    buffer_(buffer),
    query_queue_(query_queue),
    query_mutex_(query_mutex),
    running_(running),
    batch_size_(batch_size)
{
    input_handler_=std::make_unique<InputHandler>(input_file);
    text_processor_ = std::make_unique<TextProcessor>("../dict/", true);//如果是为了测试可以临时改为../../dict,正式运行为../dict

    std::cout<<"[InputThread] [信息] 初始化成功"<<std::endl;
}

void InputThread::run()
{
    std::cout << "[InputThread] [信息]开始运行" << std::endl;

    if (!input_handler_->open()) {
        std::cerr << "[InputThread] [错误]无法打开输入文件" << std::endl;
        buffer_.markInputFinished();//缓冲区结束输入
        running_.store(false);//结束进程
        return;
    }

    //批次缓存,提高吞吐量
    std::vector<TimeSlot> batch;
    batch.reserve(batch_size_);

    //统计信息，便于调试（日志）
    size_t total_lines = 0;
    size_t text_lines = 0;
    size_t query_lines = 0;
    size_t total_words = 0;

    //数据的变量
    unsigned int timestamp;
    std::string text;
    bool is_query;
    int k;

    while(running_.load() && !input_handler_->eof()){
        //读取一行
        if(!input_handler_->readLine(timestamp,text,is_query,k)){
            continue;
        }

        total_lines++;

        if(is_query){
            query_lines++;

            //在锁中调用而后释放
            {
                std::lock_guard<std::mutex> lock(query_mutex_);
                query_queue_.push(QueryCommand(timestamp, k));
            }

            // std::cout << "[InputThread] [信息]添加查询 [时间=" 
            //           << timestamp << ", K=" << k << "]" << std::endl;
            continue;
        }

        text_lines++;

        TimeSlot slot(timestamp);
        slot.words=text_processor_->processWithPOS(text);

        total_words+=slot.words.size();

        if(!slot.words.empty()){
            batch.push_back(std::move(slot));
        }

        if(batch.size()>=batch_size_){
            bool success=true;
            for(auto& item:batch){
                if(!buffer_.push(std::move(item))){
                    std::cerr << "[InputThread] [警告]Buffer 已关闭，停止写入" << std::endl;
                    success=false;
                    break;
                }
            }
            batch.clear();

            if(!success){
                break;
            }
        }
    }

    //清理+日志
    std::cout << "[InputThread] [信息]提交剩余 " << batch.size() << " 条数据" << std::endl;
    for (auto& item : batch) {
        if (!buffer_.push(std::move(item))) {
        std::cerr << "[InputThread] [警告]提交剩余数据时 Buffer 已关闭" << std::endl;
        break;
        }   
    }
    buffer_.markInputFinished();
    input_handler_->close();
    
    std::cout << "\n========== InputThread 统计信息 ==========" << std::endl;
    std::cout << "总行数:   " << total_lines << std::endl;
    std::cout << "文本行:   " << text_lines << std::endl;
    std::cout << "查询行:   " << query_lines << std::endl;
    std::cout << "总词数:   " << total_words << std::endl;
    std::cout << "平均词数: " << (text_lines > 0 ? (double)total_words / text_lines : 0) << std::endl;
    std::cout << "==========================================" << std::endl;
    
    std::cout << "[InputThread] [信息]退出" << std::endl;
}