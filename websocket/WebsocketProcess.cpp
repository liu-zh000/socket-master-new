#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
int WebsocketProcess(void)
{
    while (true) {
        // 进程 D 逻辑
        std::cout << "Process websocket  doing something." << std::endl;
        sleep(4);  // 休眠4秒，可以根据实际需求调整
    }
    return 0;
}