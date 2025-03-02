#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>
#include <memory>
#include <algorithm>

/// 整个的buffer布局如下，避免了只能一次完全读出数据的困难
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

class Buffer{
public:
    // prependable 初始大小，readIndex 初始位置
    static const size_t kCheapPrepend = 8;
    // writeable 初始大小，writeIndex 初始位置  
    // 刚开始 readerIndex 和 writerIndex 处于同一位置
    static const size_t kInitialSize = 1024;    
    explicit Buffer(size_t initialSize = kInitialSize)
        :   buffer_(kCheapPrepend + initialSize),
            readerIndex_(kCheapPrepend),
            writerIndex_(kCheapPrepend)
        {}

private:

    std::vector<char>buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
    static const char kCRLF[]; //定义分割字符
};

#endif