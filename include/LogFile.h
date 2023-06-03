#ifndef _LOGFILE_
#define _LOGFILE_

#include <memory>
#include <mutex>
#include <string_view>
#include <filesystem>
#include <fstream>

namespace doggy 
{

constexpr int MAX_LOGFILE_SIZE = 1024*1024*500;

class LogFile : std::enable_shared_from_this<LogFile>
{
public:


    static std::shared_ptr<LogFile> Create(const std::filesystem::path& dir_path=std::filesystem::current_path(), bool thread_safe = false);

    LogFile(const std::filesystem::path& dir_path=std::filesystem::current_path(), bool threadsafe=false);
    
    ~LogFile();
    // 不允许拷贝
    LogFile(const LogFile&)=delete;
    LogFile& operator=(const LogFile&)=delete;

    void Append(std::string_view logs);
    void Flush();

private:
    void append(std::string_view logs);
private:
    std::filesystem::path file_path_;
    bool thread_safe_;
    std::mutex file_mutex_;
    std::fstream out_;  

    static std::shared_ptr<LogFile> current_file_;
    static std::string current_file_name_;
    static size_t current_suffix_;
    static size_t current_file_size_;
};

} // end namespace doggy

#endif