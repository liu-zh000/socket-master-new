#ifndef APP_MANAGER_H
#define APP_MANAGER_H
#include <unordered_map>
#include "../app/appconn.h"
#include "../config/configProxy.h"
#include "../config/createProxy.h"
#include "../log/applog.h"
#include "../../process/deviceipc.h"
//#include "BatDevice.h"
//#include "monitorVedio.h"
#include  <set>

#define  MAX_FD  (5)

class appManager{
public:
    //RECORD_INFO row ;
    //std::string mysql_last_time = row[4] ? row[4] : " ";//lz
    //std::string mysql_last_time = alarmInfo_->update_time;//lz

    appManager(){
        auto logger = (LogManager::instance()).getLogger();
        config_ = factory_.create();
        deviceInfo_ = new RadarDeviceInfo();
        alarmInfo_ = new RadarAlarmInfo();
        config_->getMachineInfo(*deviceInfo_);
        deviceInfo_->printInfo();
        std::string id1 = deviceInfo_->id;//lz20240704
        config_->set_id(id1);//lz20240704
        config_->getAlarmInfo(*alarmInfo_);
        alarmInfo_->printInfo();
        //bat_ = new BatDevice(); // new了一个电池电量检测类
        
        std::string pathVedio = "/srv/ftp/record/live/stream";
        std::string pathRecord = "/usr/local/WarningRecordDir";
        last_time_ = std::time(nullptr);
        //mv_ = new MonitorVedio(pathVedio, pathRecord);
        communicate_ = new communicateObjct(QUEUE_NAME_1, QUEUE_NAME_2);
        logger->info("soft verson:{}", deviceInfo_->soft_version_number);
        logger->info("hard verson:{}", deviceInfo_->hard_version_number);
    }

    std::string mysql_last_time = "2023-11-08 00:48:33";
    //std::string mysql_last_time = alarmInfo_->update_time;//lz1104
    // void checkDbUpdate()//lz
    // {
    //     std::cout << "in checkDbUpdate!" << std::endl;
    //     //if(sleep(1))
    //     //{
    //         auto logger = (LogManager::instance()).getLogger();
    //         config_->getAlarmInfo(*alarmInfo_);
    //         alarmInfo_->printInfo();
    //         //std::string mysql_last_time = "2023-11-08 00:48:33";
    //         std::string mysql_now_time = alarmInfo_->update_time;
    //         logger->info("update_time:{}", mysql_now_time);
    //         logger->info("update_time_last:{}", mysql_last_time);
    //         if(mysql_now_time == mysql_last_time)
    //         {
    //             return;
    //         }
    //         else 
    //         {
                
    //             double a = alarmInfo_->detection_range;
    //             double b = alarmInfo_->clustering_threshold;
    //             int c = alarmInfo_->max_count;
    //             int d = alarmInfo_->min_count;
    //             double e = alarmInfo_->total_time;
    //             double f = alarmInfo_->two_detection_range;
    //             logger->info("detection_range:{}", a);//检测距离
    //             logger->info("clustering_threshold:{}", b);//聚类阈值
    //             logger->info("max_count:{}", c);//点云最大数目
    //             logger->info("min_count:{}", d);//点云最小数目
    //             logger->info("total_time:{}", e);//累计时间
    //             logger->info("two_detection_range:{}", f);//
    //             Message sql_msg;
    //             sql_msg.index   = 1;
    //             sql_msg.type_   = MSG_WEB_PARA_UPDATE;
    //             char *buf = sql_msg.msg_text;
    //             sprintf(buf,"%.1lf %.2lf %d %d %.1lf ",a ,b ,c ,d ,e);    
    //             if(!(communicate_->sendMessage(sql_msg)))
    //             {
    //                 std::cout << "web send  wrong !" << std::endl;
    //             }
    //             else
    //                 {std::cout << "web send  success! is: " << sql_msg.msg_text <<std::endl; }

    //             mysql_last_time =  mysql_now_time   ;

    //         }
    //     //}
    // }

    ~appManager(){
        if(config_){
            delete config_;
        }
        if(alarmInfo_){
            delete alarmInfo_;
        }
        if(deviceInfo_){
            delete deviceInfo_;
        }
        if(communicate_){
            delete communicate_;
        }
        if(config_){
            delete config_;
        }
        // if(bat_){       // 析构电池电量检测类
        //     delete(bat_);
        // }
        // if(mv_)
        //     delete mv_;
    }
    void Close(int fd){
        auto logger = (LogManager::instance()).getLogger();
        assert(users_.count(fd) > 0);
        users_[fd].Close();
        userCount--;
        logger->info("Client:{}({}:{}) quit, UserCount:{}", fd, users_[fd].GetIP(), users_[fd].GetPort(), userCount);
    }

    void getMsg(Message &msg){
        auto logger = (LogManager::instance()).getLogger();
        if(communicate_->receiveMessage(msg)){
            logger->info("recv msg:{}", msg.msg_text);
        }
        else{
            msg.type_ = -1;
        }
    }
    int process(int fd){
        //Message msg;
        //msg.type_ = -1;
        //getMsg(msg);
        int x = users_[fd].process();
        return x;        
    }

    int init(int fd, sockaddr_in addr){
        auto logger = (LogManager::instance()).getLogger();
        if(userCount >= MAX_FD){
            logger->warn("Clients is full!");
            return -1;
        }
        users_[fd].init(fd, addr, alarmInfo_, deviceInfo_, config_,  communicate_);//bat_,
        logger->info("Client:{}({}:{}) quit, UserCount:{}", fd, users_[fd].GetIP(), users_[fd].GetPort(), userCount);
        return 0;
    }
    int needWrite(int fd){
        return users_[fd].ToWriteBytes();
    }
    int read_(int fd, int *err){
        return users_[fd].read_(err);
    }

    int msgProcess(std::set<int> &fds){
        std::time_t cur = std::time(nullptr);
        //mv_->monitor();
        Message msg;
        msg.type_ = -1;
        getMsg(msg);
        
        if(msg.type_ == MSG_RTSP_SERVER_WARN){
            //mv_->recv_warning(cur);
        }
        if(cur - last_time_ >=  60){
            timeoutProcess();
            last_time_ = cur;
        }
        if(msg.type_ != -1){
            for(auto fd : fds)
            users_[fd].msgProcess(msg);
        }
            
        
    }

    void timeoutProcess(void){
        //otherthing, update state
        //checkDbUpdate();//lz
    }
    int write_(int fd, int *err){
        return users_[fd].write_(err);
    }
    
private:
    std::unordered_map<int, AppConn> users_;
    std::time_t last_time_ = -1;
    int userCount = 0;
    RadarAlarmInfo *alarmInfo_ = nullptr;
    RadarDeviceInfo *deviceInfo_ = nullptr;
    
    createProxy factory_;
    configProxy *config_ = nullptr;
    communicateObjct *communicate_;
    //BatDevice *bat_ = nullptr; // 电池电量检测类
    //MonitorVedio *mv_ = nullptr;
};

#endif
