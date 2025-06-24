#ifndef _LOG_MANAGER_H  
#define _LOG_MANAGER_H  
  
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"  
#include "freertos/queue.h"  
#include "freertos/semphr.h"  
#include "esp_log.h"  
#include "BuildOptions.h"  
  
#define LOG_QUEUE_SIZE 50  
#define LOG_MESSAGE_MAX_SIZE 256  
#define LOG_FILE_NAME_MAX_SIZE 32  
  
// 日志等级控制宏 - 可以在编译时设置  
#ifndef LOG_LEVEL_THRESHOLD  
#define LOG_LEVEL_THRESHOLD LOG_LEVEL_INFO  
#endif  

typedef enum {  
    LOG_LEVEL_ERROR = 0,  
    LOG_LEVEL_WARN = 1,  
    LOG_LEVEL_INFO = 2,  
    LOG_LEVEL_DEBUG = 3,  
    LOG_LEVEL_VERBOSE = 4  
} log_level_t;
  
typedef struct {  
    log_level_t level;  
    char tag[16];  
    char fileName[LOG_FILE_NAME_MAX_SIZE];  
    int lineNumber;  
    char message[LOG_MESSAGE_MAX_SIZE];  
    uint32_t timestamp;  
} log_message_t;  
  
class LogManager {  
private:  
    static LogManager* instance;  
    static QueueHandle_t logQueue;  
    static TaskHandle_t logTaskHandle;  
    static bool initialized;  
      
public:  
    static LogManager& getInstance();  
    void init();  
    void start();  
    void stop();  
    static void logTask(void* parameter);  
    void safeLog(log_level_t level, const char* tag, const char* fileName, int lineNumber, const char* format, ...);  
      
    // 便捷的日志函数  
    static void logError(const char* tag, const char* fileName, int lineNumber, const char* format, ...);  
    static void logWarn(const char* tag, const char* fileName, int lineNumber, const char* format, ...);  
    static void logInfo(const char* tag, const char* fileName, int lineNumber, const char* format, ...);  
    static void logDebug(const char* tag, const char* fileName, int lineNumber, const char* format, ...);  
    static void logVerbose(const char* tag, const char* fileName, int lineNumber, const char* format, ...);  
};  
  
// 提取文件名的宏（去掉路径）  
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)  
  
// 带等级控制的安全日志宏  
#define SAFE_ESP_LOGE(tag, format, ...) \  
    do { \  
        if (LOG_LEVEL_ERROR <= LOG_LEVEL_THRESHOLD) { \  
            LogManager::logError(tag, __FILENAME__, __LINE__, format, ##__VA_ARGS__); \  
        } \  
    } while(0)  
  
#define SAFE_ESP_LOGW(tag, format, ...) \  
    do { \  
        if (LOG_LEVEL_WARN <= LOG_LEVEL_THRESHOLD) { \  
            LogManager::logWarn(tag, __FILENAME__, __LINE__, format, ##__VA_ARGS__); \  
        } \  
    } while(0)  
  
#define SAFE_ESP_LOGI(tag, format, ...) \  
    do { \  
        if (LOG_LEVEL_INFO <= LOG_LEVEL_THRESHOLD) { \  
            LogManager::logInfo(tag, __FILENAME__, __LINE__, format, ##__VA_ARGS__); \  
        } \  
    } while(0)  
  
#define SAFE_ESP_LOGD(tag, format, ...) \  
    do { \  
        if (LOG_LEVEL_DEBUG <= LOG_LEVEL_THRESHOLD) { \  
            LogManager::logDebug(tag, __FILENAME__, __LINE__, format, ##__VA_ARGS__); \  
        } \  
    } while(0)  
  
#define SAFE_ESP_LOGV(tag, format, ...) \  
    do { \  
        if (LOG_LEVEL_VERBOSE <= LOG_LEVEL_THRESHOLD) { \  
            LogManager::logVerbose(tag, __FILENAME__, __LINE__, format, ##__VA_ARGS__); \  
        } \  
    } while(0)  
  
#endif
