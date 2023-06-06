#include "include/AsyncLog.h"
#include "include/FixedBuffer.h"
#include "include/LogFile.h"

#include <condition_variable>
#include <iostream>
#include <thread>
#include <filesystem>
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>
#include <sys/prctl.h>


namespace doggy
{

// 使用constexpr完成编译时计算
constexpr size_t ASYNC_LOG_BUFFER_SIZE = 4000*1024;

class AsyncLogImpl
{
public:
    AsyncLogImpl(const std::filesystem::path& dir_path, int flush_interval_s)
        : running_(true),
          dir_path_(dir_path),
          thread_(&AsyncLogImpl::DoBackgroundWork,this),
          notified(false),
          flush_interval_s_(flush_interval_s),
          cur_buf_(std::make_unique<Buffer>()),
          next_buf_(std::make_unique<Buffer>())
    {}

    // 标识AsyncLog是否正在运行
    std::atomic<bool> running_;
    // 日志文件的输出目录
    const std::filesystem::path dir_path_; 
    // 负责将缓存区中日志内容写入文件的后台线程
    std::thread thread_;
    // 同步相关设施
    std::mutex cv_m_;
    std::condition_variable cv_;
    bool notified;

    const std::chrono::seconds flush_interval_s_;
    
    // 缓存区
    typedef FixedBuffer<ASYNC_LOG_BUFFER_SIZE> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;

    BufferPtr cur_buf_;
    BufferPtr next_buf_;
    std::vector<BufferPtr> buffers_;

private:
    void DoBackgroundWork();
};

} // end namespace doggy

using namespace doggy;

AsyncLog::AsyncLog(const std::filesystem::path& dir_path, int flush_interval_s)
    : impl_(std::make_unique<AsyncLogImpl>(dir_path, flush_interval_s))
{
    // do nothing
}

AsyncLog::~AsyncLog()
{
    Stop();
}

void AsyncLog::Stop()
{
    impl_->running_.store(false);

    if(impl_->thread_.joinable())
    {
        impl_->thread_.join();
    }
}

void AsyncLog::Append(const std::string_view logline)
{
    // 临界区
    std::lock_guard<std::mutex> lg(impl_->cv_m_);
    // 如果当前缓冲区还足够使用,则直接将日志内容拷贝到当前缓冲区
    // 不主动唤醒异步写日志的后台线程
    if(impl_->cur_buf_->Avail() >= logline.size())
    {
        impl_->cur_buf_->Append(logline);
    }
    else
    {
        impl_->buffers_.emplace_back(std::move(impl_->cur_buf_));
        if(impl_->next_buf_)
        {
            impl_->cur_buf_ = std::move(impl_->next_buf_);
        }
        else 
        {
            impl_->cur_buf_ = std::make_unique<AsyncLogImpl::Buffer>();
        }
        impl_->cur_buf_->Append(logline);
        impl_->notified = true;
        impl_->cv_.notify_one();
    }
}

void AsyncLogImpl::DoBackgroundWork()
{
    // 设置后台线程名字, 方便分线程观测程序的运行情况
    prctl(PR_SET_NAME, "AsyncLog", 0, 0, 0);
    
    BufferPtr buf_1 = std::make_unique<Buffer>();
    BufferPtr buf_2 = std::make_unique<Buffer>();
    buf_1->Bzero();
    buf_2->Bzero();

    // 线程内缓存区
    std::vector<BufferPtr> buffers_to_write;

    while(running_)
    {
        {// critical section: 互斥访问condition variable和前台线程缓存区
            auto timeout = flush_interval_s_;
            std::unique_lock<std::mutex> lk(cv_m_);
            cv_.wait_for(lk, timeout, [this]{return notified;});
            
            notified = false;

            buffers_.emplace_back(std::move(cur_buf_));     
            cur_buf_ = std::move(buf_1);
            
            std::swap(buffers_to_write,buffers_);
            
            if(!next_buf_)
            {
                next_buf_ = std::move(buf_2);
            }
            
        }// critical section end

        auto output = doggy::LogFile::Create(dir_path_, false);

        // 日志生产速度远大于日志消费速度,产生日志堆积
        // 为避免日志缓存占用过多内存,直接抛弃部分日志
        if(buffers_to_write.size() > 25)
        {
            char buf[256];
            // FIXME:时间函数补充
            std::snprintf(buf,sizeof(buf),"Dropped log message at %s, %zd larger buffers\n",
                "0", buffers_to_write.size()-2);
            std::fputs(buf,stderr);
            output->Append(buf);
            buffers_to_write.erase(buffers_to_write.begin()+2, buffers_to_write.end());
        }
        // 输出日志到文件
        for(const auto& buffer : buffers_to_write)
        {
            output->Append(buffer->ToStringView());
        }
        if(buffers_to_write.size() > 2)
        {
            buffers_to_write.resize(2);
        }

        buf_1 = std::move(buffers_to_write[0]);
        buf_1->Clear();
        // 如果buf_2被move to next_buf, 那么buffers_to_write中至少有两个buffer
        if(!buf_2)
        {
            buf_2 = std::move(buffers_to_write[1]);
            buf_2->Clear();
        }
        buffers_to_write.clear();
        output->Flush();
    }
    // 在DoBackgroundWork结束时, 冲洗掉前台线程缓存在的日志
    for(const auto& buffer : buffers_)
    {
        LogFile::Create(dir_path_,false)->Append(buffer->ToStringView());
    }
    LogFile::Create(dir_path_,false)->Flush();
}