#include <iostream>
#include <string>
#include <memory>

// 产品类：表示要构建的复杂对象
class Product {
private:
    std::string partA;
    std::string partB;
    std::string partC;

public:
    void setPartA(const std::string& value) { partA = value; }
    void setPartB(const std::string& value) { partB = value; }
    void setPartC(const std::string& value) { partC = value; }

    void showProduct() const {
        std::cout << "Product Parts:" << std::endl;
        if (!partA.empty()) std::cout << "Part A: " << partA << std::endl;
        if (!partB.empty()) std::cout << "Part B: " << partB << std::endl;
        if (!partC.empty()) std::cout << "Part C: " << partC << std::endl;
    }
};

// 抽象建造者：定义创建产品各个部分的接口
class Builder {
public:
    virtual ~Builder() = default;
    virtual void buildPartA() = 0;
    virtual void buildPartB() = 0;
    virtual void buildPartC() = 0;
    virtual std::unique_ptr<Product> getProduct() = 0;
};

// 具体建造者1：实现抽象建造者的接口
class ConcreteBuilder1 : public Builder {
private:
    std::unique_ptr<Product> product;

public:
    ConcreteBuilder1() : product(std::make_unique<Product>()) {}

    void buildPartA() override {
        product->setPartA("ConcreteBuilder1 Part A");
    }

    void buildPartB() override {
        product->setPartB("ConcreteBuilder1 Part B");
    }

    void buildPartC() override {
        product->setPartC("ConcreteBuilder1 Part C");
    }

    std::unique_ptr<Product> getProduct() override {
        return std::move(product);
    }
};

// 具体建造者2：实现抽象建造者的接口
class ConcreteBuilder2 : public Builder {
private:
    std::unique_ptr<Product> product;

public:
    ConcreteBuilder2() : product(std::make_unique<Product>()) {}

    void buildPartA() override {
        product->setPartA("ConcreteBuilder2 Part A");
    }

    void buildPartB() override {
        product->setPartB("ConcreteBuilder2 Part B");
    }

    void buildPartC() override {
        product->setPartC("ConcreteBuilder2 Part C");
    }

    std::unique_ptr<Product> getProduct() override {
        return std::move(product);
    }
};

// 指挥者：指导建造过程
class Director {
public:
    void construct(Builder& builder) {
        builder.buildPartA();
        builder.buildPartB();
        builder.buildPartC();
    }
};

int main() {
    // 创建指挥者和具体建造者
    Director director;

    // 使用具体建造者1
    ConcreteBuilder1 builder1;
    director.construct(builder1);
    auto product1 = builder1.getProduct();
    std::cout << "Product from ConcreteBuilder1:" << std::endl;
    product1->showProduct();

    // 使用具体建造者2
    ConcreteBuilder2 builder2;
    director.construct(builder2);
    auto product2 = builder2.getProduct();
    std::cout << "\nProduct from ConcreteBuilder2:" << std::endl;
    product2->showProduct();

    return 0;
}