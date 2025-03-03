#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

#include "Poller.h"
#include "Timestamp.h"

class EPollPoller:public Poller{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    Timestamp poll(int timeout, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    // 默认监听数量
    static const int kInitEventListSize = 16;
    // 填写活跃的连接
    void fillActiveChannels(int numEvents,ChannelList* activeChannels)const;
    // 更新channel通道，本质是调用了epoll_ctl
    void update(int operation,Channel* channel);

    int epollfd_;   //epoll_create在内核创建空间返回的fd
    std::vector<epoll_event> events_;
};


#endif