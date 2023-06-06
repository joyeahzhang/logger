#include "include/Logger.h"
#include "include/LogStream.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <chrono>


namespace doggy 
{

class LoggerImpl
{
public:
    LoggerImpl(LogLevel level, SourceFile file, long line);
    void Finish();

    LogLevel level_;
    SourceFile basename_;
    int line_;
    LogStream stream_;  
};

std::string LogLevelString[]
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

void DefaultOutput(std::string_view logs)
{
    std::cout << logs;
}

void DefaultFlush()
{
    std::fflush(stdout);
}

// 设置默认的日志输出函数
Logger::OutputFunc Logger::output_func_= DefaultOutput;
Logger::FlushFunc Logger::flush_func_ = DefaultFlush;
LogLevel Logger::output_level_ = LogLevel::TRACE;

} // end namespace doggy

using namespace doggy;


Logger::Logger(SourceFile file, int line):impl_(std::make_unique<LoggerImpl>(LogLevel::TRACE,file,line))
{

}

Logger::Logger(SourceFile file, int line, LogLevel level):impl_(std::make_unique<LoggerImpl>(level,file,line))
{

}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func):impl_(std::make_unique<LoggerImpl>(level,file,line))
{
    impl_->stream_ << func << "(): ";
}

Logger::Logger(SourceFile file, int line, bool abort)
    : impl_(std::make_unique<LoggerImpl>( (abort ? LogLevel::FATAL : LogLevel::ERROR) , file, line))
{

}

Logger::Logger(Logger&&) = default;
Logger& Logger::operator=(Logger&&) = default;

Logger::~Logger()
{
    impl_->Finish();
    output_func_(impl_->stream_.ToStringView());
    if(impl_->level_ == LogLevel::FATAL)
    {
        flush_func_();
        abort();
    }
}

LogStream& Logger::Stream() const
{
    return impl_->stream_;
}

void Logger::SetOutput(const OutputFunc& func)
{
    output_func_ = func;
}

void Logger::SetFlush(const FlushFunc& func)
{
    flush_func_ = func;
}

void Logger::SetOutputLogLevel(LogLevel level)
{
    output_level_ = level;
}

LogLevel Logger::GetOutputLogLevel()
{
    return output_level_;
}



LoggerImpl::LoggerImpl(LogLevel level, SourceFile file, long line)
    :   level_(level),
        basename_(file),
        line_(line),
        stream_(LogStream()) 
{
    stream_ << "[" << LogLevelString[int(level_)] << "]";

    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(std::move(now));
    auto tm = std::localtime(&tt);
    char time[128];
    sprintf(time, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    stream_<<time;

    stream_ << " "<<file.ToStringView()<<":" << line_ << " ";
}

void LoggerImpl::Finish()
{
    stream_ << "\n";
}
