#include <iostream>
#include <memory>
#include <chrono>
#include <vector>
 
constexpr int N = 10'000'000; // 操作次数
 
// 测试对象
struct Foo {
    int value;
    Foo(int v) : value(v) {}
};
 
// 测试 1: shared_ptr 的创建和销毁
void test_shared_ptr_create_destroy() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        auto p = std::make_shared<Foo>(42);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "shared_ptr create/destroy: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / N 
              << " ns/op\n";
}
 
// 测试 2: 原始指针的创建和销毁（手动管理内存）
void test_raw_ptr_create_destroy() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        Foo* p = new Foo(42);
        delete p;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "raw_ptr create/destroy: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / N 
              << " ns/op\n";
}
 
// 测试 3: shared_ptr 的拷贝构造
void test_shared_ptr_copy() {
    auto p = std::make_shared<Foo>(42);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        auto p2 = p;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "shared_ptr copy: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / N 
              << " ns/op\n";
}
 
// 测试 4: 原始指针的赋值
void test_raw_ptr_copy() {
    Foo* p = new Foo(42);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        Foo* p2 = p;
    }
    auto end = std::chrono::high_resolution_clock::now();
    delete p;
    std::cout << "raw_ptr copy: " 
              << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / N 
              << " ns/op\n";
}
 
int main() {
    test_shared_ptr_create_destroy();
    test_raw_ptr_create_destroy();
    test_shared_ptr_copy();
    test_raw_ptr_copy();
    return 0;
}