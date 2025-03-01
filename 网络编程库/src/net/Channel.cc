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

// 在TcpConnection建立的时候会游泳
void Channel::tie(const std::shared_ptr<void>&obj){
    tie_=obj;
    tied_ = true;
}

void Channel::update(){
    //TODO:Channel::update()
    // 通过该channel所属的EventLoop，调用poller对应的方法，注册fd的events事件
}

void Channel::handleEvent(Timestamp receiveTime){
    if(tied_){
        std::shared_ptr<void> guard=tie_.lock();
        if(guard){
            handleEventWithGuard(receiveTime);
        }
    }else{
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime){
    
}
