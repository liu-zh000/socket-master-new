#ifndef __MONITOR_DIRECTORY_H
#define __MONITOR_DIRECTORY_H
#include <sys/inotify.h>
#include <limits.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <ctime>
#include <mqueue.h>
#include <fcntl.h>
#include <cstring> 
#include <cerrno>
#include <fstream> 
#include <filesystem>
#include <sstream>
#include <utility>
#include <iomanip>
enum{
    RECORDED_VEDIO,
    INIT_RECORD
};

struct VedioRecord{
    std::time_t file_t_ = -1;
    std::string file_name_ = "A";
    bool unwarning_ = true;
    int state_ = INIT_RECORD;
};

class MonitorVedio{
public:
    MonitorVedio() = delete;
    MonitorVedio(const std::string& path, const std::string& log_file_path) : orgin_path_(path), log_file_path_(log_file_path), inotify_fd_(-1), watch_fd_(-1) {
        date_init();
        init();
    }

    ~MonitorVedio() {
        release_resource();
    }

    void monitor() {
        if (inotify_fd_ < 0 || watch_fd_ < 0) {
            init();
            if(inotify_fd_ < 0 || watch_fd_ < 0){
                //std::cout << "Monitoring not started due to initialization failure." << std::endl;
                return;
            }
        }

        char buffer[1024 * ((sizeof(struct inotify_event)) + NAME_MAX + 1)];
        ssize_t length = read(inotify_fd_, buffer, sizeof(buffer));
        if (length < 0) {
            if (errno == EINTR) {
                return;
            }
            //std::cerr << "read failed: " << strerror(errno) << std::endl;
            return;
        }       
        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];      
            if (event->mask & IN_MOVED_TO) {
                std::string file = event->name;
                mov_to_action(file);

            }
            if(event->mask & IN_MOVED_TO){
                std::string file = event->name;
                create_action(file);
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }

    void recv_warning(std::time_t t){
        if(t < last_vedio_.file_t_){
            std::cout << "e-" << "Function: " << __FUNCTION__ << ", Line: " << __LINE__ << std::endl;
        }
        cur_vedio_.unwarning_ = false;
        cur_vedio_.file_t_ = t;
    }
    void create_action(std::string &file_name){
        last_vedio_.unwarning_ = cur_vedio_.unwarning_;
        cur_vedio_.unwarning_ = true;
    }
    void mov_to_action(std::string &file_name){

        if(last_vedio_.unwarning_){
            std::string file_path = vedio_dir_ + file_name;
            if (access(file_path.c_str(), F_OK) == 0) {
                remove(file_path.c_str());
            }
            else {
                std::cout << "File does not exist.\n";
            }
        }
        else{
            log_file_ << file_name << std::endl;
            log_file_.flush();
        }
        if_next_day();
    }
private:
    void date_init(void){
        current_day_ = get_current_date_string();
        vedio_dir_ = orgin_path_ + "/" + current_day_ + "/";
        openLogFileInAppendMode();
    }

    void release_resource(void){
        if (watch_fd_ >= 0) {
            inotify_rm_watch(inotify_fd_, watch_fd_);
        }
        if (inotify_fd_ >= 0) {
            close(inotify_fd_);
        }
        if (log_file_.is_open()) {
            log_file_.close();
        }
        std::cout << "Resources released." << std::endl;
    }

    void if_next_day(void){
        std::string today = get_current_date_string();
        if(today != current_day_){
            release_resource();
            date_init();
            init();
        }
    }

    void init(){
        if(inotify_fd_ == -1){
            inotify_fd_ = inotify_init();
            if (inotify_fd_ < 0) {
                std::cerr << "inotify_init failed: " << strerror(errno) << std::endl;
                return;
            }
        }
        int flags = fcntl(inotify_fd_, F_GETFL, 0);
        if (flags == -1) {
            std::cerr << "Failed to get flags for inotify_fd_" << std::endl;
        }
        if (fcntl(inotify_fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
            std::cerr << "Failed to set non-blocking mode for inotify_fd_" << std::endl;
        }
        
        watch_fd_ = inotify_add_watch(inotify_fd_, vedio_dir_.c_str(),  IN_CREATE | IN_MOVED_TO);
        if (watch_fd_ < 0) {
            std::cerr << "inotify_add_watch failed: " << strerror(errno) << std::endl;
            close(inotify_fd_);
            inotify_fd_ = -1;
        } else {
            std::cout << "Monitoring directory: " << vedio_dir_ << std::endl;
        }
    }

    std::time_t convert_to_time_t(const std::string& date, const std::string& time) {
        int year, month, day, hour, minute, second;
        std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day);
        std::sscanf(time.c_str(), "%d-%d-%d", &hour, &minute, &second);

        std::tm tm = {};
        tm.tm_year = year - 1900; 
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        tm.tm_isdst = -1;
        return std::mktime(&tm);
    }

    std::string get_current_date_string() {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        char buffer[11];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
        return std::string(buffer);
    }

    void openLogFileInAppendMode() {
        std::time_t now = std::time(nullptr);
        std::tm* now_tm = std::localtime(&now);
        
        std::ostringstream oss;
        oss << (now_tm->tm_year + 1900) << "-"
            << std::setw(2) << std::setfill('0') << (now_tm->tm_mon + 1) << "-"
            << std::setw(2) << std::setfill('0') << now_tm->tm_mday << ".txt";

        std::string fileName = oss.str();
        
        std::string fullFilePath = log_file_path_ + "/" + fileName;
        
        log_file_.open(fullFilePath, std::ios_base::app);
        if (log_file_.is_open()) {
            std::cout << "Log file opened: " << fullFilePath << std::endl;
        } else {
            std::cerr << "Failed to open log file: " << fullFilePath << std::endl;
        }
    }

private:
    std::string vedio_dir_;
    int inotify_fd_;
    int watch_fd_;
    //bool unwarning_ = true;
    VedioRecord cur_vedio_;
    VedioRecord last_vedio_;
    std::string current_day_;
    std::ofstream log_file_;
    std::string log_file_path_;
    std::string orgin_path_;
};
#endif
