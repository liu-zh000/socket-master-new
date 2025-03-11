#include "sqlclientProxy.h"
#include <sstream>

bool sqlclientProxy::getAlarmInfo(RadarAlarmInfo &deviceInfo)//sqlclientProxy从数据库中检索雷达报警信息
{
    std::cout <<"in sqlclientProxy getAlarmInfo()"<<std::endl;//lz
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout <<"lock is ok !"<<__LINE__<<std::endl;//lz 20240406
    // Assume id is known std::string knownId = "1720659778782662657"; // Build the query
    std::ostringstream queryStream; 
    deviceInfo.device_id= get_id();
    // deviceInfo.id = get_id();
    //printf("ID Number = %s\n", deviceInfo.device_id);
    // deviceInfo.id = "1773587526036901890";
    queryStream << "SELECT * FROM radar_alarm_info WHERE device_id = '" << deviceInfo.device_id << "';";
    //  queryStream << "SELECT * FROM radar_alarm_info WHERE serial_number = '" << deviceInfo.serial_number << "';";
    std::string query = queryStream.str(); // Execute the query MYSQL_RES* result = mySQLClient.executeQuery(query.c_str());
    
    MYSQL_RES* result = m_client.executeQuery(query.c_str());
    // Function to fetch a single row from the result set and update the RadarDeviceInfo instance
    bool r = false;
    if (result != nullptr) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row != nullptr){
            deviceInfo.updateFromRow(row);
            r = true;
        } 
        // for(int test_times = 0; test_times < 20; test_times++){
        //     printf("read success!\n");
        // }
    }
    // for(int test_times = 0; test_times < 20; test_times++){
    //         printf("read fail!\n");
    //     }
    m_client.freeResult(result);
    
    return r;
}

bool sqlclientProxy::getMachineInfo(RadarDeviceInfo &deviceInfo)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // Assume id is known std::string knownId = "1720659778782662657"; // Build the query
    std::ostringstream queryStream; 
    // deviceInfo.id = get_id();
    deviceInfo.serial_number = get_serial_number();
    //queryStream << "SELECT * FROM radar_device_info WHERE id = '" << deviceInfo.id << "';";
    queryStream << "SELECT * FROM radar_device_info WHERE serial_number = '" << deviceInfo.serial_number << "';";
    std::string query = queryStream.str(); // Execute the query MYSQL_RES* result = mySQLClient.executeQuery(query.c_str());
    
    MYSQL_RES* result = m_client.executeQuery(query.c_str());
    // Function to fetch a single row from the result set and update the RadarDeviceInfo instance
    bool r = false;
    if (result != nullptr) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row != nullptr){
            deviceInfo.updateFromRow(row);
            r = true;
        }     
    }
    m_client.freeResult(result);
    return r;
}
