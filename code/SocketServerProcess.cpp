/*
 * @Author       : mark
 * @Date         : 2020-06-18
 * @copyleft Apache 2.0
 */ 
#include <unistd.h>
#include "server/socketserver.h"

#include <csignal>
#include <signal.h>
SocketServer *server = nullptr;
// Signal handler function
void serverCtrlHandler(int signum) {
    printf("socket server exit\n");
    if(server)
        delete server;
    exit(EXIT_SUCCESS);
}


int SocketServerProcess(void) {
    /* 守护进程 后台运行 */
    if (signal(SIGINT, serverCtrlHandler) == SIG_ERR) {
        fprintf(stderr, "Failed to register signal handler\n");
        
        exit(EXIT_FAILURE);
    }
    server = new SocketServer(
        9090, 3, 60000, false,             /* 端口 ET模式 timeoutMs 优雅退出  */
        /* Mysql配置 */
        12, 5, true, 1, 1024);             /* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server->Start();
    //delete appCom;
    return 0;
} 
  