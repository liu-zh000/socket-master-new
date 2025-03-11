#ifndef MYSQL_CLIENT_
#define MYSQL_CLIENT_
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <string>
class MySQLClient {
public:
    MySQLClient(const char* host, const char* user, const char* password, const char* database, unsigned int port = 3306)
        : host_(host), user_(user), password_(password), database_(database), port_(port) {
        mysql_init(&mysql_);
    }

    ~MySQLClient() {
        mysql_close(&mysql_);
    }

    bool connect() {
        if (mysql_real_connect(&mysql_, host_, user_, password_, database_, port_, nullptr, 0) == nullptr) {
            return false;
        }
        return true;
    }

    void disconnect() {
        mysql_close(&mysql_);
    }

    bool executeCommand(const char* command) {
        if (mysql_query(&mysql_, command) != 0) {
            return false;
        }
        return true;
    }

    MYSQL_RES* executeQuery(const char* query) {
        if (mysql_query(&mysql_, query) != 0) {
            return nullptr;
        }

        return mysql_store_result(&mysql_);
    }

    void freeResult(MYSQL_RES* result) {
        mysql_free_result(result);
    }
    bool getColumns(std::string &tablename, std::vector<std::string> &columns) {
        std::string query = "SELECT COLUMN_NAME FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_NAME = '" + tablename + "'";
        columns.clear();
        if (mysql_query(&mysql_, query.c_str()) != 0) {
            std::cerr << "Query execution failed: " << mysql_error(&mysql_) << std::endl;
            mysql_close(&mysql_);
            return false;
        }

        MYSQL_RES *result = mysql_store_result(&mysql_);

        if (result) {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(result))) {
                columns.push_back(row[0]);
                std::cout << "Column Name: " << row[0] << std::endl;
            }
            mysql_free_result(result);
            } else {
                std::cerr << "Failed to store result: " << mysql_error(&mysql_) << std::endl;
            }
    }

private:
    const char* host_;
    const char* user_;
    const char* password_;
    const char* database_;
    unsigned int port_;
    MYSQL mysql_;
};
#endif
