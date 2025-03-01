#include "Timestamp.h"

// 获取当前时间辍
Timestamp Timestamp::now(){
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    int64_t microseconds = tv.tv_usec;
    return Timestamp(seconds*1000*1000 + microseconds);
}

// 改成
// "%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",时分秒.微秒
// etc:
// 2025年03月01日 星期6 17:12:06
// 2025年03月01日 星期6 17:12:06.045872
std::string Timestamp::toFormattedString(bool showMicroseconds) const{
    char buf[64]={0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_/kMicroSecondsPerSecond);

    tm *tm_time=localtime(&seconds);

    if(showMicroseconds) {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf, sizeof(buf),"%4d年%02d月%02d日 星期%d %02d:%02d:%02d.%06d",
                tm_time->tm_year+1900, 
                tm_time->tm_mon+1, 
                tm_time->tm_mday,
                tm_time->tm_wday, 
                tm_time->tm_hour, 
                tm_time->tm_min, 
                tm_time->tm_sec, 
                microseconds);
    }
    else{
        snprintf(buf, sizeof(buf), "%4d年%02d月%02d日 星期%d %02d:%02d:%02d",
        tm_time->tm_year+1900, 
        tm_time->tm_mon+1, 
        tm_time->tm_mday,
        tm_time->tm_wday, 
        tm_time->tm_hour, 
        tm_time->tm_min, 
        tm_time->tm_sec);
    }
    return buf;
}

// int main()
// {
//     Timestamp time;
//     std::cout << time.now().toFormattedString() << std::endl;
//     std::cout << time.now().toFormattedString(true) << std::endl;

//     return 0;
// }