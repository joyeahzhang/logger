#ifndef _LOGSTREAM_
#define _LOGSTREAM_

#include <memory>
#include <string_view>

namespace doggy {

template <int SIZE>
class FixedBuffer;

inline constexpr size_t DEFAULT_LOGSTREAM_BUFFER_SIZE = 1024;

// LogStream类负责接收日志流, 它的设计参考了std::ostream, 表现在：
//  1. 它对各种类型的数据类型重载了“<<运算符”
//  2. 它拥有一个缓冲区, 用于接收“<<运算符”输入的数据

// 不过LogStream类没有负责将缓冲区内容输出到目的地的成员函数, 而仅有
// 一个ToStreamView函数负责以string_view的格式读出缓冲区中的数据。
// 因为LogStream类一定是由Logger类所持有的, 所以在设计中我将输出缓冲区中
// 内容到目的地的任务放到了Logger类中。

class LogStream final
{    
public:
    LogStream();
    // 流对象不允许拷贝
    LogStream(const LogStream&)=delete;
    LogStream& operator=(const LogStream&)=delete;

    LogStream(LogStream&&);
    LogStream& operator=(LogStream&&);

    ~LogStream();

    LogStream& operator<<(bool b);
    LogStream& operator<<(char c);
    LogStream& operator<<(short s);
    LogStream& operator<<(unsigned short us);
    LogStream& operator<<(int i);
    LogStream& operator<<(unsigned int ui);
    LogStream& operator<<(long l);
    LogStream& operator<<(unsigned long ul);
    LogStream& operator<<(long long ll);
    LogStream& operator<<(unsigned long long ull);
    LogStream& operator<<(float f);
    LogStream& operator<<(double d);
    LogStream& operator<<(const void* p);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const unsigned char* str);
    LogStream& operator<<(std::string_view sv);
    LogStream& operator<<( const FixedBuffer<DEFAULT_LOGSTREAM_BUFFER_SIZE>& buffer);

    typedef FixedBuffer<DEFAULT_LOGSTREAM_BUFFER_SIZE> Buffer ;
    
    std::string_view ToStringView() const;

private:
    template<typename T>
    void formatInteger(T value);
private:
    std::unique_ptr<Buffer> buffer_;
};

} //end namespace doggy
# endif