#ifndef APP_MANAGER_H
#define APP_MANAGER_H
#include <unordered_map>
#include "../app/appconn.h"
#include "../log/applog.h"
//#include "BatDevice.h"
//#include "monitorVedio.h"
#include  <set>

#define  MAX_FD  (5)

class appManager{
public:

    appManager(){
        auto logger = (LogManager::instance()).getLogger();
       
        //bat_ = new BatDevice(); // new了一个电池电量检测类
        
        std::string pathVedio = "/srv/ftp/record/live/stream";
        std::string pathRecord = "/usr/local/WarningRecordDir";
        last_time_ = std::time(nullptr);
        //mv_ = new MonitorVedio(pathVedio, pathRecord);

    }

    ~appManager(){
        std::cout << "in  ~appManager()" <<std::endl;
        //if(communicate_){
         //   delete communicate_;
        //}
        //if(config_){
        //    delete config_;
        //}
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

    
    int process(int fd){
        
        int x = users_[fd].process();
        return x;        
    }

    int init(int fd, sockaddr_in addr){
        auto logger = (LogManager::instance()).getLogger();
        if(userCount >= MAX_FD){
            logger->warn("Clients is full!");
            return -1;
        }
        users_[fd].init(fd, addr);//bat_,,  communicate_
        logger->info("Client:{}({}:{}) quit, UserCount:{}", fd, users_[fd].GetIP(), users_[fd].GetPort(), userCount);
        return 0;
    }
    int needWrite(int fd){
        return users_[fd].ToWriteBytes();
    }
    int read_(int fd, int *err){
        return users_[fd].read_(err);
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
   
    //BatDevice *bat_ = nullptr; // 电池电量检测类
    //MonitorVedio *mv_ = nullptr;
};

#endif
