#include "EPollPoller.h"
#include <string.h>

const int kNew = -1;    // 某个channel还没添加至Poller 、channel的成员index_初始化为-1
const int kAdded = 1;   // 某个channel已经添加至Poller
const int kDeleted = 2; // 某个channel已经从Poller删除

// TODO:epoll_create1(EPOLL_CLOEXEC)
EPollPoller::EPollPoller(EventLoop* eventLoop):
    Poller(eventLoop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize) 
{
    //创建fd句柄，epoll_create1
    if(epollfd_<0){
        std::cerr<< "epoll_create() error:" << std::endl;
    }    
}

EPollPoller::~EPollPoller(){
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs,ChannelList* activechannels){
    // 设置超时事件
    size_t numEvents=::epoll_wait(epollfd_,&(*events_.begin()),static_cast<int>(events_.size()),timeoutMs);
    int savedErrno=errno;
    Timestamp now(Timestamp::now());

    if(numEvents>0){
        //有事件产生
        fillActiveChannels(numEvents, activechannels);
        // 对events_进行扩容操作
        if(numEvents==static_cast<size_t>(events_.size())){
            events_.resize(events_.size()*2);
        }
    }else if(numEvents==0){
        //超时
        std::cerr<<"timeout!"<<std::endl;
    }else{
        // 出错
        // 不是终端出错
        if(savedErrno!=EINTR){
            std::cerr<<"epoll_wait() error: "<<strerror(savedErrno)<<std::endl;
        }
    }
    return now;
}

//改变Channel的状态，如有有需要注册或者删除epoll事件的时候，就调用这个函数
void EPollPoller::updateChannel(Channel* channel){
    const int index=channel->index();
    //判断信赖的channel是不是新的或者删除的channel，新的则加入到监听的channels中
    // 一般是一个EVentLoop监听一组Channels
    if(index==kNew||index==kDeleted){
        // 添加到键值对
        if(index==kNew){
            int fd=channel->fd();
            channels_[fd]=channel;

        }else{// index == kAdd

        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }//已经注册过
    else{
        // 没有感兴趣事件说明可以从epoll对象中删除该channel了
        if(channel->isNoneEvent()){
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        // 还有事件说明之前的事件删除，但是被修改了
        else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

// 加入活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const{
    for(int i=0;i<numEvents;++i){
        Channel*channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

void EPollPoller::removeChannel(Channel* channel){
    int fd = channel->fd();
    channels_.erase(fd);

    int index=channel->index();
    if(index == kAdded){
        update(EPOLL_CTL_DEL,channel);
    }
    channel->setIndex(kNew);
}

void EPollPoller::update(int operation, Channel *channel){
    epoll_event event;
    bzero(&event, sizeof(event));

    int fd = channel->fd();
    event.data.ptr = channel;
    event.events = channel->events();
    event.data.fd = fd;

    if(::epoll_ctl(epollfd_,operation,fd,&event) < 0){
        if(operation == EPOLL_CTL_DEL){
            std::cerr<<"epoll_ctl error: delete event"<<std::endl;
        }else{
            std::cerr<<"epoll_ctl error: add/modify event"<<std::endl;
        }
    }
}