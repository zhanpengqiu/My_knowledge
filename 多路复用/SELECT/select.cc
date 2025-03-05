#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BACKLOG 10

int main() {
    int server_fd, max_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set masterfds, readfds; // masterfds保存所有需要监控的fd，readfds用于select调用
    
    // 创建socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置端口可重用
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 绑定socket到指定端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 初始化masterfds集合并加入服务器socket
    FD_ZERO(&masterfds);
    FD_SET(server_fd, &masterfds);
    max_fd = server_fd;

    printf("%d",max_fd);

    printf("Waiting for connections on port %d...\n", PORT);

    while (1) {
        // 使用masterfds副本进行select调用
        readfds = masterfds;
        // 这里timeout参数置空代表如果没有io事件到来，则一直等待，有的话就返回就绪事件的总数
        // Timeout的参数设置
        // 设置成NULL，表示如果没有 I/O 事件发生，则 select 一直等待下去。
        // 设置为非0的值，这个表示等待固定的一段时间后从 select 阻塞调用中返回。
        // 设置成 0，表示根本不等待，检测完毕立即返回。
        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // 遍历所有可能的文件描述符
        for(int i = 0; i <= max_fd; i++) {
            // 遍历所有的readfds可读事件，判断是不是有可读事件到来
            // 如有则进入下一层的判断
            // 没有则继续查看下一个fd查找其他的fd是否有可读事件到来
            if (FD_ISSET(i, &readfds)) {
                // 如果是服务器的可读事件到来（新连接建立事件）
                if (i == server_fd) {
                    // 处理新连接
                    int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                    if (new_socket < 0) {
                        perror("accept failed");
                        continue;
                    }
                    printf("New connection, socket fd is %d, ip is : %s, port: %d\n",
                           new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // 将新连接加入到监控列表中
                    FD_SET(new_socket, &masterfds);
                    if (new_socket > max_fd) {
                        max_fd = new_socket;
                    }
                } else {
                    // 这里可以处理来自客户端的数据
                    char buffer[1024] = {0};
                    // 这里的i是socket的fd，通过系统调用读取数据到buffer中
                    int valread = read(i, buffer, 1024);
                    if (valread <= 0) {
                        // 客户端关闭连接或其他错误
                        close(i);
                        FD_CLR(i, &masterfds); // 连接错误就从masterfds中移除该socket
                    } else {
                        printf("Received data: %s from socket fd: %d\n", buffer, i);
                        write(i, buffer, strlen(buffer)); // 回显给客户端
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}