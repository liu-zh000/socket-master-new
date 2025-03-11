#ifndef DEVICE_IPC_H
#define DEVICE_IPC_H
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../code/log/applog.h"
#define MAX_MSG_SIZE (256)//lz
#define QUEUE_NAME_1 "/my_message_queue_1"
#define QUEUE_NAME_2 "/my_message_queue_2"
enum{
    MSG_TEST = 0,
    MSG_RTSP_SERVER_WARN = 1,
    MSG_SERVER_RTSP_UPDATE = 2,
    MSG_WEB_PARA_UPDATE = 3,
    APP_SET_SHOW_MODE = 4,
};


struct Message {
    long type_;
    char msg_text[MAX_MSG_SIZE];
    long index;
};
class communicateObjct{
public:    
    communicateObjct(const char* host, const char *target):host_(host), target_(target){
        auto logger = (LogManager::instance()).getLogger();
        struct mq_attr attr;
        attr.mq_msgsize = sizeof(Message);  // 设置消息队列的最大消�?大小
        attr.mq_flags = 0;
        attr.mq_maxmsg = 10;
        sendMq_ = mq_open(target, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &attr);
        if(sendMq_ == -1){
            logger->warn("create send queue error, {}", target);
        }
        recvMq_ = mq_open(host, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &attr);
        if(recvMq_ == -1){
            logger->warn("create recv queue error, {}", host);
        }
    }
    bool sendMessage(const Message &message) {
        if (mq_send(sendMq_, reinterpret_cast<const char *>(&message), sizeof(Message), 0) == -1) {
            auto logger = (LogManager::instance()).getLogger();
            if (errno == EAGAIN) {
                logger->warn("send: is full");
                return false;
            } else {
                logger->warn("mq_send ERROR");
                exit(EXIT_FAILURE);
            }
        }
        return true;
    }

    bool receiveMessage(Message &receivedMessage) {
        if (mq_receive(recvMq_, reinterpret_cast<char *>(&receivedMessage), sizeof(Message), nullptr) == -1) {
            if (errno == EAGAIN) {
                // 消息队列为空
                //LOG_INFO("recv: is empty\n");
                return false;
            } else {
                perror("mq_receive");
                exit(EXIT_FAILURE);
            }
        }
        return true;
    }
    ~communicateObjct(){
        mq_close(sendMq_);
        mq_close(recvMq_);
        mq_unlink(host_);
        mq_unlink(target_);
    }
private:
    mqd_t sendMq_;
    mqd_t recvMq_;
    const char* host_;
    const char* target_;
};

int WebsocketProcess(void);
int rtspProcess(void);
int SocketServerProcess(void);
#endif