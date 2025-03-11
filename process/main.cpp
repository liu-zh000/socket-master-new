#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <mqueue.h>
#include "deviceipc.h"
volatile sig_atomic_t isRunning = 1;
pid_t pidAPP, pidRTSP, pidwebsocket;
// 信号处理函数
void handleSignal(int signo) {
    if (signo == SIGINT) {
        // 向子进程发送 SIGTERM 信号

        // 等待子进程退出
        waitpid(pidAPP, NULL, 0);
        waitpid(pidRTSP, NULL, 0);
        waitpid(pidwebsocket, NULL, 0);
        
        exit(EXIT_SUCCESS);
    }
}


int main() {
#if 1
    SocketServerProcess();
#else    
    struct sigaction sa;

    // 设置信号处理函数
    sa.sa_handler = handleSignal;
    sigaction(SIGINT, &sa, NULL);

    // 设置信号中断方式
    siginterrupt(SIGINT, 1);
        
    pid_t pidAPP = fork();
    if (pidAPP == -1) {
        perror("fork");
        return 1;
    }
    if (pidAPP == 0) {
        SocketServerProcess();
        return 0;
    }

    pid_t pidRTSP = fork();
    if (pidRTSP == -1) {
        perror("fork");
        return 1;
    }
    if (pidRTSP == 0) {
        rtspProcess(); 
        return 0; 
    }

    pid_t pidwebsocket = fork();
    if (pidwebsocket == -1) {
        perror("fork");
        return 1;
    }
    if (pidwebsocket == 0) {
        WebsocketProcess();  
        return 0;
    }

    while(true){
        std::cout << "Daemon process checking statuses..." << std::endl;
        sleep(5);  // 休眠5秒，可以根据实际需求调整
    }

    write(STDOUT_FILENO, "stop server\n", 12);
    kill(pidAPP, SIGINT);
    kill(pidRTSP, SIGINT);
    kill(pidwebsocket, SIGINT);
    // 等待所有子进程结束
    waitpid(pidAPP, nullptr, 0);
    waitpid(pidRTSP, nullptr, 0);
    waitpid(pidwebsocket, nullptr, 0);
#endif
    return 0;
}
