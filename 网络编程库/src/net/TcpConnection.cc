#include<functional>
#include <string>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/tcp.h>

#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop){
    // 如果传入EventLoop没有指向有意义的地址则出错
    // 正常来说在 TcpServer::start 这里就生成了新线程和对应的EventLoop
    if(!loop){
        errno = EINVAL;
        throw std::runtime_error("TcpConnection::connect() in invalid EventLoop");
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
                             : name_(nameArg),
                               loop_(CheckLoopNotNull(loop)),
                               socket_(new Socket(sockfd)),
                               reading_(true),
                               channel_(new Channel(loop, sockfd)),
                               localAddr_(localAddr),
                               peerAddr_(peerAddr),
                               state_(kConnecting),
                               highWaterMark_(64*1024*1024) //654M 避免发送太快，对面接受太慢
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError,this));

    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection(){
    std::cout<<"TcpConnection::dtor[" << name_.c_str() << "] at fd=" << channel_->fd() << " state=" << static_cast<int>(state_);
}

//发送数据
void TcpConnection::send(const std::string &buf){
    if(state_==kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(buf.c_str(),buf.size());
        }
    }else{
        // 遇到重载函数的绑定，可以使用函数指针来指定确切的函数
        void(TcpConnection::*fp)(const void *data,size_t len)=&TcpConnection::sendInLoop;
        loop_->runInLoop(std::bind(fp,this,buf.c_str(),buf.size()));
    }
}

void TcpConnection::send(Buffer *buf){
    if(state_==kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(buf->peek(),buf->readableBytes());
            buf->retrieveAll();
        }else{
            // sendInLoop有多重重载，需要使用函数指针确定
            void(TcpConnection::*fp)(const std::string& message)=&TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp,this,buf->retrieveAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message){
    sendInLoop(message.c_str(),message.size());
}
/**
 * 发送数据 应用写的快 而内核发送数据慢 需要把待发送数据写入缓冲区，而且设置了水位回调
 **/
void TcpConnection::sendInLoop(const void* data, size_t len){

}

// 关闭连接
void TcpConnection::shutdown(){
    if(state_==kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(
            std::bind(&TcpConnection::shutdownInLoop, this)
        );
    }
}

void TcpConnection::shutdownInLoop(){
    if (!channel_->isWriting()) // 说明当前outputBuffer_的数据全部向外发送完成
    {
        socket_->shutdownWrite();
    }
}

void TcpConnection::connectEstablished(){
    setState(kConnected); // 建立连接，设置一开始状态为连接态
    /**
     * TODO:tie
     * channel_->tie(shared_from_this());
     * tie相当于在底层有一个强引用指针记录着，防止析构
     * 为了防止TcpConnection这个资源被误删掉，而这个时候还有许多事件要处理
     * channel->tie 会进行一次判断，是否将弱引用指针变成强引用，变成得话就防止了计数为0而被析构得可能
     */
    channel_->tie(shared_from_this());
    channel_->enableReading();// 向poller注册channel的EPOLLIN读事件
    // 新连接建立 执行回调
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed(){
    if(state_ == kConnected){
        setState(kDisconnected);
        channel_->disableAll(); // 关闭所有事件
        connectionCallback_(shared_from_this());
    }
    channel_->remove();//把channel所有感兴趣的事件从poller删除
}

void TcpConnection::handleRead(Timestamp receiveTime){
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if(n>0){
        // 接收到数据，执行回调
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }else if(n==0){
        // 接收到EOF
        handleClose();
    }else{
        // 出错
        if(savedErrno!=EAGAIN){
            errno = savedErrno;
            std::cerr << "TcpConnection::handleRead";
        }
        handleError();
    }
}


void TcpConnection::handleWrite()
{
    if (channel_->isWriting())
    {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        // 正确读取数据
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            // 说明buffer可读数据都被TcpConnection读取完毕并写入给了客户端
            // 此时就可以关闭连接，否则还需继续提醒写事件
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting();
                // 调用用户自定义的写完数据处理函数
                if (writeCompleteCallback_)
                {
                    // 唤醒loop_对应得thread线程，执行写完成事件回调
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            std::cerr << "TcpConnection::handleWrite() failed";
        }
    }
    // state_不为写状态
    else
    {
        std::cerr << "TcpConnection fd=" << channel_->fd() << " is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    setState(kDisconnected);    // 设置状态为关闭连接状态
    channel_->disableAll();     // 注销Channel所有感兴趣事件
    
    TcpConnectionPtr connPtr(shared_from_this());
    connectionCallback_(connPtr);   
    closeCallback_(connPtr);        // 关闭连接得回调
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    // TODO:getsockopt ERROR
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen))
    {
        err = errno;
    }
    else
    {
        err = optval;
    }
    std::cerr << "cpConnection::handleError name:" << name_.c_str() << " - SO_ERROR:" << err;
}