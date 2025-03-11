#ifndef SPDLOG_H
#define SPDLOG_H
#include "spdlog/spdlog.h"
#include"spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include"spdlog/sinks/rotating_file_sink.h"
#include"spdlog/sinks/daily_file_sink.h"
#include"spdlog/sinks/dist_sink.h"
#include "spdlog/fmt/bin_to_hex.h"
#include <memory>
#include <iostream>
#define PRINTLOC() \
       do { \
        std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << ", Function: " << __func__ << std::endl; \
    } while(0)
#define APP_VERSION ("1.1.0")
class LogManager {
public:
    static LogManager& instance() {
        static LogManager instance;
        return instance;
    }

    std::shared_ptr<spdlog::logger> getLogger() {
        return m_logger;
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
    LogManager() {
        // 创建每天生成一个日志文件的文件日志器
        auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("dlogs/log.log", 0, 0);

        // 创建控制台彩色输出的日志器
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        // 创建多重日志器，同时输出到文件和控制台
        m_logger = std::make_shared<spdlog::logger>("m_logger", spdlog::sinks_init_list{file_sink, console_sink});

        // 注册日志器
        //spdlog::register_logger(m_logger);

        // 当遇到 info 消息级别以上的立刻刷新到日志
        m_logger->flush_on(spdlog::level::info);
    }

    
};
#endif