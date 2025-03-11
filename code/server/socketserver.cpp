/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */

#include "socketserver.h"
#include <netinet/tcp.h>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../log/applog.h"
using namespace std;

SocketServer::SocketServer(
            int port, int trigMode, int timeoutMS, bool OptLinger,
            int sqlPort, const char* sqlUser, const  char* sqlPwd,
            const char* dbName, int connPoolNum, int threadNum,
            bool openLog, int logLevel, int logQueSize):
            port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
            epoller_(new Epoller())
    {
    fdSet_.clear();
    aManager_ = new appManager();
    InitEventMode_(trigMode);
    if(!InitSocket_()) { isClose_ = true;}
    auto logger = (LogManager::instance()).getLogger();
    if(openLog) {
        logger->info("VER:{}", APP_VERSION);
        if(isClose_) {
            logger->error("========== Server init error!==========");
        }
        else {
            logger->info("========== Server init ==========");
            logger->info("Port:{}", port_);
        }
    }
}

SocketServer::~SocketServer() {
    auto logger = (LogManager::instance()).getLogger();
    logger->info("socket exit");
    close(listenFd_);
    if(aManager_){
        delete aManager_;
    }
    isClose_ = true;
}

void SocketServer::InitEventMode_(int trigMode) {
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    AppConn::isET = (connEvent_ & EPOLLET);
}

void SocketServer::Start() {
    int timeMS = 10;  /* epoll wait timeout == -1 无事件将阻�?? */
    auto logger = (LogManager::instance()).getLogger();
    if(!isClose_) {
        logger->info("========== Server start ==========");
     }
    while(!isClose_) {

        int eventCnt = epoller_->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            /* 处理事件 */
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if(fd == listenFd_) {
                DealListen_();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                CloseConn_(fd);
            }
            else if(events & EPOLLIN) {
                //assert(users_.count(fd) > 0);
                DealRead_(fd);
            }
            else if(events & EPOLLOUT) {
                logger->error("error write");
            } else {
                logger->error("Unexpected event");
            }
        }
        DealMsg_();
        DealWrite_();
  
    }
}

void SocketServer::SendError_(int fd, const char*info) {
    int ret = send(fd, info, strlen(info), 0);
    auto logger = (LogManager::instance()).getLogger();
    if(ret < 0) {
        logger->warn("send error to client{} error!", fd);
    }
    close(fd);
}

void SocketServer::CloseConn_(int fd) {
    auto logger = (LogManager::instance()).getLogger();
    logger->info("server close:{}", fd);
    fdSet_.erase(fd);
    epoller_->DelFd(fd);
    aManager_->Close(fd);
}

void SocketServer::AddClient_(int fd, sockaddr_in addr) {
    int ret = aManager_->init(fd, addr);
    if(ret == -1){
        SendError_(fd, "Server busy!");
        return;
    }

    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    fdSet_.insert(fd);
}

void SocketServer::DealListen_() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if(fd <= 0)
            return;
        AddClient_(fd, addr);
    } while(listenEvent_ & EPOLLET);
}

void SocketServer::DealRead_(int fd) {
    int ret = -1;
    int readErrno = 0;
    ret = aManager_->read_(fd, &readErrno);
    //ret = client->read_(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        printf("read error:%d in %d line\n", readErrno ,__LINE__);//lz 20240406
        CloseConn_(fd);
        return;
    }
    aManager_->process(fd);
    epoller_->ModFd(fd, connEvent_ | EPOLLIN);
}
void SocketServer::DealMsg_(void){
    //for (auto it = fdSet_.begin(); it != fdSet_.end(); ++it) {
        //int fd = *it;
        aManager_->msgProcess(fdSet_);
    //}    
}
void SocketServer::DealWrite_(void) {
    int writeErrno = 0;
    for (auto it = fdSet_.begin(); it != fdSet_.end(); ++it) {
        int fd = *it;
        if(aManager_->needWrite(fd)){
            int ret = aManager_->write_(fd, &writeErrno);
            if(ret < 0 && writeErrno != EAGAIN) {
                printf("write error:%d in %d line\n", writeErrno ,__LINE__);//lz 20240406
                CloseConn_(fd);
            }
        }
    }
}

/* Create listenFd */
bool SocketServer::InitSocket_() {
    int ret;
    struct sockaddr_in addr;
    auto logger = (LogManager::instance()).getLogger();
    
    if(port_ > 65535 || port_ < 1024) {
        logger->error("Port:{} error!",  port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger optLinger = { 0 };
    if(openLinger_) {
        /* 优雅关闭: 直到所剩数�?发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd_ < 0) {
        logger->error("Create socket error :{}!", port_);
        return false;
    }

    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if(ret < 0) {
        close(listenFd_);
        logger->error("Init linger error!:{}", port_);
        return false;
    }

    int optval = 1;
    /* �?口�?�用 */
    /* �?有最后一�?套接字会正常接收数据�? */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));
    if(ret == -1) {
        logger->error("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0) {
        logger->error("Bind Port:{} error!", port_);
        close(listenFd_);
        return false;
    }

    ret = listen(listenFd_, 6);
    if(ret < 0) {
        logger->error("Listen port:{} error!", port_);
        close(listenFd_);
        return false;
    }
    ret = epoller_->AddFd(listenFd_,  listenEvent_ | EPOLLIN);
    if(ret == 0) {
        logger->error("Add listen error!");
        close(listenFd_);
        return false;
    }
    SetFdNonblock(listenFd_);
    logger->info("Server port:{}", port_);
    return true;
}

int SocketServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void SocketServer::SetFdNoDelay(int fd){
    assert(fd > 0);
    int flag = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)) == -1) {
        perror("Error setting TCP_NODELAY option");
        close(fd);
        exit(EXIT_FAILURE);
    }
}



