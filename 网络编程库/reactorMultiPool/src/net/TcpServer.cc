#include <functional>
#include <string.h>

#include "TcpServer.h"
#include "TcpConnection.h"

//查看传入的baseLoop指针是不是有意义的
static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        std::cerr << "TcpServer::ctor - nullptr loop passed!";
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg,Option option)
    : loop_(CheckLoopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr,option==kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_ )),
      connectionCallback_(),
      messageCallback_(),
      writeCompleteCallback_(),
      threadInitCallback_(),
      started_(0),
      nextConnId_(1){
    // 当有新用户连接时，Acceptor类中绑定的acceptChannel_会有读事件发生执行handleRead()调用TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer(){
    for(auto& item:connections_){
        TcpConnectionPtr conn(item.second);
        // 把原始的智能指针复位 让栈空间的TcpConnectionPtr conn指向该对象 当conn出了其作用域 即可释放智能指针指向的对象
        item.second.reset();
        // 销毁连接
        loop_->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numThreads){
    threadPool_->setThreadNum(numThreads);
}

// 开启监听操作
void TcpServer::start(){
    if (started_++==0){
        threadPool_->start(threadInitCallback_);
        // 开始接受新连接
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd,const InetAddress& peerAddr){
    // 轮训算法，获得一个处理新连接的subLoop管理这个connfd的channel
    EventLoop* subLoop=threadPool_->getNextLoop();

    // 建立新的TcpConnection
    char buf[64]={0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    // 这里没有设置为原子类是因为其只在mainloop中执行 不涉及线程安全问题
    ++nextConnId_;
    std::string connName=name_+buf;

    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addrlen=sizeof(local);
    if(::getsockname(sockfd,(sockaddr*)&local,&addrlen)<0){
        std::cerr<<"getsockname error"<<std::endl;
        ::close(sockfd);
        return;
    }
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(subLoop, connName, sockfd, localAddr, peerAddr));

    // 新建立的TcpConnection和channel 加入到 connections_中
    connections_[connName]=conn;

    // 新建立的TcpConnection 建立好后，会被 channel 所管理，当有事件发生时，channel 所绑定的回调函数会被执行
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    // 设置了如何关闭连接的回调
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    subLoop->runInLoop(
        std::bind(&TcpConnection::connectEstablished, conn));
}
void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}