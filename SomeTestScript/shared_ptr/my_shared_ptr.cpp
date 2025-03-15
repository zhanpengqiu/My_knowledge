//控制块的设计
#include <atomic>
using namespace std;
class ControlBlockBase {
public:
    virtual ~ControlBlockBase() = default;
    virtual void dispose() noexcept = 0;  // 销毁对象
    virtual void destroy() noexcept = 0;  // 销毁控制块自身
    std::atomic<size_t> shared_count{1};
    std::atomic<size_t> weak_count{0};
};
     
template<typename T, typename Deleter>
class ControlBlockImpl : public ControlBlockBase {
public:
    ControlBlockImpl(T* ptr, Deleter deleter) : ptr(ptr), deleter(deleter) {}
    
    void dispose() noexcept override {
        deleter(ptr);
        ptr = nullptr;
    }
    
    void destroy() noexcept override {
        delete this;
    }
    
private:
    T* ptr;
    Deleter deleter;
};
     
//sharedptr
template<typename T>
class shared_ptr {
public:
    // 默认构造函数
    shared_ptr() : ptr(nullptr), ctrl(nullptr) {}
    
    // 从原始指针构造
    template<typename Deleter = std::default_delete<T>>
    explicit shared_ptr(T* p, Deleter d = Deleter()) {
        if (p) {
            ctrl = new ControlBlockImpl<T, Deleter>(p, d);
            ptr = p;
        }
    }
    
    // 拷贝构造函数
    shared_ptr(const shared_ptr& other) : ptr(other.ptr), ctrl(other.ctrl) {
        if (ctrl) ctrl->shared_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    // 移动构造函数
    shared_ptr(shared_ptr&& other) noexcept : ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }
    
    // 析构函数
    ~shared_ptr() {
        if (!ctrl) return;
        // 减少shared计数，若归零则销毁对象
        if (ctrl->shared_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            ctrl->dispose();
            // 若weak计数也为零，销毁控制块
            if (ctrl->weak_count.load(std::memory_order_acquire) == 0) {
                ctrl->destroy();
            }
        }
    }
    
private:
    T* ptr;
    ControlBlockBase* ctrl;
};
     
    //补充一个weakptr的实现
    template<typename T>
    class weak_ptr {
    public:
        weak_ptr(const shared_ptr<T>& sp) : ctrl(sp.ctrl) {
            if (ctrl) ctrl->weak_count.fetch_add(1, std::memory_order_relaxed);
        }
     
        ~weak_ptr() {
            if (!ctrl) return;
            // 减少weak计数，若归零且shared计数为零，则销毁控制块
            if (ctrl->weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1 &&
                ctrl->shared_count.load(std::memory_order_acquire) == 0) {
                ctrl->destroy();
            }
        }
     
    private:
        ControlBlockBase* ctrl;
    };