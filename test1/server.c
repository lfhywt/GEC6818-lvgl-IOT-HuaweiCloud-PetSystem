#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include "shared_mem.h"
#include <stdatomic.h>

#define SERVER_IP "192.168.171.60"
#define SERVER_PORT_CTRL 10000
#define SERVER_PORT_IMG 10001

typedef struct {
    int ctrl_fd;
    atomic_bool is_connected;
} CtrlContext;

static bool print_enable = false;
static bool g_quit = false;
static pthread_mutex_t shared_mutex = PTHREAD_MUTEX_INITIALIZER;
SharedData *shared = NULL;
static int global_img_fd = -1;

ssize_t send_all(int sock, const void *buf, size_t len) {
    size_t total = 0;
    while (total < len && !g_quit) {
        ssize_t n = send(sock, (char *)buf + total, len - total, 0);
        if (n <= 0) return n;
        total += n;
    }
    return total;
}

int parse_status_json(const char *buf, int *light, int *kong, int *camera, int *feed) {
    if (!buf) return -1;
    int ret = sscanf(buf, "{\"light\":%d,%*[^0-9]%d,%*[^0-9]%d,%*[^0-9]%d}", light, kong, camera, feed);
    return (ret == 4) ? 0 : -1;
}

void *recv_status_thread(void *arg) {
    CtrlContext *ctx = (CtrlContext *)arg;
    int fd = ctx->ctrl_fd;
    char buf[512];
    while (atomic_load(&ctx->is_connected) && !g_quit) {
        int n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            printf("⚠️ 开发板断开连接（接收线程）\n");
            atomic_store(&ctx->is_connected, false);
            break;
        }
        buf[n] = '\0';
        pthread_mutex_lock(&shared_mutex);
        if (parse_status_json(buf, &shared->light_flag, &shared->kong_flag, &shared->camera_flag, &shared->feed_flag) == 0) {
            if (print_enable)
                printf("✅ 解析 -> light=%d, kong=%d, camera=%d, feed=%d\n", shared->light_flag, shared->kong_flag, shared->camera_flag, shared->feed_flag);
        }
        pthread_mutex_unlock(&shared_mutex);
    }
    return NULL;
}

void *report_status_thread(void *arg) {
    CtrlContext *ctx = (CtrlContext *)arg;
    int fd = ctx->ctrl_fd;
    struct timeval last_time, now;
    gettimeofday(&last_time, NULL);

    while (atomic_load(&ctx->is_connected) && !g_quit) {
        if (fd <= 0) break;
        gettimeofday(&now, NULL);
        double elapsed = (now.tv_sec - last_time.tv_sec) + (now.tv_usec - last_time.tv_usec) / 1000000.0;

        if (elapsed >= 1.0) {
            last_time = now;
            char json_buf[256];
            pthread_mutex_lock(&shared_mutex);
            snprintf(json_buf, sizeof(json_buf), "{\"light\":%d,\"kong\":%d,\"camera\":%d,\"feed\":%d}", shared->light_flag, shared->kong_flag, shared->camera_flag, shared->feed_flag);
            pthread_mutex_unlock(&shared_mutex);

            ssize_t sent = send_all(fd, json_buf, strlen(json_buf));
            if (sent <= 0) {
                printf("⚠️ 发送失败，连接已断开（上报线程）\n");
                break;
            }
            if (print_enable) printf("📤 上报状态: %s\n", json_buf);
        }
        usleep(1000);
    }
    return NULL;
}

void *control_channel_handler(void *arg) {
    int server_ctrl = *(int *)arg;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    while (!g_quit) {
        int ctrl_fd = accept(server_ctrl, (struct sockaddr *)&client_addr, &len);
        if (ctrl_fd < 0) {
            if (g_quit) break;
            perror("accept ctrl");
            sleep(1);
            continue;
        }
        printf("✅ 控制通道已连接：%s\n", inet_ntoa(client_addr.sin_addr));

        CtrlContext *ctx = malloc(sizeof(CtrlContext));
        ctx->ctrl_fd = ctrl_fd;
        atomic_init(&ctx->is_connected, true);

        pthread_t send_tid, recv_tid;
        pthread_create(&send_tid, NULL, report_status_thread, ctx);
        pthread_create(&recv_tid, NULL, recv_status_thread, ctx);

        pthread_join(recv_tid, NULL);
        pthread_join(send_tid, NULL);

        atomic_store(&ctx->is_connected, false);
        close(ctx->ctrl_fd);
        free(ctx);
        printf("控制通道断开，等待重连...\n");
    }
    return NULL;
}

void send_image(int client_fd, const char *filename) {
    if (client_fd < 0) {
        printf("❌ 图片通道未连接\n");
        return;
    }
    struct stat st;
    if (stat(filename, &st) != 0) {
        perror("文件不存在");
        return;
    }
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("打开文件失败");
        return;
    }
    char cmd[16] = "IMG_ADD";
    if (send_all(client_fd, cmd, sizeof(cmd)) <= 0) {
        perror("发送IMG_ADD失败");
        fclose(fp);
        return;
    }
    char basename[128] = {0};
    const char *p = strrchr(filename, '/');
    strcpy(basename, p ? p + 1 : filename);
    if (send_all(client_fd, basename, sizeof(basename)) <= 0) {
        perror("发送文件名失败");
        fclose(fp);
        return;
    }
    int filesize = st.st_size;
    if (send_all(client_fd, &filesize, sizeof(filesize)) <= 0) {
        perror("发送文件大小失败");
        fclose(fp);
        return;
    }
    char buf[1024];
    int n, sent = 0;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0 && !g_quit) {
        if (send_all(client_fd, buf, n) <= 0) {
            perror("发送文件内容失败");
            break;
        }
        sent += n;
    }
    fclose(fp);
    printf("✅ 已发送图片: %s (%d 字节)\n", basename, sent);
}

