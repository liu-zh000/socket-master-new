/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 
#include "appconn.h"
#include <exception>
#include "../log/applog.h"
#include <time.h>
#include <string>
//#include "BatDevice.h"

#include "StorageRecord.h"

#define WORK_PATH "/home/hhu/socket-master-new/1"

using namespace std;

const char* AppConn::srcDir;
int batVlast = 100;
int charge_times = 0;

bool AppConn::isET;
    
AppConn::AppConn() {
    fd_ = -1;
    addr_ = { 0 };
    isClose_ = true;
};

AppConn::~AppConn() { 
    Close(); 

};

void AppConn::init(int fd, const sockaddr_in& addr, RadarAlarmInfo *alarmInfo, 
    RadarDeviceInfo *deviceInfo, configProxy *config) {///, communicateObjct *communicate
//BatDevice *bat,
    assert(fd > 0);
    addr_ = addr;
    fd_ = fd;
    isClose_ = false;
    alarmInfo_ = alarmInfo;
    deviceInfo_ = deviceInfo;
    config_ = config;
    //bat_ = bat;     // 电池电量检测类
    //communicate_ = communicate;//lz
}

void AppConn::SendCmd(char* arr, int len)
{
    auto logger = (LogManager::instance()).getLogger();
    std::string str(arr, len);
    logger->info("send:{}", str);
    writeBuff_.append(arr, len);
}
void AppConn::Close() {
    if(isClose_ == false){
        isClose_ = true; 
        close(fd_);
    }
}

int AppConn::GetFd() const {
    return fd_;
}

struct sockaddr_in AppConn::GetAddr() const {
    return addr_;
}

const char* AppConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}

int AppConn::GetPort() const {
    return addr_.sin_port;
}

int AppConn::read_(int* saveErrno) {
    char tempAarrys[FRAME_MAX_LEN];
    int len = FRAME_MAX_LEN;
    int sum = 0;
    do{
        len = read(fd_, tempAarrys, FRAME_MAX_LEN);
        *saveErrno = errno;

        if(len > 0){
            readBuff_.append(tempAarrys, len);
            sum = sum + len;
        }
        if(*saveErrno == EAGAIN){
            break;
        }
    }while(len > 0);
    return sum;
}

int AppConn::write_(int* saveErrno) {
    int sum = writeBuff_.length();
    
    char arrays[FRAME_MAX_LEN];
    if(sum <= 0){
        return 0;
    }
    do {
        int frameLen = sum < FRAME_MAX_LEN ? sum : FRAME_MAX_LEN;
        writeBuff_.pop(arrays, frameLen);
        int len = write(fd_, arrays, frameLen);
        if(len <= 0) {
            *saveErrno = errno;
            break;
        }
        sum = sum - len;
    } while(isET || sum > 0);
    return sum;
}

int AppConn::getFrameFromBuffer(Frame &frame, char* bytes, int len)
{
    int discard = 0;
    frame.init(bytes, len);
    while((frame.state_ != FRAME_OK) && (len > 0))
    {
        if(frame.state_ == FRAME_WAIT)
            break;  
        bytes++;
        discard++;
        len--;
        frame.init(bytes, len);        
    }
    return discard;
}
void AppConn::msgProcess(Message &msg)
{
    switch (msg.type_)
    {
    case MSG_RTSP_SERVER_WARN:
        warn(msg.msg_text);
        break;
    
    default:
        break;
    }
}
bool AppConn::process(void) {
    auto logger = (LogManager::instance()).getLogger();
    //msgProcess(msg);
    char bytes[FRAME_MAX_LEN];
    int len = readBuff_.length();
    len = (len < FRAME_MAX_LEN) ? len : FRAME_MAX_LEN;
    if(len <= 0) {
        return false;
    }
    //len > FRAME_MAX_LEN, may cause error
    readBuff_.peek(bytes, len);
    //logger->info("recv************************************************");
    Frame frame;
    int discard = getFrameFromBuffer(frame, bytes, len);
    //logger->info("get          recv************************************************");
    if(frame.state_ != FRAME_OK){
        readBuff_.pop(bytes, discard);
        logger->warn("1discard:{}", discard);
        return false;
    }
    if(discard > 0){
        readBuff_.pop(bytes, discard);
        logger->warn("2discard:{}", discard);
    }
    //logger->info("recv:{}", str);
    len = frame.size();
    frameProcess(frame);
    readBuff_.pop(bytes, len);
    std::string str(bytes, len);
    logger->info("recv:{}", str);
    return true;
}



