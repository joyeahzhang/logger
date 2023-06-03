#ifndef _LOGSTREAM_
#define _LOGSTREAM_

#include <memory>
#include <ostream>
#include <string_view>

namespace doggy {

template <int SIZE>
class FixedBuffer;

inline constexpr size_t DEFAULT_LOGSTREAM_BUFFER_SIZE = 1024;

class LogStream final //不可继承 && 不可拷贝
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