#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include "../process/deviceipc.h"
#include <signal.h>

communicateObjct *communicate_ = nullptr;
volatile sig_atomic_t ctrlCPressed = 0;

// Signal handler function
void ctrlCHandler(int signum) {
    printf("Ctrl+C pressed. Cleaning up...\n");
    if(communicate_){
        delete communicate_;
    }
    exit(EXIT_SUCCESS);
}
void rtsp_prepare(){
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        fprintf(stderr, "Failed to register signal handler\n");
        exit(EXIT_FAILURE);
    }
    communicate_ = new communicateObjct(QUEUE_NAME_2, QUEUE_NAME_1);
    Message msg;
    msg.type_ = MSG_RTSP_SERVER_WARN;

    const char* mst_txt = "rtsp send msg\n";
    memcpy(msg.msg_text, mst_txt, strlen(mst_txt));
}

//char *addWarnInfo(float x, float y, float z, float size, char *buf)
//{
//    sprintf(buf, "%.1f %.1f %.1f %.1f ", x, y, z, size);
//    int len = strlen(buf);
//    LOG_INFO("add warn %s,%d\n", buf, strlen(buf));
//    return buf + len;
//}
char *addWarnInfo(float  x,float y, float z, float size,float center_x,float center_y,float cap_w,float cap_h,char *buf) 
{
     sprintf(buf, "%.1f %.1f %.1f %.1f %.1f %.1f %.1f %.1f ", x, y, z, size,center_x,center_y,cap_w,cap_h);
     int len = strlen(buf);
    // LOG_INFO("add warn %s,%d\n", buf, strlen(buf));
     return buf + len;
}

void test_addWarnInfo()
{
    Message msg;
    msg.type_ = MSG_RTSP_SERVER_WARN;
    char* buf = msg.msg_text;
    char* temp = addWarnInfo(1.1, 1.2, 1.3, 1.0, 100, 100, 50, 50, buf);
    char* temp1 = addWarnInfo(2.1, 2.2, 2.3, 1.0, 200, 200, 50, 50, temp);
    char* temp2 = addWarnInfo(2.1, 2.2, 2.3, 1.0, 300, 300, 50, 50, temp1);
    if(!communicate_->sendMessage(msg)){
        std::cout << " rtsp send queue is full ." << std::endl;  
    }
}
int rtspProcess(void)
{   
    rtsp_prepare();
    int index = 0;
    while (true) {
        test_addWarnInfo();
        index++;
        std::cout << "Process rtsp  doing something:" << index << std::endl;
        sleep(5);  // 休眠4秒，可以根据实际需求调整
    }

    return 0;
}
