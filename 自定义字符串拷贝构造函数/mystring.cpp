#include <iostream>
#include <cstring>

using namespace std;

class MyString{
private:
    char* data;
public:
    MyString(const char* str=""){
        if(data == nullptr) {
            data=new char[1];
            data[0]='\0';
        }else{
            data=new char[strlen(str)+1];
            strcpy(data,str);
        }
        cout<<"MyString created: "<<data<<endl;
    }

    MyString(const MyString& other){
        data=new char[strlen(other.data)+1];
        strcpy(data,other.data);
        cout<<"MyString copied: "<<data<<endl;
    }

    MyString& operator=(const MyString& other){
        if(this != &other){
            // 删除原有的数据
            delete[] data;
            data=new char[strlen(other.data)+1];
            strcpy(data,other.data);
        }
        cout<<"MyString assigned: "<<data<<endl;
        return *this;
    }
    ~MyString(){
        cout<<"MyString destroyed: "<<data<<endl;
        delete[] data;
    }
    void print(){
        cout<<data<<endl;
    }
};
int main(){
    MyString str1("Hello");
    MyString str2("World");
    str1.print();
    str2.print();
    
    // 调用拷贝构造函数
    MyString str3=str1;
    str3.print();

    //调用赋值构造函数
    str1=str2;
    str1.print();
    str2.print();

    return 0;
}