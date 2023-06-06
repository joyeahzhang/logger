#ifndef _FIXEDBUFFER_
#define _FIXEDBUFFER_

#include <array>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <string_view>
#include <algorithm>

namespace doggy {

constexpr int MAX_FIXEDBUFFER = 4000 * 1024;
constexpr int MIN_FIXEDBUFFER = 4 * 1024;


template <int SIZE>
class FixedBuffer final //不允许继承
{
public:
    // 构造函数
    FixedBuffer() : cur_(0){};

    // 不允许拷贝
    FixedBuffer(const FixedBuffer&)=delete;
    FixedBuffer& operator=(const FixedBuffer&)=delete;

    // move constructor & move assignment operator
    FixedBuffer(FixedBuffer&& other) = default;
    FixedBuffer& operator=(FixedBuffer&& other) = default;

    // destructor
    ~FixedBuffer() = default;


    size_t Append(const std::string_view data);

    inline char* Data() const noexcept { return data_.data();}
    
    inline size_t Capactiy() const noexcept { return data_.size(); }

    inline size_t Size() const noexcept { return cur_; } 

    inline size_t Avail() const noexcept { return data_.size() - cur_; }

    inline void Clear() noexcept { cur_ = 0; }
    
    void Bzero() { cur_ = 0, data_.fill(0); }

    inline std::string_view ToStringView() const { return std::string_view(data_.data(), Size()); }

private:
    // 缓冲区
    std::array<char, SIZE> data_;
    // 下一个即将加入缓冲区的元素的索引
    std::size_t cur_; 
};


template<int SIZE>
size_t FixedBuffer<SIZE>::Append(const std::string_view sv)
{
    std::size_t bytes_to_append = std::min(this->Avail(), sv.size());
    std::memcpy(&data_[cur_], sv.data(), bytes_to_append);
    cur_ += bytes_to_append;
    return bytes_to_append;
}

} // namespace doggy end

# endif