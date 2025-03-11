#ifndef CREATE_PROXY_H
#define CREATE_PROXY_H
#include "sqlclient.h"
#include "sqlclientProxy.h"
#include "projectconfig.h"
#include "configProxy.h"
#include "../log/applog.h"
class createProxy{
public:
    configProxy* create(void){
        //需要修改的数据库信�?,登录�?,密码,库名
        std::string user = "root";
        std::string passwd = "root";
        std::string databasename = "radar";
        auto logger = (LogManager::instance()).getLogger();
        m_projectProxy = new ProjectConfig("radar.xml");
        std::string id = m_projectProxy->get_id();  //qq
        std::string serial_number = m_projectProxy->get_serial_number();
        //m_client = new MySQLClient("localhost", user.c_str(), passwd.c_str(), databasename.c_str());
        m_client = new MySQLClient("192.168.0.5", user.c_str(), passwd.c_str(), databasename.c_str());
        //std::cout<<"wait to connect //lz"<<std::endl;
        //if(m_client->connect()){
        if(false){
            m_sqlProxy = new sqlclientProxy(*m_client);
            m_serverInfo = m_sqlProxy;
            logger->info("connect database ok");
        }
        else{
            logger->info("connect database failed, read config file");
            m_serverInfo = m_projectProxy;
        }
        //m_serverInfo->set_id(id);   //qq
        m_serverInfo->set_serial_number(serial_number);
        return m_serverInfo;
    }
    ~createProxy(){
        if(m_serverInfo)
            delete m_serverInfo;
        if(m_projectProxy)
            delete m_projectProxy;
        if(m_sqlProxy)
            delete m_sqlProxy;
        if(m_client)
            delete m_client;
    }
private:
    configProxy *m_serverInfo = nullptr;
    ProjectConfig *m_projectProxy = nullptr;
    sqlclientProxy *m_sqlProxy = nullptr;
    MySQLClient *m_client = nullptr;
};
#endif