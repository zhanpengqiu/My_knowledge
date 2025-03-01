#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"
#include "Channel.h"
#include "Timestamp.h"

#include <vector>
#include <unordered_map>

class Poller:noncopyable{
public:
    using ChannelList=std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeout,ChannelList* channels)=0;
    virtual void updateChannel(Channel* channel)=0;
    virtual void removeChannel(Channel* channel)=0;

    bool hasChannel(Channel* channel)const;

    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    using ChannelMap=std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;// 定义Poller所属的事件循环EventLoop
};


#endif // POLLER_H