FILE *fp_channel;
char channle_buffer[32];

void AppConn::cmd_ask_info(Frame &frame)
{
    Frame nfame;
    std::string which = frame.paras_[0];

    std::vector<string> paras;
    paras.push_back(which);
    int infoType = std::stod(frame.paras_[0]);
    std::string dis ;
    std::string dis_two ;//lz1104
    std::string version;
    switch(infoType){
        case CMD_ASK_INFO_PARA1_3_DISTANCE:
            
            dis_two = std::to_string(alarmInfo_->two_detection_range);//lz1104
            dis_two = dis_two.substr(0, 3);//lz1104
            paras.push_back(dis_two);//lz1104
            dis =  std::to_string(alarmInfo_->detection_range);
            dis = dis.substr(0, 3);
            paras.push_back(dis);
            //std::cout<<"alarmInfo_->two_detection_range:"<<alarmInfo_->two_detection_range<<" and "<<dis_two<<"  alarmInfo_->detection_range:"<<dis<<std::endl;
        break;
        case CMD_ASK_INFO_PARA1_0_SOFT:
            version = deviceInfo_->soft_version_number;
            paras.push_back(version);
        break;
        case CMD_ASK_INFO_PARA1_1_HARDWARE:
            version = deviceInfo_->hard_version_number;
            paras.push_back(version);
        break; 
        case CMD_ASK_INFO_PARA1_2_SERNUM:
            version = deviceInfo_->serial_number;
            paras.push_back(version);
        break; 
        case 4://信道
                           // std::cout << "get channel command*********************************************** " <<std::endl;
                                // 执行 get.sh 并获取输出
                            fp_channel = popen("/home/hhu/socket-master-new/1", "r");
                            if (fp_channel == NULL) {
                                perror("popen failed");
                                //return 1;
                            }

                            // 读取输出
                            while (fgets(channle_buffer, sizeof(channle_buffer), fp_channel) != NULL) {
                                printf("Output from get.sh: %s", channle_buffer);
                            }
                          
                            
			     channle_buffer[strcspn(channle_buffer, "\n")] = '\0'; 

                            // 关闭管道
                            if (pclose(fp_channel) == -1) {
                                perror("pclose failed");
                               // return 1;
                            }
                            
                             if(channle_buffer[0] == 'a')
                            {
                                char new_channel_buffer[32] = "99";
                                version = new_channel_buffer;
                            }
                            else{
                                version = channle_buffer;
                            }
                            
           
            paras.push_back(version);
        break; 
        default:
        break;
    }

    char arr[FRAME_MAX_LEN];
    int len = nfame.make(arr, NANO_ID, APP_ID, TYPE_ACK, CMD_ASK_INFO, paras);
    SendCmd(arr,len);
    // writeBuff_.append(arr, len);

}
//时间：2023-12-10 23:15:00
//S1513071702221300 682\n
//hex 53 31 35 31 33 30 37 31 37 30 32 32 32 31 33 30 30 20 36 38 32 0A
void AppConn::cmd_set_time(Frame &frame)
{
    int t = std::stoi(frame.paras_[0]);
    int cur = setTime(t);
    std::string curStr = std::to_string(cur);
    std::vector<string> paras;
    paras.push_back(curStr);
    Frame nfame;
    char arr[FRAME_MAX_LEN];
    int len = nfame.make(arr, NANO_ID, APP_ID, TYPE_ACK, CMD_ASK_TIME, paras);
    writeBuff_.append(arr, len);
}


