#pragma once
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <filesystem>
#include <memory>
#include <vector>
#include <thread>
#include "Noncopyable.hpp"

#define FRONTSIZE 1024
#define BACKSIZE 1024 * 1024
enum LogType
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

#define LOG_ERROR(format, ...)                           \
    {                                                    \
        LogType type = ERROR;                            \
        auto logger = Logger::GetInstance();             \
        char buf[FRONTSIZE];                             \
        snprintf(buf, FRONTSIZE, format, ##__VA_ARGS__); \
        logger->Log(type, buf);                          \
    }

#define LOG_INFO(format, ...)                            \
    {                                                    \
        LogType type = INFO;                             \
        auto logger = Logger::GetInstance();             \
        char buf[FRONTSIZE];                             \
        snprintf(buf, FRONTSIZE, format, ##__VA_ARGS__); \
        logger->Log(type, buf);                          \
    }
#define LOG_DEBUG(format, ...)                           \
    {                                                    \
        LogType type = DEBUG;                            \
        auto logger = Logger::GetInstance();             \
        char buf[FRONTSIZE];                             \
        snprintf(buf, FRONTSIZE, format, ##__VA_ARGS__); \
        logger->Log(type, buf);                          \
    }
#define LOG_WARNING(format, ...)                         \
    {                                                    \
        LogType type = WARNING;                          \
        auto logger = Logger::GetInstance();             \
        char buf[FRONTSIZE];                             \
        snprintf(buf, FRONTSIZE, format, ##__VA_ARGS__); \
        logger->Log(type, buf);                          \
    }
class Logger : public Noncopyable
{
private:
    using BufferPtr = std::unique_ptr<std::string>;
    Logger();
    std::mutex mtx_;
    std::condition_variable cv_;
    std::fstream file_;
    BufferPtr current_buf_;
    BufferPtr next_buf_;
    std::vector<BufferPtr> buffers_;
    std::thread thread_;
    LogType type_;
    static constexpr int duration_ = 3;

public:
    static Logger *GetInstance();
    void Log(LogType type, const char *buf);
    void ThreadFunc();
    LogType GetType() const;
    void SetType(LogType);
};