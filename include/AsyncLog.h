#ifndef _ASYNCLOG_
#define _ASYNCLOG_

#include <filesystem>
#include <string_view>
#include <memory>

namespace doggy 
{

class AsyncLogImpl;

// AsyncLog类的功能是接收日志输入, 在后台线程中将日志输出到文件中
class AsyncLog final
{
public:
    // dir_path : 日志文件的输出目录
    // flush_interval_s : 将内存缓存区中日志强制flush到文件的最大间隔秒数
    AsyncLog(const std::filesystem::path& dir_path, int flush_interval_s = 3);

    ~AsyncLog();
    // 不允许拷贝
    AsyncLog(const AsyncLog&) = delete;
    AsyncLog& operator=(const AsyncLog&) = delete;

    void Append(const std::string_view logline);
    void Stop();
    
private:
    std::unique_ptr<AsyncLogImpl> impl_;
};

} // namespace doggy end

#endif