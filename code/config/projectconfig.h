#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H
#include "tinyxml2.h"
#include "machineInfo.h"
#include "configProxy.h"
#include <sstream>
#include <vector>
#include <mutex>

 
class ProjectConfig : public configProxy{
public:

    ProjectConfig(const char* project);
    std::string get_id(){
        return m_id;
    }
    bool getAlarmInfo(RadarAlarmInfo &deviceInfo){//ProjectConfig从xml中获取雷达报警信息
        deviceInfo.id                       = getElementTxt("radar_alarm_info", RAI_ID);
        deviceInfo.create_by                = getElementTxt("radar_alarm_info", RAI_CREATE_BY);
        deviceInfo.create_time              = getElementTxt("radar_alarm_info", RAI_CREATE_TIME);
        deviceInfo.update_by                = getElementTxt("radar_alarm_info", RAI_UPDATE_BY);
        deviceInfo.update_time              = getElementTxt("radar_alarm_info", RAI_UPDATE_TIME);
        deviceInfo.sys_org_code             = getElementTxt("radar_alarm_info", RAI_ORG_CODE);
        deviceInfo.device_id                = getElementTxt("radar_alarm_info", RAI_DEVICE_NAME);
        std::string range                   = getElementTxt("radar_alarm_info", RAI_DETECTION_RANGE);
        deviceInfo.detection_range          = std::stod(range);
        std::string threshold               = getElementTxt("radar_alarm_info", RAI_CLUSTERING_THRESHOLD);
        deviceInfo.clustering_threshold     = std::stod(threshold);
        std::string maxcount                = getElementTxt("radar_alarm_info", RAI_MAX_COUNT);//lz
        deviceInfo.max_count                = std::stoi(maxcount);//lz
        std::string mincount                = getElementTxt("radar_alarm_info", RAI_MIN_COUNT);//lz
        deviceInfo.min_count                = std::stoi(mincount);//lz
        std::string totaltime               = getElementTxt("radar_alarm_info", RAI_TOTAL_TIME);//lz
        deviceInfo.total_time               = std::stod(totaltime);//lz
        std::string two_range               = getElementTxt("radar_alarm_info", RAI_TWO_DETECTION_RANGE);//lz
        deviceInfo.two_detection_range               = std::stod(two_range);//lz1104
        // printf("get %lf\n", deviceInfo.detection_range);//lz)
        // printf("get %d\n", deviceInfo.max_count);//lz)
        // printf("get %d\n", deviceInfo.min_count);//lz)
        // printf("get %lf\n", deviceInfo.total_time);//lz)
        // printf("get %lf\n", deviceInfo.clustering_threshold);//lz)
        return true;
    }
    bool getMachineInfo(RadarDeviceInfo &deviceInfo){
        deviceInfo.id                       = getElementTxt("radar_device_info", RDI_ID);
        deviceInfo.create_by                = getElementTxt("radar_device_info", RDI_CREATE_BY);
        deviceInfo.create_time              = getElementTxt("radar_device_info", RDI_CREATE_TIME);
        deviceInfo.update_by                = getElementTxt("radar_device_info", RDI_UPDATE_BY);
        deviceInfo.update_time              = getElementTxt("radar_device_info", RDI_UPDATE_TIME);
        deviceInfo.sys_org_code             = getElementTxt("radar_device_info", RDI_ORG_CODE);
        deviceInfo.name                     = getElementTxt("radar_device_info", RDI_NAME);
        deviceInfo.serial_number            = getElementTxt("radar_device_info", RDI_SERIAL_NUMBER);
        deviceInfo.soft_version_number      = getElementTxt("radar_device_info", RDI_SOFT_VERSION_NUMBER);
        deviceInfo.hard_version_number      = getElementTxt("radar_device_info", RDI_HARD_VERSION_NUMBER);
        deviceInfo.factory_date             = getElementTxt("radar_device_info", RDI_FACTORY_DATE);
        deviceInfo.hand_code                = getElementTxt("radar_device_info", RDI_HAND_CODE);
        deviceInfo.ip_address               = getElementTxt("radar_device_info", RDI_IP_ADDRESS);
        std::string rate                    = getElementTxt("radar_device_info", RDI_RATE);//lz
        deviceInfo.rate                     = std::stoi(rate);//lz
        return true;
    }
    bool setAlarmInfo(const char* key, std::string &info){
        std::lock_guard<std::mutex> lock(m_mutex);
        //m_machineValues[pos] = info;root, const char *name, const char *txt
        const char *txt = info.c_str();
        setElementTxt("radar_alarm_info", key, txt);
        return true;
    }
    bool setAlarm_double_info(const char* key, double info){
        std::ostringstream queryStream;
        queryStream << info;
        std::string query = queryStream.str();
        const char *txt = query.c_str();
        setElementTxt("radar_alarm_info", key, txt);
        m_doc.SaveFile(project_.c_str()) ;
        return true;
    }

    bool setMachineInfo(const char* key, std::string &info){
        std::lock_guard<std::mutex> lock(m_mutex);
        const char *txt = info.c_str();
        setElementTxt("radar_device_info", key, txt);
        return true; 
    }
    ~ProjectConfig(){
            // 保存XML文档到文件
        if (m_doc.SaveFile(project_.c_str()) == tinyxml2::XML_SUCCESS) {
            std::cout << "save file" << std::endl;
        }
    }
private:
    std::vector<std::string> m_alarmColumns;
    std::vector<std::string> m_machineColumns;
    std::string project_;
    //std::vector<std::string> m_alarmValues;
    //std::vector<std::string> m_machineValues;
    std::mutex m_mutex;
    int getElementVal(const char* name);
    const char* getElementTxt(const char* name);
    const char* getElementTxt(const char* root, const char *name);
    void setElementTxt(const char* root, const char *name, const char *txt);
    void insertElementEndChild(const char* root,const char* name, const char* txt);
    void insertElementEndChild(const char* root,const char* name, int val);
    void insertElementSlibing(const char* root,const char* name, const char* txt);
    void insertElementSlibing(const char* root,const char* name, int val);
private:
    int createXML(const char* project);
    tinyxml2::XMLElement* search(tinyxml2::XMLElement* element, const char* name);
    tinyxml2::XMLDocument m_doc;
};

#endif // PROJECTCONFIG_H
