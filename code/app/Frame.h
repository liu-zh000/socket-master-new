#ifndef FRAME_H
#define FRAME_H
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iostream>
#include <cstdint>
#include "../log/applog.h"
#define FRAME_MAX_LEN (256 )//lz
#define FRAME_HEAD ('S')
#define FRAME_HEAD_LOC  (0)
#define FRAME_LEN_LOC_0 (1)
#define FRAME_LEN_LOC_1 (2)
#define FRAME_LEN_LOC_2 (3)
#define FRAME_LEN_LOC_3 (4)
#define FRAME_HOST_LOC (5)
#define FRAME_DST_LOC   (6)
#define FRAME_TYPE_LOC  (7)
#define FRAME_CMD_LOC   (8)
#define FRAME_PARA1 (9)
#define FRAME_BEFORE_NUM    (5)
#define FRAME_CTRL_NUM  (4)
#define FRAME_SUM_LEN   (3)
#define CMD_TEST        ('0')
#define CMD_INIT        ('1')
#define CMD_START       ('2')
#define CMD_STOP        ('3')
#define CMD_ASK_TIME    ('4')
#define CMD_ASK_STATE   ('5')
#define CMD_ASK_INFO ('6')
#define CMD_SET_TIME    ('7')
#define CMD_WARN        ('8')
#define CMD_SET_INFO    ('9')
#define CMD_ASK_INFO_PARA1_0_SOFT                   (0)
#define CMD_ASK_INFO_PARA1_1_HARDWARE       (1)
#define CMD_ASK_INFO_PARA1_2_SERNUM      (2)
#define CMD_ASK_INFO_PARA1_3_DISTANCE    (3)
#define APP_ID          ('1')
#define NANO_ID         ('3')
#define TYPE_HOST       ('0')
#define TYPE_ACK        ('1')
enum{
    FRAME_INIT,
    FRAME_OK,
    FRAME_WAIT,
    FRAME_ACK,
    FRAME_HEAD_ERROR,
    FRAME_LENGTH_ERROR,
    FRAME_CHECK_ERROR,
    FRAME_UNKNOW,
};

class Frame
{
public:
    Frame(){}
    Frame(char *ptr, int len){
        init(ptr, len);
    }
    void init(char *ptr, int len){
        auto logger = (LogManager::instance()).getLogger();
        
        
            if (ptr == nullptr || len <= 0) {
                printf("Invalid buffer\n");
                return;
            }
            printf("ASCII Len=%d:\n", len);
            for (int i = 0; i < len; ++i) {
                putchar(isprint(ptr[i]) ? ptr[i] : '.'); // 不可见字符显示为点
            }
            printf("\n");
        

        head_ = ptr[0];
        if(head_ != FRAME_HEAD){
            state_ = FRAME_HEAD_ERROR;
            return;            
        }
        length_ = (ptr[FRAME_LEN_LOC_0] - '0') * 1000 + (ptr[FRAME_LEN_LOC_1] - '0') * 100 + (ptr[FRAME_LEN_LOC_2] - '0') * 10 +  (ptr[FRAME_LEN_LOC_3] - '0');
        if(length_ > FRAME_MAX_LEN){
            state_ = FRAME_LENGTH_ERROR;
            return;
        }
        else if(length_ > len){
            state_ = FRAME_WAIT;
            return;            
        }
        else{
            host_ = ptr[FRAME_HOST_LOC];
            client_ = ptr[FRAME_DST_LOC];
            type_ = ptr[FRAME_TYPE_LOC];
            cmd_ = ptr[FRAME_CMD_LOC];
            check_ = 0;
            int last = length_ + FRAME_BEFORE_NUM;
            for(int i = FRAME_BEFORE_NUM; i < last; i++){
                check_ += ptr[i];
            }
            check_ = check_ % 1000;
            int paraLen = length_ - FRAME_CTRL_NUM;
            paras_.clear();
            char* startPtr = (char*)(ptr + FRAME_BEFORE_NUM + FRAME_CTRL_NUM);
            char* endPtr = startPtr;
            int wl = 0;
            while((*endPtr != '\n') &&(paraLen > 0)){
                paraLen--;
                if(*startPtr == ' '){
                    startPtr++;
                    endPtr = startPtr;
                    continue;
                }
                if(*endPtr != ' '){
                    endPtr++;
                    continue;
                }
                if(endPtr > startPtr){
                    std::string para(startPtr, endPtr);
                    paras_.push_back(para);
                    endPtr++;
                    startPtr =  endPtr;
                }
            }
            int sc = (ptr[last] - '0')* 100 + (ptr[last + 1]  - '0')* 10 + (ptr[last + 2] - '0');
            if(sc != check_){
                logger->info("recv error check");
                state_ = FRAME_CHECK_ERROR;
                return;
            }
            state_ = FRAME_OK;
        }
    }
    char head_;
    int length_;
    char host_;
    char client_;
    char type_;
    char cmd_;
    std::vector<std::string> paras_;
    int check_;
    int state_ = FRAME_INIT;
    int make(char* dst, char host, char client, char type, char cmd, const std::vector<std::string> &paras){
        dst[FRAME_HEAD_LOC] = FRAME_HEAD;
        dst[FRAME_HOST_LOC] = host;
        dst[FRAME_DST_LOC] = client;
        dst[FRAME_TYPE_LOC] = type;
        dst[FRAME_CMD_LOC] = cmd;
        int l = paras.size();
        char *pdst = dst + FRAME_BEFORE_NUM + FRAME_CTRL_NUM;
        int num = FRAME_BEFORE_NUM + FRAME_CTRL_NUM;
        for(int i = 0; i < l; i++){
            int paraLen = paras[i].length();
            const char *paraPtr = paras[i].c_str();
            std::memcpy(pdst, paraPtr, paraLen);
            num = num + paraLen;
            pdst = pdst + paraLen;
            *pdst = 0x20;//space
            pdst++;
            num = num + 1;
        }
        int sum = 0;
        for(int i = FRAME_BEFORE_NUM; i < num; i++){
            sum = sum + dst[i];
        }
        sum = sum % 1000;
        char check[3] = {0};
        for(int i = 0; i < 3; i++){
            check[i] = sum % 10 + '0';
            sum = sum / 10;
            *(pdst + 2 - i) = check[i];
        }
        for(int i = 0; i < 3; i++){
            *pdst = check[2 - i];
            pdst++;
        }
        *pdst = '\n';
        int tmp = num;
        for(int i = 0; i < 4; i++){
            dst[FRAME_LEN_LOC_3 - i] = tmp % 10 + '0';
            tmp = tmp / 10;
        }
        //return num + FRAME_BEFORE_NUM + FRAME_CTRL_NUM;//lz
        return num + 4;
    }

    int ack(char *dst, const std::vector<std::string> &paras){
        int l = paras.size();
        int len =  make(dst, client_, host_, TYPE_ACK, cmd_, paras);
        return len;
    }
    int size(){
        return length_ + FRAME_BEFORE_NUM + FRAME_CTRL_NUM;
    }
};

#endif