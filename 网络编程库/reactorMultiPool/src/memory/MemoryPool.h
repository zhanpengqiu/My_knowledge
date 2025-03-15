#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

#define PAGE_SIZE 4096
#define MP_ALIGNMENT 16
#define mp_align(n, alignment) (((n)+(alignment-1)) & ~(alignment-1))
#define mp_align_ptr(p, alignment) (void *)((((size_t)p)+(alignment-1)) & ~(alignment-1))

struct SmallNode{
    unsigned char* end_;     // 该块的结尾
    unsigned char* last_;    // 该块目前使用位置
    unsigned int quote_;     // 该块被引用次数
    unsigned int failed_;    // 该块失效次数
    struct SmallNode* next_; // 指向下一个块
};

struct LargeNode{
    void *address;
    unsigned int size_;
    struct LargeNode* next_; 
};

struct Pool{
    LargeNode* largeList_;   // 管理大块内存链表
    SmallNode* head_;        // 头节点
    SmallNode* current_;     // 指向当前分配的块，这样可以避免遍历前面已经不能分配的块 
};

class MemoryPool{
public:
    MemoryPool()=default;
    ~MemoryPool()=default;
    // 初始化内存
    void createPool();

    void destroyPool();

    void* malloc(unsigned long size);

    void calloc(unsigned long size);

    void freeMemory(void* ptr);

    void resetPool();

    Pool* getPool(){return pool_;}

private:
    void* mallocLargeNode(unsigned long size);

    void* mallocSmallNode(unsigned long size);

    Pool* pool_=nullptr;
};

#endif // MEMORY_POOL