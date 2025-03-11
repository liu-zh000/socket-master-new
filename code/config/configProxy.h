#ifndef CONFIG_PROXY_H
#define CONFIG_PROXY_H
#include "machineInfo.h"

#include <string>
class configProxy{
public:
    bool virtual getAlarmInfo(RadarAlarmInfo &deviceInfo) = 0;
    bool virtual getMachineInfo(RadarDeviceInfo &deviceInfo) = 0;
    bool virtual setAlarmInfo(const char* pos, std::string &info) = 0;
    bool virtual setAlarm_double_info(const char* pos, double info) = 0;
    bool virtual setMachineInfo(const char* pos, std::string &info) = 0;
   std::string &get_id(){
       return m_id;
    }
    void set_id(std::string &id){
        m_id = id;
    }
      std::string &get_serial_number(){
        return m_serial_number;
    }
    void set_serial_number(std::string &serial_number){
        m_serial_number = serial_number;
    }
protected:
    std::string m_id = "empty";
    std::string m_serial_number = "empty";
};


#endif