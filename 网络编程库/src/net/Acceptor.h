#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;

// 接受器：负责监听并接收来自客户端的TCP连接
/**
 * Acceptor 运行在mainLoop中，不断接受新的tcp连接到来，并将处理连接的部分下放到subLoop中
 * 实际操作：TcpServer发现Acceptor有一个新连接，则将此channel分发给一个subloop
 */
class Acceptor{
public:
    using NewConnectionCallback=std::function<void(int sockfd,const InetAddress& )>;
    Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb){
        NewConnectionCallback_=cb;
    }

    bool listenning() const { return listening_;}
    void listen();
private:
    void handleRead();

    EventLoop* loop_; // Acceptor用的是用户自定义的Baseoop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback NewConnectionCallback_;
    bool listening_;
};

#endif // ACCEPTOR_H
