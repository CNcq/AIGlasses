#ifndef AI_GLASSES_DESKTOP_LOG_H
#define AI_GLASSES_DESKTOP_LOG_H

#include <iostream>
#include <string>
#include <cstdarg>
#include <cstdio>

namespace ai_glasses {

// 桌面平台的日志模拟
enum LogLevel {
    LOG_UNKNOWN = 0,
    LOG_DEFAULT = 1,
    LOG_VERBOSE = 2,
    LOG_DEBUG = 3,
    LOG_INFO = 4,
    LOG_WARN = 5,
    LOG_ERROR = 6,
    LOG_FATAL = 7,
};

// 简单的日志输出函数
inline void log_print(LogLevel level, const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    switch (level) {
        case LOG_INFO:
            std::cout << "[INFO] ";
            break;
        case LOG_ERROR:
            std::cerr << "[ERROR] ";
            break;
        case LOG_WARN:
            std::cerr << "[WARN] ";
            break;
        case LOG_DEBUG:
            std::cout << "[DEBUG] ";
            break;
        default:
            std::cout << "[LOG] ";
            break;
    }
    
    std::cout << "(" << tag << ") ";
    vprintf(format, args);
    std::cout << std::endl;
    
    va_end(args);
}

} // namespace ai_glasses

// 宏定义（与 Android log.h 兼容）
#ifndef LOG_TAG
#define LOG_TAG "AIGlasses"
#endif
#define LOGV(...) ai_glasses::log_print(ai_glasses::LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) ai_glasses::log_print(ai_glasses::LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGI(...) ai_glasses::log_print(ai_glasses::LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) ai_glasses::log_print(ai_glasses::LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) ai_glasses::log_print(ai_glasses::LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...) ai_glasses::log_print(ai_glasses::LOG_FATAL, LOG_TAG, __VA_ARGS__)

#endif // AI_GLASSES_DESKTOP_LOG_H
