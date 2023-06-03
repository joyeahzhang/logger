#include "include/FixedBuffer.h"
#include "include/LogStream.h"

#include <cstddef>
#include <memory>
#include <ostream>
#include <string_view>


namespace doggy::detail 
{
constexpr int KMAX_NUMERIC_SIZE = 48;
constexpr char digits[] = "9876543210123456789";
constexpr char digitsHex[] = "0123456789ABCDEF";
const char* zero = digits + 9;

// 将数字转换成为char[]
template <typename T>
std::size_t Convert(T value, char buffer[])
{
    auto i = value;
    char* p = buffer;

    do
    {
        auto lsd = i % 10; // 取value最低位
        i /= 10; // value抛弃最低位
        *p++ = zero[lsd]; // 将lsd转换成字符,插入到buffer的低位
    }while(i);

    //负数需要添加负号
    if(value < 0)
    {
        *p++ = '-';
    }
    // 添加C-style string结束符
    *p = '\0';

    std::reverse(buffer,p);

    return p-buffer;
}

size_t Convert2Hex(uintptr_t value, char *buffer)
{
    uintptr_t i = value;
    char* p = buffer;
    do 
    {
        auto lsd = i%16;
        i/=16;
        *p++ = digitsHex[lsd];
    }while (i);
    *p = '\0'; // 为什么在reverse之前加上'\0'
    std::reverse(buffer,p);
    return p-buffer;
}
}//end namespace doggy::digit


using namespace doggy;
using namespace doggy::detail;

LogStream::LogStream() : buffer_(std::make_unique<LogStream::Buffer>())
{

}

LogStream::LogStream(LogStream&&) = default;
LogStream& LogStream::operator=(LogStream&&) = default;

LogStream::~LogStream() = default;

template<typename T>
void LogStream::formatInteger(T value)
{
    if(buffer_->Avail() >= KMAX_NUMERIC_SIZE)
    {
        char tmp_buf[KMAX_NUMERIC_SIZE];
        auto len = Convert(value, tmp_buf);
        buffer_->Append(std::string_view(tmp_buf, len));
    }
}

LogStream& LogStream::operator<<(bool b)
{
    // 将bool值转换成字符串加入缓存区
    buffer_->Append(b ? "1" : "0");
    return *this;
}

LogStream& LogStream::operator<<(char c)
{
    buffer_->Append(&c);
    return *this;
}

LogStream& LogStream::operator<<(short s)
{
    *this << static_cast<int>(s);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short us)
{
    *this << static_cast<unsigned>(us);
    return *this;
}

LogStream& LogStream::operator<<(int i)
{
    formatInteger(i);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int ui)
{
    formatInteger(ui);
    return *this;
}

LogStream& LogStream::operator<<(long l)
{
    formatInteger(l);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long ul)
{
    formatInteger(ul);
    return *this;
}

LogStream& LogStream::operator<<(long long ll)
{
    formatInteger(ll);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long ull)
{
    formatInteger(ull);
    return *this;
}

LogStream& LogStream::operator<<(float f)
{
    *this << static_cast<double>(f);
    return *this;
}

LogStream& LogStream::operator<<(double d)
{
    if(buffer_->Avail() >= KMAX_NUMERIC_SIZE)
    {
        char tmp_buf [KMAX_NUMERIC_SIZE];
        auto len = snprintf(tmp_buf, KMAX_NUMERIC_SIZE, "%0.12g", d);
        buffer_->Append(std::string_view(tmp_buf, len));
    }
    return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
    std::uintptr_t v = reinterpret_cast<std::uintptr_t>(p);
    constexpr size_t KMAX_NUMERIC_SIZE_Plus_2 = KMAX_NUMERIC_SIZE + 2;
    if(buffer_->Avail() >= KMAX_NUMERIC_SIZE_Plus_2)
    {
        char tmp_buf[KMAX_NUMERIC_SIZE_Plus_2];
        tmp_buf[0] = '0';
        tmp_buf[1] = 'x';
        size_t len = Convert2Hex(v, &tmp_buf[2]);
        buffer_->Append(std::string_view(tmp_buf, len + 2));
    }
    return *this;
}

LogStream& LogStream::operator<<(const char* str)
{
    if(str)
    {
        buffer_->Append(str);
    }
    else 
    {
        buffer_->Append("(null)");
    }
    return *this;
}

LogStream& LogStream::operator<<(const unsigned char* str)
{
    return *this<<reinterpret_cast<const char*>(str);
}

LogStream& LogStream::operator<<(std::string_view sv)
{
    buffer_->Append(sv);
    return *this;
}

LogStream& LogStream::operator<<(const FixedBuffer<DEFAULT_LOGSTREAM_BUFFER_SIZE>& buffer)
{
    return *this<<buffer.ToString();
}

std::string_view LogStream::ToStringView() const
{
    return buffer_->ToString();
}