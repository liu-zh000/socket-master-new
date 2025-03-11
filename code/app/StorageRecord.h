#ifndef STORAGE_RECORD_H
#define STORAGE_RECORD_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#define FILE_NAME_MAX_LEN (20)

#define LINE_HEAD 1
#define LINE_PATH   2
#define LINE_HISTORY_FILE 3
#define LINE_LAST_FILE 4 
enum{
    FREE_TYPE = 0,
    USED_TYPE
};
enum{
    LINES_INIT = -1,
    LINES_WORK_UPDATE_AND_DEL,
    LINES_WORK_DEL,
    LINES_FAIL,
    LINES_OK
};

enum{
    STORAGE_OK = 0,
    STORAGE_NOT_FIND_FILE,
    STORAGE_NOT_FIND_DIR
};
#define RECORD_HEAD ("MP4_RECORD")
#define RECORD_CONFIG_FILE_NAME ("recordconfig.txt")
class StorageRecord{
private:
    std::string m_path;
    uint32_t m_total_MB = 0;
    FILE *m_config_file = NULL;
    int32_t m_line_state = LINES_INIT;
    std::string m_history_name;
    std::string m_last_name;
    std::string m_real_name;
public:
    StorageRecord(const char* path){
        m_path = std::string(path);
        struct statvfs buf;
        if (statvfs(m_path.c_str(), &buf) == 0) {
            unsigned long long total = 0;
            total = buf.f_blocks * buf.f_frsize;
            m_total_MB = (uint32_t)(total / 1024 / 1024);
        } else {
            printf("%d\n", __LINE__);
            perror("statvfs");
        }
        //printf("%d\n", __LINE__);
        
    }
    ~StorageRecord(){
        printf("release StorageRecord\n");
        if(m_config_file){
            fclose(m_config_file);
        }
    }

    void setTimeMap(void){
        time_t currentTime;
        struct tm *localTime;
        currentTime = time(NULL);
        localTime = localtime(&currentTime);
        int hour = localTime->tm_hour;
        int minute = localTime->tm_min;
        int sec = localTime->tm_sec;
        char line[FILE_NAME_MAX_LEN];
        sprintf(line, "%02d-%02d-%02d-00\n", hour, minute, sec);
        FILE * fd = fopen(m_last_name.c_str(), "a+");
        if(fd != NULL){
            fwrite(line, strlen(line), 1, fd);
            fclose(fd);
        }
        else{
            std::cout << "can't record:" << std::string(line) << std::endl;
        }
            
    }

    uint32_t getStoragePercent(){
        uint32_t available = getStorageMB(m_path.c_str(), FREE_TYPE);
        float p = ((float)available)  / ((float)(m_total_MB)) * 100;
        uint32_t r = (uint32_t)p;
        return r;
    }

