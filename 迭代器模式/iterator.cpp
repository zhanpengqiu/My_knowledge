#include <iostream>
#include <vector>
using namespace std;

// 迭代器接口：定义访问和遍历集合元素的方法
class Iterator {
public:
    virtual ~Iterator() {}
    virtual bool hasNext() = 0; // 判断是否还有下一个元素
    virtual int next() = 0;     // 获取下一个元素
};

// 具体迭代器：实现迭代器接口，负责遍历集合的具体逻辑
class ConcreteIterator : public Iterator {
private:
    vector<int> collection; // 被迭代的集合
    int position;           // 当前位置

public:
    // 构造函数，传入集合数据
    ConcreteIterator(const vector<int>& coll) : collection(coll), position(0) {}

    bool hasNext() override {
        return position < collection.size();
    }

    int next() override {
        if (hasNext()) {
            return collection[position++];
        }
        throw out_of_range("No more elements in the collection.");
    }
};

// 集合接口：定义创建迭代器的方法
class Collection {
public:
    virtual ~Collection() {}
    virtual Iterator* createIterator() = 0; // 创建并返回一个迭代器
};

// 具体集合：实现集合接口，并维护集合数据
class ConcreteCollection : public Collection {
private:
    vector<int> items; // 集合中的元素

public:
    // 添加元素到集合
    void addItem(int item) {
        items.push_back(item);
    }

    Iterator* createIterator() override {
        return new ConcreteIterator(items); // 返回具体迭代器
    }
};

// 测试代码
int main() {
    ConcreteCollection collection;

    // 向集合中添加元素
    collection.addItem(10);
    collection.addItem(20);
    collection.addItem(30);

    // 创建迭代器并遍历集合
    Iterator* iterator = collection.createIterator();
    cout << "Iterating over collection:" << endl;
    while (iterator->hasNext()) {
        cout << iterator->next() << " ";
    }
    cout << endl;

    delete iterator; // 释放迭代器资源
    return 0;
}