/*
 * @Author       : mark
 * @Date         : 2020-06-17
 * @copyleft Apache 2.0
 */ 
#ifndef WEBSERVER_H
#define WEBSERVER_H



#include "epoller.h"

#include "../app/appManager.h"
#include <set>
class SocketServer {
public:
    SocketServer(
        int port, int trigMode, int timeoutMS, bool OptLinger, 
        int connPoolNum, int threadNum,
        bool openLog, int logLevel, int logQueSize);

    ~SocketServer();
    void Start();
private:
    bool InitSocket_(); 
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);
  
    void DealListen_();

    void DealWrite_(void);
    void DealRead_(int fd);

    void SendError_(int fd, const char*info);
    //void ExtentTime_(AppConn* client);
    void CloseConn_(int fd);

    static int SetFdNonblock(int fd);
    void SetFdNoDelay(int fd);
    int port_;
    bool openLinger_;
    int timeoutMS_;  /* 毫秒MS */
    bool isClose_;
    int listenFd_;
    
    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::shared_ptr<Epoller> epoller_;

    appManager *aManager_;
    std::set<int> fdSet_;
};


#endif //WEBSERVER_H