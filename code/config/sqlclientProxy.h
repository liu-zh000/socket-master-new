#ifndef SQLCLIENT_PROXY
#define SQLCLIENT_PROXY
#include "sqlclient.h"
#include "machineInfo.h"
#include "configProxy.h"
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>
class sqlclientProxy : public configProxy{
public:
    sqlclientProxy(MySQLClient &client):m_client(client){
        std::string alarmTable = "radar_alarm_info";
        m_client.getColumns(alarmTable, m_alarmColumns);
        std::string machineTable = "radar_device_info";
        m_client.getColumns(machineTable, m_machineColumns);
    }
    bool getMachineInfo(RadarDeviceInfo &deviceInfo);
    bool getAlarmInfo(RadarAlarmInfo &deviceInfo);
    bool setAlarmInfo(const char* pos, std::string &info){
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ostringstream queryStream;
       // queryStream << "UPDATE radar_alarm_info SET " << pos << " = " << info << " WHERE id = '" << m_id << "';";
        //queryStream << "UPDATE radar_alarm_info SET " << pos << " = " << info << " WHERE serial_number = '" << m_serial_number << "';";
        queryStream << "UPDATE radar_alarm_info SET " << pos << " = " << info << " WHERE device_id = '" << m_id << "';";//lz 20240406
        std::string query = queryStream.str(); // Execute the query mySQLClient.executeUpdate(query.c_str());
        m_client.executeQuery(query.c_str());
        return true;
    }
    bool setAlarm_double_info(const char* pos, double info){
        std::ostringstream queryStream;
        queryStream << info;
        std::string query = queryStream.str();
        return setAlarmInfo(pos, query);
    }

    bool setMachineInfo(const char* pos, std::string &info){
        std::lock_guard<std::mutex> lock(m_mutex);
        std::ostringstream queryStream;
        //queryStream << "UPDATE radar_device_info SET " << pos << " = " << info << " WHERE id = '" << m_id << "';";
        queryStream << "UPDATE radar_device_info SET " << pos << " = " << info << " WHERE serial_number = '" << m_serial_number << "';";
        std::string query = queryStream.str(); // Execute the query mySQLClient.executeUpdate(query.c_str());
        m_client.executeQuery(query.c_str());
        return true; 
    }
    
private:
    std::vector<std::string> m_alarmColumns;
    std::vector<std::string> m_machineColumns;
    MySQLClient &m_client;
    std::mutex m_mutex;
};
#endif