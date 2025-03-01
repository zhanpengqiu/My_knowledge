#include "Poller.h"

Poller::Poller(EventLoop* eventLoop)
    : ownerLoop_(eventLoop){

    }
bool Poller::hasChannel(Channel* channel)const{
    auto it=channels_.find(channel->fd());
    return it!=channels_.end()&&it->second==channel;
}