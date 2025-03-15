#include "Acceptor.h"
#include "InetAddress.h"
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

//创建socket的fd，指定为ipv4协议及tcp
static int createNonblocking(){
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0){
        perror("socket");
        abort();
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop,const InetAddress& listenAddress, bool reuseport)
    : loop_(loop),
      acceptSocket_(createNonblocking()),
      acceptChannel_(loop, acceptSocket_.getfd()),
      listening_(false)
{
    std::cout<<"Acceptor create nonblocking"<<std::endl;

    acceptSocket_.setReuseAddr(reuseport);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddress);
    /**
     * TcpServer::start() => Acceptor.listen
     * 有新用户的连接，需要执行一个回调函数
     * 因此向封装了acceptSocket_的channel注册回调函数
     */
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor(){
    // 把从Poller中感兴趣的事件删除掉
    acceptChannel_.disableAll();    
    // 调用EventLoop->removeChannel => Poller->removeChannel 把Poller的ChannelMap对应的部分删除
    acceptChannel_.remove();   
}

void Acceptor::listen(){
    listening_ = true;
    acceptSocket_.listen();
    // 将acceptChannel的读事件注册到poller
    acceptChannel_.enableReading();
}

// listenfd有事件发生了，就是有新用户连接了
void Acceptor::handleRead(){
    InetAddress peerAddr;
    //接受新连接
    int connfd=acceptSocket_.accept(&peerAddr);

    //确定有新连接到来
    if(connfd>=0){
        if(NewConnectionCallback_){
            // 轮询找到subLoop 唤醒并分发当前的新客户端的Channel
            NewConnectionCallback_(connfd,peerAddr);
        }else{
            std::cout<<"no newConnectionCallback() function"<<std::endl;
            ::close(connfd);
        }
    }else{
        std::cerr<<"accept falied"<<std::endl;
        
        // 当前进程的fd已经用完了
        // 可以调整单个服务器的fd上限
        // 也可以分布式部署
        if (errno == EMFILE){
            std::cerr<<"sockfd reached limit"<<std::endl;
        }
    }
}