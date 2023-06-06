#ifndef _LOGGER_
#define _LOGGER_

#include <functional>
#include <memory>
#include <string_view>


namespace doggy {

class LoggerImpl;
class LogStream;

enum class LogLevel
{
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3, 
    ERROR = 4,
    FATAL = 5,
    NUM_LOG_LEVELS,
};

// SourceFile类负责将__FILE__宏转换成“仅包含文件名”的string_view
// SourceFile类不持有任何资源, 它必须在__FILE__有效时使用
class SourceFile
{
public:
    template<int SIZE>
    SourceFile(const char (&file)[SIZE])
    {
        std::string_view file_sv(file);
        auto last = file_sv.find_last_of('/');
        if(last == std::string_view::npos)
        {
            basename_ = file_sv;
        }
        else 
        {
            basename_ = file_sv.substr(last+1);
        }
    }
    std::string_view ToStringView(){ return basename_; }
private:
    std::string_view basename_; // 不包含上级路径的纯文件名
};

class Logger
{
public:
    // SourceFile是对string_view的简单包装,占用空间很小,按值传递可能更加好
    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool abort);
    
    Logger(Logger&&);
    Logger& operator=(Logger&&);
    
    ~Logger();

    LogStream& Stream() const; 

    using OutputFunc = std::function<void(std::string_view)>;
    using FlushFunc = std::function<void()>  ;
    
    static void SetOutputLogLevel(LogLevel level);
    static void SetOutput(const OutputFunc&);
    static void SetFlush(const FlushFunc&);
    static LogLevel GetOutputLogLevel();

    static LogLevel output_level_;
    static OutputFunc output_func_;
    static FlushFunc flush_func_;

private:
    std::unique_ptr<LoggerImpl> impl_;
};


#define LOG_TRACE if (doggy::Logger::GetOutputLogLevel() <= doggy::LogLevel::TRACE) doggy::Logger(__FILE__, __LINE__, doggy::LogLevel::TRACE, __func__).Stream()
#define LOG_DEBUG if (doggy::Logger::GetOutputLogLevel() <= doggy::LogLevel::DEBUG) doggy::Logger(__FILE__, __LINE__, doggy::LogLevel::DEBUG, __func__).Stream()
#define LOG_INFO if (doggy::Logger::GetOutputLogLevel() <= doggy::LogLevel::INFO) doggy::Logger(__FILE__, __LINE__, doggy::LogLevel::INFO, __func__).Stream()
#define LOG_WARN doggy::Logger(__FILE__, __LINE__, doggy::LogLevel::WARN).Stream();
#define LOG_ERROR doggy::Logger(__FILE__, __LINE__, doggy::LogLevel::ERROR).Stream();
#define LOG_FATAL doggy::Logger(__FILE__, __LINE__, doggy::LogLevel::FATAL).Stream();
#define LOG_SYSERR doggy::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL doggy::Logger(__FILE__, __LINE__, true).stream()

} //end namespace doggy

#endif