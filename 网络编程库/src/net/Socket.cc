#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <iostream>
#include "Socket.h"
#include "InetAddress.h"

Socket::~Socket(){
    // 关闭连接
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress& addr){
    if(0!=::bind(sockfd_, (sockaddr*)addr.getSockAddr(),sizeof(sockaddr_in))){
        std::cerr<<"bind sockfd:" << sockfd_ << " fail";
    }
}

void Socket::listen(){
    if(0!=::listen(sockfd_,1024)){
        std::cerr<<"bind sockfd:" << sockfd_ << " fail";
    }
}
// TODO:socket的各个设置的用法
void Socket::shutdownWrite()
{
    /**
     * 关闭写端，但是可以接受客户端数据
     */
    if (::shutdown(sockfd_, SHUT_WR) < 0)
    {
        std::cerr << "shutdownWrite error";
    }
}
int Socket::accept(InetAddress* peerAddr){
    /**
     * 1. accept函数的参数不合法
     * 2. 对返回的connfd没有设置非阻塞
     * Reactor模型 one loop per thread
     * poller + non-blocking IO
    **/
    sockaddr_in addr;
    socklen_t addrlen=sizeof(addr);
    ::memset(&addr,0,sizeof(addr));

    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if(connfd>=0){
        peerAddr->setSockAddr(addr);
    }else{
        std::cerr<<"bind sockfd:" << sockfd_ << " fail";
    }
    return connfd;
}

// 不启动Nagle算法
void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

// 设置地址复用，其实就是可以使用处于Time-wait的端口
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

// 通过改变内核信息，多个进程可以绑定同一个地址。通俗就是多个服务的ip+port是一样
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)); // TCP_NODELAY包含头文件 <netinet/tcp.h>
}