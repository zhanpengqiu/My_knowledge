#pragma once

#include "noncopyable.h"

// 声明
class InetAddress;

class Socket : noncopyable{
public:
    explicit Socket(int sockfd):sockfd_(sockfd) {}

    ~Socket();

    // 获取套接字的描述符
    int getfd()const{return sockfd_;}

    void bindAddress(const InetAddress& addr);
    // 侦听接口
    void listen();
    // 接受一个连接
    int accept(InetAddress *peeraddr);
    // 设置半关闭
    void shutdownWrite();

    void setTcpNoDelay(bool on);// 设置nagel算法

    void setReuseAddr(bool on);//设置地址复用

    void setReusePort(bool on);// 设置端口复用

    void setKeepAlive(bool on);//设置长链接
private:
    const int sockfd_;
};