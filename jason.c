// status_server.c —— 接收 JSON 状态的测试程序
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define STATUS_PORT 10000

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr, cli;
    socklen_t len = sizeof(cli);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(STATUS_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
    listen(server_fd, 5);
    printf("📡 JSON 状态服务器已启动，端口 %d\n", STATUS_PORT);

    while (1)
    {
        int cli_fd = accept(server_fd, (struct sockaddr *)&cli, &len);
        printf("客户端连接: %s\n", inet_ntoa(cli.sin_addr));

        char buf[512];
        while (1)
        {
            int n = recv(cli_fd, buf, sizeof(buf) - 1, 0);
            if (n <= 0) {
                printf("连接断开。\n");
                close(cli_fd);
                break;
            }
            buf[n] = 0;
            printf("收到状态包: %s\n", buf);
        }
    }
}
