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
    //构造函数，将次poller绑定到一个EventLoop（线程循环）中
    Poller(EventLoop* loop);
    // 析函数，释放所有channel。并且具有继承的性质，所以设置为虚函数
    virtual ~Poller()=default;
    //对外暴露的poll结构，主要用于调用epoll_wait或者poll_wait
    // 接受超时事件，并返回发生事件的ChannelList，返回的时间为poll函数被调用的时间点
    virtual Timestamp poll(int timeout,ChannelList* channels)=0;
    // 注册新的事件（使用Channel包装的事件）
    virtual void updateChannel(Channel* channel)=0;
    // 删除已注册的事件（使用Channel包装的事件），在Channel被关闭或者被销��时被调用，并在Poller中删除该channel并关闭epoll的句��
    virtual void removeChannel(Channel* channel)=0;
    // 检测当前是否有事件(使用Channel包装的事件)监听
    bool hasChannel(Channel* channel)const;
    // 创建并返回一个 Poller 类型的对象指针
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    using ChannelMap=std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;// 定义Poller所属的事件循环EventLoop
};


#endif // POLLER_H