    uint32_t getStorageMB(const char* path, int32_t type)
    {
        struct statvfs buf;
        uint32_t kb = 0;
        if (statvfs(path, &buf) == 0) {
            unsigned long long total = 0;
            unsigned long long available = 0;
            available = buf.f_frsize * buf.f_bavail;
            if(type == USED_TYPE)
                total = buf.f_blocks * buf.f_frsize - available;
            else
                total = available;
            kb = (uint32_t)(total / 1024 / 1024);
        } else {
            perror("statvfs");
        }
        return kb;
    }
    void initRecordFile(void){

        const char *filename = RECORD_CONFIG_FILE_NAME;

        m_config_file = fopen(filename, "r");
        if (m_config_file == NULL) {
            printf("%d\n", __LINE__);
            exit(-1);
        }

        uint8_t c = 0;
        uint8_t i = 0;
        char lines[512];
        uint8_t line_num = 0;
        printf("%d\n", __LINE__);
   
        while((feof(m_config_file)) == 0){
            c = fgetc(m_config_file);
            lines[i] = c;
            if(c == '\n'){
                line_num++;
                lines[i] = '\0';
                printf("%d:%s\n", __LINE__, lines);
                parseLines(lines, line_num);
                i = 0;
            }
            else{
                i++;
            }
        }
        parse();
    }
private:
    int  listFiles(std::string &path, std::vector<std::string> &allFiles) {
        struct dirent *dp;
        DIR *dir = opendir(path.c_str());

        if (!dir) {
            std::cout << "Unable to open directory:" << path << std::endl;
            return STORAGE_NOT_FIND_DIR;
        }

        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                std::string dpName(dp->d_name);
                std::string local_path = dpName;
                if (dp->d_type != DT_DIR) {
                    allFiles.push_back(local_path);
                    std::cout << __LINE__ << local_path << std::endl;
                }
            }
        }
        closedir(dir);
        return STORAGE_OK;
    }



    void linesWorkDel(void){
        printf("%d\n", __LINE__);
        std::string history_path = m_path + m_history_name;
        linesWorkDel(history_path, m_last_name);
        std::cout << history_path <<":clear vedio dir" << std::endl;
        if(m_history_name != m_last_name){
            std::string last_path = m_path + m_last_name;
            linesWorkDel(last_path, m_last_name);
            std::cout << last_path <<":clear vedio dir" << std::endl;
        }
    }

    int linesWorkDel(std::string &path, std::string &ref){
        std::vector<std::string> allFiles;        
        int rel = listFiles(path, allFiles);
        if(rel == STORAGE_OK){
            rel = findAndDeleteFiles(ref, allFiles);
        }
        return rel;
    }

    int findAndDeleteFiles(std::string &ref, std::vector<std::string> &allFiles){
        std::vector<bool> need_save;
        for(auto x : allFiles){
            need_save.push_back(false);
        }

        FILE* fd = fopen(ref.c_str(), "r");
        if(fd == NULL){
            return STORAGE_NOT_FIND_FILE;
        }

        int row = 0;
        char line[FILE_NAME_MAX_LEN];
        while (fgets(line, FILE_NAME_MAX_LEN, fd) != NULL) {
            std::string word(line);
            int index = getIndex(word, allFiles);
            if(index >= 0)
                need_save[index] = true;

        }
        fclose(fd);
        int len = need_save.size();
        for(int i = 0; i < len; i++){
            if(!need_save[i]){
                const char *f = allFiles[i].c_str();
                if (remove(f) == 0) {
                    printf("sucess rm '%s'.\n", f);
                } else {
                    printf("fail remove:'%s'\n", f);
                }
            }
        }

    }
    int getIndex(std::string &which, std::vector<std::string> &allFiles){
        int l_max = allFiles.size();
        int index = -1;
        for(int i = 0; i < l_max; i++){
            if(which > allFiles[i]){
                index = i - 1;
                break;
            }
        }
        return index;
    }

    void linesFail(void){
        printf("init lines fail\n");
    }

    void lineOK(void){
        printf("vedio dir has been cleared\n"); 
    }

    void parseLines(const char* lines, uint8_t line_num){
        if(line_num == LINE_HEAD){
            std::string head = RECORD_HEAD;
            const char *h = head.c_str();
            int r = strncmp(lines, h, strlen(h));
            if(r != 0){
                m_line_state = LINES_FAIL;
            }
        }
        else if(line_num == LINE_PATH){
            m_path = std::string(lines);
        }
        else if(line_num == LINE_HISTORY_FILE){
            m_history_name = std::string(lines);
        }
        else if(line_num == LINE_LAST_FILE){
            m_last_name = std::string(lines);
            std::string realtime;
            getFileByLocalTime(realtime);
            if(m_last_name < realtime){
                m_line_state = LINES_WORK_UPDATE_AND_DEL;
            }
            else if(m_last_name == realtime){
                m_line_state = LINES_WORK_DEL;
            }
            else{
                m_line_state = LINES_FAIL;
            }

        }
        return;
    }
    void linesWorkUpdate(){
        const char *line[4] = {RECORD_HEAD, m_path.c_str(), m_last_name.c_str(), m_real_name.c_str()};
        char x[1] = {'\n'};
        for(int i = 0; i < 4; i++){
            int len = strlen(line[i]);
            fwrite(line[i], len, 1, m_config_file);
            fwrite(x, 1, 1, m_config_file);
        }
        fflush(m_config_file);
        fclose(m_config_file);
        m_config_file = NULL;
    }

    void parse(void){
        switch(m_line_state){
            case LINES_FAIL:
                linesFail();
            break;
            case LINES_WORK_UPDATE_AND_DEL:
                linesWorkDel();
                linesWorkUpdate();
            break;
            case LINES_WORK_DEL:
                linesWorkDel();
            break;
            case LINES_OK:
                lineOK();
            break;
            default:
            break;

        }
    }
    void findLastDir(std::string &max_dir){
        char path[FILE_NAME_MAX_LEN];
        struct dirent *dp;
        DIR *dir = opendir(m_path.c_str());

        if (!dir) {
            printf("Unable to open directory: %s\n", m_path.c_str());
            return;
        }

        max_dir = "00-00-00-00";
        while ((dp = readdir(dir)) != NULL) {
            if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
                if (dp->d_type == DT_DIR) {
                    std::string d(dp->d_name);
                    if(d > max_dir)
                        max_dir = std::string(dp->d_name);
                    
                }
            }
        }
        std::cout << max_dir << std::endl;
        closedir(dir);        
    }

    void getFileByLocalTime(std::string &m_real_name){
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        int year = tm.tm_year + 1900; 
        int month = tm.tm_mon + 1; 
        int day = tm.tm_mday;
        char fn[FILE_NAME_MAX_LEN];
        sprintf(fn, "%04d-%02d-%02d", year, month, day);
        m_real_name = std::string(fn);
    }
};

#endif