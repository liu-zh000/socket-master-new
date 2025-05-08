/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef APP_CONN_H
#define APP_CONN_H

#include <sys/types.h>
#include <sys/socket.h>     // readv/writev
#include <arpa/inet.h>   // sockaddr_in
#include <stdlib.h>      // atoi()
#include <errno.h>      

#include "../buffer/CBuffer.h"
#include "Frame.h"


//#include "BatDevice.h"
class AppConn {
public:
    AppConn();

    ~AppConn();

    void init(int sockFd, const sockaddr_in& addr);//BatDevice *bat ,communicateObjct *communicate

    void SendCmd(char* arr, int len);

    int read_(int* saveErrno);
    int write_(int* saveErrno);
    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;
    
    sockaddr_in GetAddr() const;
    void warn(void);
    void warn(const char* info);
    static bool isET;
    static const char* srcDir;
    int ToWriteBytes(void){
        return writeBuff_.length();
    }

    bool process(void);
    //void msgProcess(Message &msg);

    int getFrameFromBuffer(Frame &frame, char* bytes, int len);
    void frameProcess(Frame &frame);
    void ack_(const char *arrays, int len);
    void tick(){
        m_ticks++;
    }
    void cmd_ask_info(Frame &frame);
    void cmd_set_info(Frame &frame);
    void cmd_set_time(Frame &frame);
    void cmd_state(Frame &frame);
    int setTime(int recvTime);
    
private:
    int Toget_capacity();
    float Toget_voltage();
    int fd_;
    struct  sockaddr_in addr_;
    bool isClose_;


    
    CBuffer readBuff_; // 读缓冲区
    CBuffer writeBuff_; // 写缓冲区
    uint32_t m_ticks = 0;
   
    //BatDevice *bat_;    // 电池电量检测类

};


#endif //APP_CONN_H