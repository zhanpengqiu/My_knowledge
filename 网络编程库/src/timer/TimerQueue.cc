#include "EventLoop.h"
#include "Channel.h"
#include "Timer.h"
#include "TimerQueue.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>

int createTimerfd(){
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd < 0){
        perror("timerfd_create");
        abort();
    }
    return timerfd;
}

TimerQueue::TimerQueue(EventLoop *loop)
    :loop_(loop),
     timerfd_(createTimerfd()),
     timerfdChannel_(loop_,timerfd_) ,
     timers_()
     {

     }
TimerQueue::~TimerQueue(){
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
    //删除所有的定时器
    for(const Entry& timer:timers_){
        delete timer.second;
    }

}

void TimerQueue::addTimer(TimerCallback cb, Timestamp when,double interval){
    Timer* timer = new Timer(std::move(cb),when,interval);
    loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
}