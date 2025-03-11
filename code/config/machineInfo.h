#ifndef MACHINE_INFO_
#define MACHINE_INFO_
#include <iostream>

#include <string>
typedef char** RECORD_INFO;
 
#define RDI_ID                  "id"
#define RDI_CREATE_BY           "create_by"
#define RDI_CREATE_TIME         "create_time"
#define RDI_UPDATE_BY           "update_by"
#define RDI_UPDATE_TIME         "update_time"
#define RDI_ORG_CODE            "sys_org_code"
#define RDI_NAME                "name"
#define RDI_SERIAL_NUMBER       "serial_number"
#define RDI_SOFT_VERSION_NUMBER "soft_version_number"
#define RDI_HARD_VERSION_NUMBER "hard_version_number"
#define RDI_FACTORY_DATE        "factory_date"
#define RDI_HAND_CODE           "hand_code"
#define RDI_IP_ADDRESS          "ip_address"
 #define RDI_RATE                "rate"//lz

#define RAI_ID                  "id" 
#define RAI_CREATE_BY           "create_by" 
#define RAI_CREATE_TIME         "create_time" 
#define RAI_UPDATE_BY           "update_by"   
#define RAI_UPDATE_TIME         "update_time" 
#define RAI_ORG_CODE            "sys_org_code" 
#define RAI_DEVICE_NAME         "device_id" 
#define RAI_DETECTION_RANGE     "detection_range" 
#define RAI_CLUSTERING_THRESHOLD    "clustering_threshold" 
#define RAI_MAX_COUNT           "max_count"   //lz
#define RAI_MIN_COUNT           "min_count"   //lz
#define RAI_TOTAL_TIME    "total_time" //lz   
#define RAI_TWO_DETECTION_RANGE    "two_detection_range" //lz1104

class RadarDeviceInfo {
public:
    std::string id;
    std::string create_by;
    std::string create_time;
    std::string update_by;
    std::string update_time;
    std::string sys_org_code;
    std::string name;
    std::string serial_number;
    std::string soft_version_number;
    std::string hard_version_number;
    std::string factory_date;
    std::string hand_code;
    std::string ip_address;
     int  rate;//lz
    
    // Function to fetch data from MySQL row and update class members
    void updateFromRow(RECORD_INFO row) {
        if (row != nullptr) {
            id = row[0] ? row[0] : "";
            create_by = row[1] ? row[1] : "";
            create_time = row[2] ? row[2] : "";
            update_by = row[3] ? row[3] : "";
            update_time = row[4] ? row[4] : "";
            sys_org_code = row[5] ? row[5] : "";
            name = row[6] ? row[6] : "";
            serial_number = row[7] ? row[7] : "";
            soft_version_number = row[8] ? row[8] : "";
            hard_version_number = row[9] ? row[9] : "";
            factory_date = row[10] ? row[10] : "";
            hand_code = row[11] ? row[11] : "";
            ip_address = row[12] ? row[12] : "";
            rate = std::stoi(row[13]);//lz
            
        }
    }

    // Function to print the content of the class members
    void printInfo() const {
        std::cout << "ID: " << id << std::endl;
        std::cout << "Create By: " << create_by << std::endl;
        std::cout << "Create Time: " << create_time << std::endl;
        std::cout << "Update By: " << update_by << std::endl;
        std::cout << "Update Time: " << update_time << std::endl;
        std::cout << "Sys Org Code: " << sys_org_code << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Serial Number: " << serial_number << std::endl;
        std::cout << "soft Version Number: " << soft_version_number << std::endl;
        std::cout << "hard Version Number: " << hard_version_number << std::endl;
        std::cout << "Factory Date: " << factory_date << std::endl;
        std::cout << "Hand Code: " << hand_code << std::endl;
        std::cout << "IP Address: " << ip_address << std::endl;
        std::cout << "Rate: " << rate << std::endl;//lz
    }
};


class RadarAlarmInfo {
public:
    std::string id;
    std::string serial_number;
    std::string create_by;
    std::string create_time;
    std::string update_by;
    std::string update_time;
    std::string sys_org_code;
    std::string device_id;
    double detection_range;
    double clustering_threshold;
    int max_count;//lz
    int min_count;//lz
    double total_time;//lz
    double two_detection_range;//lz1104

    // Function to fetch data from MySQL row and update class members
    void updateFromRow(RECORD_INFO row) {
        if (row != nullptr) {
            id = row[0] ? row[0] : "";
            create_by = row[1] ? row[1] : "";
            create_time = row[2] ? row[2] : "";
            update_by = row[3] ? row[3] : "";
            update_time = row[4] ? row[4] : "";
            sys_org_code = row[5] ? row[5] : "";
            device_id = row[6] ? row[6] : "";
            detection_range = std::stod(row[7]);
            clustering_threshold = std::stod(row[8]);
            max_count = std::stoi(row[9]);//lz
            min_count = std::stoi(row[10]);//lz
            total_time = std::stod(row[11]);//lz
            two_detection_range = std::stod(row[12]);//lz1104
        }
    }
    void printInfo() {
        std::cout << "ID: " << id << std::endl;
        std::cout << "Device ID: " << device_id << std::endl;
        std::cout << "Detection Range: " << detection_range << std::endl;
        std::cout << "Clustering Threshold: " << clustering_threshold << std::endl;
        std::cout << "Max Count: " << max_count << std::endl;//lz
        std::cout << "Min Count: " << min_count << std::endl;//lz
        std::cout << "Total Time: " << total_time << std::endl;//lz
        std::cout << "Two Detection Range: " << two_detection_range << std::endl;//lz1104
        // Print other fields as needed
    }
};


#endif