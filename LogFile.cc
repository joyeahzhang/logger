#include "include/LogFile.h"

#include <ctime>
#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <fstream>
#include <string_view>


namespace doggy::detail 
{
    std::string GetCurrentHourTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        auto now_tm = *std::localtime(&now_t);

        char buffer[32];
        std::strftime(buffer, sizeof(buffer), "%Y%m%d%H", &now_tm);
        return std::string(buffer);
    }
}


namespace doggy 
{
    std::shared_ptr<LogFile> LogFile::current_file_ = nullptr;
    std::string LogFile::current_file_name_{};
    size_t LogFile::current_suffix_ = 0;
    size_t LogFile::current_file_size_ = 0;
}

using namespace doggy;
using namespace std;




shared_ptr<LogFile> LogFile::Create(const std::filesystem::path& dir_path, bool threadsafe)
{
    string now_h = detail::GetCurrentHourTimestamp();
    // 如果当前小时还没有创建过日志文件, 则以当前时刻为文件名创建日志文件
    if(current_file_name_.empty() || string_view(current_file_name_).substr(0,13) != now_h)
    {
        current_file_name_ = now_h;
        current_suffix_ = 0;
        current_file_size_ = 0;
        
        auto file_path = dir_path / (current_file_name_ + "-" + to_string(current_suffix_) + ".log");
        current_file_ = make_shared<LogFile>(file_path, threadsafe);
    }
    // 如果当前文件已经达到最大容量, 则为当前小时再创建一个日志文件, 并以递增的后缀区分
    else if(current_file_size_ > MAX_LOGFILE_SIZE)
    {
        ++current_suffix_;
        auto file_path = dir_path / (current_file_name_ + "-" + to_string(current_suffix_) + ".log");
        current_file_size_ = 0;
        current_file_ = make_shared<LogFile>(file_path, threadsafe);
    }
    return current_file_;
}


LogFile::LogFile(const std::filesystem::path& file_path, bool threadsafe)
    : file_path_(file_path),
      thread_safe_(threadsafe)
{

    if(auto dir_path = file_path.parent_path(); !std::filesystem::exists(file_path.parent_path()))
    {   
        std::filesystem::create_directories(dir_path);
    }
    out_.open(file_path,std::fstream::out | std::fstream::app);
}

LogFile::~LogFile()
{
    out_.flush();
    out_.close();
}

void LogFile::Append(std::string_view logs)
{
    if(thread_safe_)
    {
        std::lock_guard<std::mutex> lg(file_mutex_);
        append(logs);
    }
    else
    {
        append(logs);
    }
}

void LogFile::Flush()
{
    out_.flush();
}

void LogFile::append(std::string_view logs)
{
    current_file_size_ += logs.size();
    out_ << logs;
}