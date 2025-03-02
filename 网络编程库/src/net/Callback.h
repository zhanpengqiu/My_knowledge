#ifndef CALLBACK_H
#define CALLBACK_H

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
class Timestamp;

// 以下是TcpConnection的回调函数
using TcpConnectionPtr=std::unique_ptr<TcpConnection>;
using ConnectionCallback=std::function<void(const TcpConnectionPtr&)>;
using CloseCallback=std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback=std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback=std::function<void(const TcpConnectionPtr&,size_t)>;

// 消息处理的callback
using MessageCallback=std::function<void(const TcpConnectionPtr&,
                                        Buffer *,
                                        Timestamp *)>;

#endif // CALLBACK_H