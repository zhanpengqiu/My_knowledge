#include "Poller.h"
#include "EPollPoller.h"

#include <stdlib.h>

// 通过环境变量选择那种触发方式
Poller* Poller::newDefaultPoller(EventLoop* eventLoop){
    if(getenv("MUDUO_USE_POLL")){
        return nullptr; //生成poll实例
    }else{
        return new EPollPoller(eventLoop);//生成epoll实例
    }
}