void delete_image(int client_fd, const char *filename) {
    if (client_fd < 0) {
        printf("❌ 图片通道未连接\n");
        return;
    }
    char cmd[16] = "IMG_DEL";
    if (send_all(client_fd, cmd, sizeof(cmd)) <= 0) {
        perror("发送IMG_DEL失败");
        return;
    }
    int name_len = strlen(filename);
    if (send_all(client_fd, &name_len, sizeof(name_len)) <= 0) {
        perror("发送文件名长度失败");
        return;
    }
    if (send_all(client_fd, filename, name_len) <= 0) {
        perror("发送文件名失败");
        return;
    }
    printf("🗑️ 已发送删除指令: %s\n", filename);
}

void *image_channel_handler(void *arg) {
    int server_img = *(int *)arg;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    while (!g_quit) {
        if (global_img_fd < 0) {
            int new_img_fd = accept(server_img, (struct sockaddr *)&client_addr, &len);
            if (new_img_fd < 0) {
                if (g_quit) break;
                perror("accept 图片通道");
                sleep(1);
                continue;
            }
            printf("✅ 图片通道已连接：%s\n", inet_ntoa(client_addr.sin_addr));
            global_img_fd = new_img_fd;
        } else {
            char ping[5] = "PING";
            if (send_all(global_img_fd, ping, sizeof(ping)) <= 0) {
                perror("图片通道连接断开");
                close(global_img_fd);
                global_img_fd = -1;
            }
            sleep(1);
        }
    }

    if (global_img_fd >= 0) {
        close(global_img_fd);
        global_img_fd = -1;
    }
    return NULL;
}

int main() {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    shared = (SharedData *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }
    memset(shared, 0, sizeof(SharedData));
    printf("✅ shared attached at %p\n", (void *)shared);

    int server_ctrl = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_ctrl, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr_ctrl, addr_img;
    addr_ctrl.sin_family = AF_INET;
    addr_ctrl.sin_port = htons(SERVER_PORT_CTRL);
    addr_ctrl.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (bind(server_ctrl, (struct sockaddr *)&addr_ctrl, sizeof(addr_ctrl)) < 0) {
        perror("bind 控制通道失败");
        return -1;
    }
    listen(server_ctrl, 1);
    printf("🚀 控制通道启动，等待连接...\n");

    int server_img = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_img, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr_img.sin_family = AF_INET;
    addr_img.sin_port = htons(SERVER_PORT_IMG);
    addr_img.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (bind(server_img, (struct sockaddr *)&addr_img, sizeof(addr_img)) < 0) {
        perror("bind 图片通道失败");
        return -1;
    }
    listen(server_img, 1);
    printf("🚀 图片通道启动，等待连接...\n");

    pthread_t ctrl_thread;
    if (pthread_create(&ctrl_thread, NULL, control_channel_handler, &server_ctrl) != 0) {
        perror("创建控制通道线程失败");
        return -1;
    }

    pthread_t img_thread;
    if (pthread_create(&img_thread, NULL, image_channel_handler, &server_img) != 0) {
        perror("创建图片通道线程失败");
        close(server_ctrl);
        close(server_img);
        return -1;
    }

    while (1) {
        printf("\n============================\n");
        printf("1. 发送图片\n");
        printf("2. 删除图片\n");
      //  printf("3. 修改设备状态\n");
      //  printf("4. 退出程序\n");
        printf("3. 开启打印\n");
        printf("4. 关闭打印\n");
        printf("============================\n");
        printf("请输入操作编号: ");

        int choice = -1;
        scanf("%d", &choice);

        if (choice == 1 || choice == 2) {
            int wait_count = 0;
            while (global_img_fd < 0 && wait_count < 5) {
                printf("等待图片通道连接...(%d/5)\n", wait_count + 1);
                sleep(1);
                wait_count++;
            }
            if (global_img_fd < 0) {
                printf("❌ 图片通道未连接，操作取消。\n");
                continue;
            }
        }

        if (choice == 1) {
            char path[256];
            printf("请输入要发送的图片路径: ");
            scanf("%s", path);
            send_image(global_img_fd, path);
        } else if (choice == 2) {
            char name[128];
            printf("请输入要删除的图片名(不带路径): ");
            scanf("%s", name);
            delete_image(global_img_fd, name);
        } else if (choice == -1) {
            // printf("请输入状态 (light kong camera feed 0/1)：\n");
            // pthread_mutex_lock(&shared_mutex);
            // printf("light: ");
            // scanf("%d", (int *)&shared->light_flag);
            // printf("kong: ");
            // scanf("%d", (int *)&shared->kong_flag);
            // printf("camera: ");
            // scanf("%d", (int *)&shared->camera_flag);
            // printf("feed: ");
            // scanf("%d", (int *)&shared->feed_flag);
            // pthread_mutex_unlock(&shared_mutex);
        } else if (choice == -1) {
            // printf("服务器退出中...\n");
            // g_quit = true;
            // break;
        } else if (choice == 3) {
            print_enable = true;
            printf("✅ 已开启打印\n");
        } else if (choice == 4) {
            print_enable = false;
            printf("🛑 已关闭打印\n");
        } else {
            printf("无效输入。\n");
        }
    }

    pthread_join(ctrl_thread, NULL);
    pthread_join(img_thread, NULL);

    close(server_img);
    close(server_ctrl);
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);
    pthread_mutex_destroy(&shared_mutex);
    printf("服务器已退出。\n");
    return 0;
}