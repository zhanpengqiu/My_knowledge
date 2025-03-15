# TCP/IP详解
## Nagle算法
### 问题
在使用一些协议通讯的时候，比如Telnet，会有一个字节字节的发送的情景，每次发送一个字节的有用数据，就会产生41个字节长的分组，20个字节的IP Header和20个字节的TCP Header，这就导致了1个字节的有用信息要浪费掉40个字节的头部信息，这是一笔巨大的字节开销，而且这种Small packet在广域网上会增加拥塞的出现。
### 方案
Nagle算法主要是避免发送小的数据包，要求TCP连接上最多只能有一个未被确认的小分组，在该分组的确认到达之前不能发送其他的小分组。相反，TCP收集这些少量的小分组，并在确认到来时以一个分组的方式发出去。
#### 应用情况
在默认的情况下,Nagle算法是默认开启的，Nagle算法比较适用于发送方发送大批量的小数据，并且接收方作出及时回应的场合，这样可以降低包的传输个数。同时协议也要求提供一个方法给上层来禁止掉Nagle算法

当你的应用不是连续请求+应答的模型的时候，而是需要实时的单项的发送数据并及时获取响应，这种case就明显不太适合Nagle算法，明显有delay的。

#### 伪代码
```python
if there is new data to send
  if the window size >= MSS and available data is >= MSS
    send complete MSS segment now
  else
    if there is unconfirmed data still in the pipe
      enqueue data in the buffer until an acknowledge is received
    else
      send data immediately
    end if
  end if
end if
```
#### 解释
1. 对于MSS的片段直接发送

2. 如果有没有被确认的data在缓冲区内，先将待发送的数据放到buffer中直到被发送的数据被确认【最多只能有一个未被确认的小分组】

3. 两种情况置位，就直接发送数据，实际上如果小包，但是没有未被确认的分组，就直接发送数据

### 缺点
Nagle指出Nagle算法与Delay ACK机制有共存的情况下会有一些非常糟糕的状况，比如举一个场景：PC1和PC2进行通信，PC1发数据给PC2，PC1使用Nagle算法，PC2有delay ACK机制

1. PC1发送一个数据包给PC2，PC2会先不回应，delay ACK

2. PC1再次调用send函数发送小于MSS的数据，这些数据会被保存到Buffer中，等待ACK，才能再次被发送

从上面的描述看，显然已经死锁了，PC1在等待ACK，PC2在delay ACK，那么解锁的代价就是Delay ACK的Timer到期，至少40ms[40ms~500ms不等]，也就是2种算法在通信的时候，会产生不必要的延时！

## Delay Ack算法
### 问题
简单的说，Delay Ack就是延时发送ACK，在收到数据包的时候，会检查是否需要发送ACK，如果需要的话，进行快速ACK还是延时ACK，在无法使用快速确认的条件下，就会使用Delay Ack。

### 方案
1.当有响应数据发送的时候，ACK会随着数据一块发送

2.如果没有响应数据，ACK就会有一个延迟，以等待是否有响应数据一块发送，但是这个延迟一般在40ms~500ms之间，一般情况下在40ms左右，如果在40ms内有数据发送，那么ACK会随着数据一块发送，对于这个延迟的需要注意一下，这个延迟并不是指的是收到数据到发送ACK的时间延迟，而是内核会启动一个定时器，每隔200ms就会检查一次，比如定时器在0ms启动，200ms到期，180ms的时候data来到，那么200ms的时候没有响应数据，ACK仍然会被发送，这个时候延迟了20ms.

3.如果在等待发送ACK期间，第二个数据又到了，这时候就要立即发送ACK！
### 缺点
过多的delay会拉长RTT
### 优点
减少了数据段的个数，提高了发送效率

# Socket接口
1. Socket
```cpp
int socket(int domain, int type, int protocol);
// domain：即协议域，又称为协议族（family）。常用的协议族有，AF_INET、AF_INET6、AF_LOCAL（或称 AF_UNIX，Unix域socket）、AF_ROUTE等等。协议族决定了socket的地址类型，在通信中必须采用对应的地址，如AF_INET决定了要用ipv4地址（32位的）与端口号（16位的）的组合、AF_UNIX决定了要用一个绝对路径名作为地址。
// type：指定socket类型。常用的socket类型有，SOCK_STREAM、SOCK_DGRAM、 SOCK_RAW、SOCK_PACKET、SOCK_SEQPACKET等等。
// protocol：故名思意，就是指定协议。常用的协议有，IPPROTO_TCP、IPPTOTO_UDP、IPPROTO_SCTP、 IPPROTO_TIPC等，它们分别对应TCP传输协议、UDP传输协议、STCP传输协议、TIPC传输协议。
```
2. bind
```cpp
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// sockfd：即socket描述字，它是通过socket()函数创建了，唯一标识一个socket。bind()函数就是将给这个描述字绑定一个名字。
// addr：一个const struct sockaddr *指针，指向要绑定给sockfd的协议地址。这个地址结构根据地址创建socket时的地址协议族的不同而不同，如ipv4对应的是：
```
3. connect
```cpp
int listen(int sockfd, int backlog);
// listen函数的第一个参数即为要监听的socket描述字
// 第二个参数为相应socket可以排队的最大连接个数。
```
4. listen
```cpp
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
// connect函数的第一个参数即为客户端的socket描述字
// 第二参数为服务器的socket地址
// 第三个参数为socket地址的长度。
```
5. accept
```cpp
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
// accept函数的第一个参数为服务器的socket描述字
// 第二个参数为指向struct sockaddr *的指针
// 用于返回客户端的协议地址
// 第三个参数为协议地址的长度
// 如果accpet成功，那么其返回值是由内核自动生成的一个全新的描述字，代表与返回客户的TCP连接。
```
6. read&&write
```cpp
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
              const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
```

