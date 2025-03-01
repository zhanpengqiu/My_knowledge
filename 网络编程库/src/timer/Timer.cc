#include "Timer.h"

void Timer::restart(Timestamp now){
    if(!repeat_){
        expiration_ = Timestamp::invalid();
    }else{
        expiration_ = addTime(now, interval_);
    }
    // 重新注册到TimerQueue中
    // ...
}