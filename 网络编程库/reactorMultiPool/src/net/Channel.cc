#include "Channel.h"
#include "EventLoop.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    :loop_(loop), 
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false)
{

}
// TODO:析构操作和断言，判断是否在当前线程
Channel::~Channel() {
}

// 在TcpConnection建立的时候会调用
void Channel::tie(const std::shared_ptr<void>&obj){
    tie_=obj;
    tied_ = true;
}

void Channel::update(){
    //TODO:Channel::update()
    // 通过该channel所属的EventLoop，调用poller对应的方法，注册fd的events事件
    loop_->updateChannel(this);
}
// 在channel所属的EventLoop中，把当前的channel删除掉
void Channel::remove(){
    loop_->removeChannel(this);
}


void Channel::handleEvent(Timestamp receiveTime){
    /**
     * 调用了Channel::tie得会设置tid_=true
     * 而TcpConnection::connectEstablished会调用channel_->tie(shared_from_this());
     * 所以对于TcpConnection::channel_ 需要多一份强引用的保证以免用户误删TcpConnection对象
     */
    if(tied_){
        std::shared_ptr<void> guard=tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}
// 根据相应事件执行回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime){
    // 对端关闭事件
    // 当TcpConnection对应Channel，通过shutdown关闭写端，epoll触发EPOLLHUP
    if((revents_&EPOLLHUP)&&(revents_&EPOLLIN)){
        if(closeCallback_){
            closeCallback_();
        }
    }
    //错误事件
    if(revents_&EPOLLERR) {
        std::cerr<< "error"<<std::endl;
        if(errorCallback_ ){
            errorCallback_();
        }
    }
    //读事件
    if(revents_&(EPOLLIN|EPOLLPRI)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }
    // 写事件
    if(revents_&(EPOLLOUT)){
        if(writeCallback_){
            writeCallback_();
        }
    }
}
