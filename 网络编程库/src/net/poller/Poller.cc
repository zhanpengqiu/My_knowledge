#include "Poller.h"

//构造函数传入所属的eventloop即可，让poller知道自己所属与哪个线程
// 为什么要知道自己属于那个线程呢
// 因为在后续的multi-reactor中会有个线程池的成员
// 在线程池中所有的线程都被分配了一个poller，线程不断侦听这个poller，管理事件
Poller::Poller(EventLoop* eventLoop)
    : ownerLoop_(eventLoop){

    }
// 查找这个Channel是否有目标channel
bool Poller::hasChannel(Channel* channel)const{
    auto it=channels_.find(channel->fd());
    return it!=channels_.end()&&it->second==channel;
}