void AppConn::frameProcess(Frame &frame)
{
    char cmd = frame.cmd_;
    char buf[FRAME_MAX_LEN] = {0};
    std::vector<std::string> paras;
    paras.push_back("0");
    int len = 0;
    if(frame.type_ == TYPE_ACK)
        return;
    switch (cmd)
    {
    case CMD_TEST        :
    case CMD_INIT        :
    case CMD_START       :
    case CMD_STOP        :
    case CMD_ASK_TIME    :
        len = frame.ack(buf, paras);
        ack_(buf, len);
        break;
    case  CMD_ASK_STATE   :
        cmd_state(frame);
        break;
    case CMD_ASK_INFO :
        cmd_ask_info(frame);
        break;
    case CMD_SET_TIME    :
        cmd_set_time(frame);
        break;
    case CMD_SET_INFO:
        cmd_set_info(frame);
        break;
    default:
        break;
    }
}

void AppConn::cmd_set_info(Frame &frame)
{
    auto logger = (LogManager::instance()).getLogger();       
    int t = std::stoi(frame.paras_[0]);
    std::string a = frame.paras_[0];
    std::string b = "1";
    if(t  == 0 )
    {
         b = "0";
    }
    else if(t  == 1)//"START SLAM!
    {
        char command[256];
        snprintf(command, sizeof(command), "/home/hhu/start/run.sh");
        int result = system(command);
        if (result == -1)
        {
            std::cout << "EXC FAILED !command :" << command << std::endl;
            b = "1";
        }
        else
        {
            b = "0";
        }
        std::cout << "START SLAM!" << std::endl;
    }
    else if (t == 2)//SAVE COLOR_MAP AND STOP
    { 
        char command1[256];
        snprintf(command1, sizeof(command1), "/home/hhu/start/save.sh");
        int result1 = system(command1);
        if (result1 == -1)
        {
            std::cout << "EXC FAILED !command1 :" << command1 << std::endl;
            b = "1";
        }
        else
        {
            b = "0";
        }
        std::cout << "SAVE COLOR_MAP!" << std::endl;

        char command[256];
        snprintf(command, sizeof(command), "/home/hhu/start/kill.sh");
        int result = system(command);
        if (result == -1)
        {
            std::cout << "EXC FAILED !command :" << command << std::endl;
            b = "1";
        }
        else
        {
            b = "0";
        }
        std::cout << "KILL!" << std::endl;

    }
    else if (t == 3)//SAVE NO_COLOR_MAP
    {

        //int cmd_ = std::stoi(frame.paras_[1]);
        // 执行
        char command_1[256];
        // 构建命令字符串
        snprintf(command_1, sizeof(command_1), "/home/hhu/start/save-no-color.sh");

        // if(channel_to_set == 99)
        // {
        //     snprintf(command_1, sizeof(command_1), "/home/jetson/radar/socket-server-master/code/app/set_bg_channel.sh auto");//lz1104
        // }
        // else  int result = system(command_1);
        // {
        //     snprintf(command_1, sizeof(command_1), "/home/jetson/radar/socket-server-master/code/app/set_bg_channel.sh %s", frame.paras_[1].c_str());
        // }
        int result_1 = system(command_1);
        if (result_1 == -1)
        {
            std::cout << "EXC FAILED !command_1 :" << command_1 << std::endl;
            b = "1";
        }
        else
        {
            b = "0";
        }
       std::cout << "SAVE NO_COLOR_MAP!" << std::endl;
        char command[256];
        // 构建命令字符串
        snprintf(command, sizeof(command), "/home/hhu/start/kill.sh");
        int result = system(command);
        if (result == -1)
        {
            std::cout << "EXC FAILED !command :" << command << std::endl;
            b = "1";
        }
        else
        {
            b = "0";
        }
        std::cout << "KILL!" << std::endl;
    }
    
    std::vector<string> paras; 
    paras.push_back(a);
    paras.push_back(b); 
    Frame nfame;
    char arr[FRAME_MAX_LEN];
    int len = nfame.make(arr, NANO_ID, APP_ID, TYPE_ACK,CMD_SET_INFO , paras);
    // std::string str(arr, len);
    // logger->info("send:{}", str);
    // writeBuff_.append(arr, len);
    SendCmd(arr, len);

}

 void AppConn::cmd_state(Frame &frame)
 {
    auto logger = (LogManager::instance()).getLogger();
    std::vector<string> paras;

    // StorageRecord x(WORK_PATH);
    // int state = 0;
    // int fy = x.getStorageMB(WORK_PATH, FREE_TYPE);
    // int ut = x.getStorageMB(WORK_PATH, USED_TYPE);
    // //printf("free:%dMB, used:%dMB", fy, ut);
    // int saa = static_cast<int>(std::round(static_cast<float>(ut) /static_cast<float>( fy + ut) * 100));
    // std::string sa = std::to_string(saa);
    // if (sa.length() >= 3) {
	// 	sa = sa.substr(0, 3);
	// }
	// else
	// 	sa = "0" + sa;
    // //("sa:{} ", sa);

    // //logger->info("bat_detect:{}V ", bat_->getVrevised());
    // float bat_val_num = bat_->getVrevised();

    // //LWR function:qupingjun
    // static int read_count = 0;
    // static vector<double> sum_bat_num;
    // sum_bat_num.push_back(bat_val_num);
    // read_count++;
    // if(read_count>=10)
    // {
    //     double sum_bat = 0;
    //     for(double num : sum_bat_num)
    //     {
    //         sum_bat+=num;
    //     }
    //     bat_val_num = sum_bat/10;
    //     sum_bat_num.erase(sum_bat_num.begin());
    // }
    // else
    // {
    //     bat_val_num = sum_bat_num.front();
    // }

    // static float min_bat_val_num = 1000.0;
    // min_bat_val_num = (min_bat_val_num > bat_val_num)? bat_val_num : min_bat_val_num;//tem_bat_val_num取最小值
    // bat_val_num = min_bat_val_num;

    // if (bat_val_num >= 16.4)//电池电压满电范围16.8-16.4，高于16.4默认100%
    // {
    //     bat_val_num = 16.4;
    // }
    // else if(bat_val_num <= 12.8)
    // {
    //     bat_val_num = 12.8;
    // }
    
    // int bat_val;
    // if(bat_val_num > 15.52)
    // {
    //     bat_val = (int)(75 + ((bat_val_num - 15.52) / (16.4 - 15.52) * 25));
    // }
    // else if(bat_val_num > 14.64)
    // {
    //     bat_val = (int)(50 + ((bat_val_num - 14.64) / (15.52 - 14.64) * 25));
    // }
    // else if(bat_val_num > 13.60)
    // {
    //     bat_val = (int)(25 + ((bat_val_num - 13.60) / (14.64 - 13.60) * 25));
    // }
    // else
    // {
    //     bat_val =         (int)((bat_val_num - 12.8) / (13.60 - 12.8) * 25);
    // }
    
    // std::string bat_cal = std::to_string(bat_val);
    // if (bat_cal.length() >= 3) {
	// 	bat_cal = bat_cal.substr(0, 3);
	// }
	// else if(bat_cal.length() == 2)
	// 	bat_cal = "0" + bat_cal;
    // else 
	// 	bat_cal = "00" + bat_cal;

    // //logger->info("bat_cal:{} ", bat_cal);

    std::string a = "0";
    // // std::string co = "050";
    // //std::string sa = "060";
    // paras.push_back(a);
    // paras.push_back(bat_cal);//电池电量
    // paras.push_back(sa);//储存容量
    paras.push_back(a);
    paras.push_back("050");//电池电量
    paras.push_back("050");//储存容量 
    Frame nfame;
    char arr[FRAME_MAX_LEN];
    int len = nfame.make(arr, NANO_ID, APP_ID, TYPE_ACK, CMD_ASK_STATE, paras);
    // std::string str(arr, len);
    // logger->info("send:{}", str);
    // writeBuff_.append(arr, len);
    SendCmd(arr, len);
 }

