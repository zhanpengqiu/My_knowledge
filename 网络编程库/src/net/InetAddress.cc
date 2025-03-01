#include <InetAddress.h>

// 设置addr地址，作为socket的输入
///htons()作用是将端口号由主机字节序转换为网络字节序的整数值。(host to net)
// inet_addr()作用是将一个IP字符串转化为一个网络字节序的整数值，用于sockaddr_in.sin_addr.s_addr
// inet_ntoa()作用是将一个sin_addr结构体输出成IP字符串(network to ascii)
InetAddress::InetAddress(uint16_t port, std::string ip){
    ::bzero(&addr_,sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = ::htons(port);
    addr_.sin_addr.s_addr = ::inet_addr(ip.c_str());
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

std::string InetAddress::toIp() const{
    char buf[64];
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

uint16_t InetAddress::toPort() const{
    return ntohs(addr_.sin_port);
}

std::string InetAddress::toIpPort() const{
    char buf[64]={0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end=::strlen(buf);
    uint16_t port=::ntohs(addr_.sin_port);
    // 在buf后面追加port
    sprintf(buf+end,":%u",port);
    return buf;
}

#if 0
#include <iostream>
int main(){
    InetAddress addr(8080,"127.0.0.1");
    std::cout<<addr.toIpPort()<<std::endl;
    return 0;
}
#endif // 0 test