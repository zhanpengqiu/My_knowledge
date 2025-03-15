#ifndef TIMER_H
#define TIMER_H
#include "noncopyable.h"
#include "Timestamp.h"
#include <functional>

class Timer:noncopyable {
public:
    using TimerCallback=std::function<void()>;
    Timer(TimerCallback callback,Timestamp when,double interval)
        :callback_(callback),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0) // 一次性定时器设置为0
        {}
    void run() const { callback_(); } //执行定时器回调函数

    Timestamp expiration()const { return expiration_;}
    bool repeat() const { return repeat_; }

    // 重启定时器（如果是非重复事件则到期时间设置为0
    void restart(Timestamp now);
private:
    const TimerCallback callback_;//定时器回调函数
    Timestamp expiration_;  //下一次超时时刻
    const double interval_;//超时时间间隔
    const bool repeat_; //是否是重复定时器
};
#endif // !TIMER_H