void AppConn::ack_(const char *arrays, int len)
{
    auto logger = (LogManager::instance()).getLogger();
    int errorno = -1;
    std::string str(arrays, len);
    logger->info("send:{}", str);
    writeBuff_.append(arrays, len);
    //SendCmd(arrays, len);
}

void AppConn::warn(const char* info)
{
    Frame frame;
    auto logger = (LogManager::instance()).getLogger();
    std::string infos(info);
    std::istringstream iss(infos);
    std::vector<std::string> tokens;
    std::vector<std::string> paras;
    std::string token;
    if (infos.length() > FRAME_MAX_LEN) {
        infos = infos.substr(0, FRAME_MAX_LEN);
        logger->warn("warning is too long");
        std::string num = "1";
        std::string x = "0";
        std::string y = "0";
        std::string z = "0";
        std::string size = "1";
        std::string center_x = "1";
        std::string center_y = "1";
        std::string cap_w = "1";
        std::string cap_h = "1";
        tokens.push_back(num);
        tokens.push_back(x);
        tokens.push_back(y);
        tokens.push_back(z);
        tokens.push_back(size);
        tokens.push_back(center_x);
        tokens.push_back(center_y);
        tokens.push_back(cap_w);
        tokens.push_back(cap_h);
    }
    else{
        while (iss >> token) {
            tokens.push_back(token);
        }
        int tokens_l = tokens.size();
        if(tokens_l % 8){
            logger->warn("error warn info");
            std::string num = "1";
            std::string x = "0";
        std::string y = "0";
        std::string z = "0";
        std::string size = "1";
        std::string center_x = "1";
        std::string center_y = "1";
        std::string cap_w = "1";
        std::string cap_h = "1";
        tokens.push_back(num);
        tokens.push_back(x);
        tokens.push_back(y);
        tokens.push_back(z);
        tokens.push_back(size);
        tokens.push_back(center_x);
        tokens.push_back(center_y);
        tokens.push_back(cap_w);
        tokens.push_back(cap_h);
        }
        else{
            int x  = tokens_l / 8;
            std::string gp = std::to_string(x);
            paras.push_back(gp);
            for(auto it = tokens.begin(); it != tokens.end(); it++){
                paras.push_back(*it);
            }
        }

    }
    char arr[FRAME_MAX_LEN];
    int len = frame.make(arr, NANO_ID, APP_ID, TYPE_HOST, CMD_WARN, paras);
    logger->info("WARNING");
    std::string str(arr, len);
    logger->info("send:{}", str);
    //SendCmd(arr, len);
    int sl = writeBuff_.append(arr, len);
    if(sl != len){
        logger->warn("write buffer append error");
    }
}

int AppConn::setTime(int recvTime) {
    struct timespec ts;
    ts.tv_sec = recvTime;
    ts.tv_nsec = 0;
    auto logger = (LogManager::instance()).getLogger();
    if (clock_settime(CLOCK_REALTIME, &ts) == 0) {
        logger->info("System clock set successfully.");

        // 获取设置后的系统时钟时间戳
        clock_gettime(CLOCK_REALTIME, &ts);
        long int newTimestamp = ts.tv_sec;

        logger->info("Current system clock timestamp: {}", newTimestamp);

        return newTimestamp;
    } else {
        perror("clock_settime");
        return -1; // 返回一个错误码或者特殊值表示设置失败
    }
}
