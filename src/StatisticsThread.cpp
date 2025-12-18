#include "StatisticsThread.h"

void StatisticsThread::run()
{
    while(true){
        TimeSlot slot;
        if(!buffer_.pop(slot)){
            break;
        }

        //更新滑动窗口
        sliding_window_.addData(slot);
        //执行查询
        processQueries();
    }
}

void StatisticsThread::processQueries()
{
    std::lock_guard<std::mutex> lock(query_mutex_);

    unsigned int ts = sliding_window_.currentTime();

    while(!query_queue_.empty()){
        QueryCommand query = query_queue_.front();

        if(query.timestamp <= ts){
            auto topk = sliding_window_.getTopK(query.k);

            // ⭐ 使用滑动窗口时间
            query_handler_.outputTopK(ts, topk);

            query_queue_.pop();   // 只执行一次
        } else {
            break;
        }
    }
}

StatisticsThread::StatisticsThread(int thread_id, Buffer<TimeSlot> &buffer, SlidingWindow &sliding_window, QueryHandler &query_handler, std::queue<QueryCommand> &query_queue, std::mutex &query_mutex)
: thread_id_(thread_id),
      buffer_(buffer),
      sliding_window_(sliding_window),
      query_handler_(query_handler),
      query_queue_(query_queue),
      query_mutex_(query_mutex)
{
}
