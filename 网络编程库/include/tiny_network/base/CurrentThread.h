#ifndef CURRENT_THREAD_H
#define CURRENT_THREAD_H

#include  <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {
    //保存tid缓冲，避免多次系统调用
    extern __thread int t_cachedTid;
    
    void cacheTid();

    // 内链函数
    inline int tid(){
        if(__builtin_expect(t_cachedTid == 0,0)){
            cacheTid();
        }
        return t_cachedTid;
    }

}  // namespace CurrentThread


#endif //CURRENT_THREAD