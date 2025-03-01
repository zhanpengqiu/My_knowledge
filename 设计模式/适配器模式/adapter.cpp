#include <iostream>
#include <string>

// 目标接口：定义客户端期望的接口
class TargetInterface {
public:
    virtual ~TargetInterface() = default;
    virtual void request() const = 0; // 客户端需要调用的方法
};

// 旧系统类：具有不兼容的接口
class OldSystem {
public:
    void specificRequest() const {
        std::cout << "OldSystem: Specific Request" << std::endl;
    }
};

// 适配器类：将旧系统的接口适配为目标接口
class Adapter : public TargetInterface {
private:
    OldSystem oldSystem;

public:
    void request() const override {
        // 调用旧系统的特定方法
        oldSystem.specificRequest();
    }
};

// 客户端代码：只依赖目标接口
void clientCode(const TargetInterface& target) {
    target.request(); // 客户端只需要调用目标接口的方法
}

int main() {
    // 创建适配器实例
    Adapter adapter;

    // 使用适配器
    std::cout << "Client: I am working with the TargetInterface:" << std::endl;
    clientCode(adapter);

    return 0;
}