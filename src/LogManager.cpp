#include "LogManager.h"  
#include <stdarg.h>  
#include <stdio.h>  
#include <string.h>  
  
LogManager* LogManager::instance = nullptr;  
QueueHandle_t LogManager::logQueue = nullptr;  
TaskHandle_t LogManager::logTaskHandle = nullptr;  
bool LogManager::initialized = false;  
  
LogManager& LogManager::getInstance() {  
    if (instance == nullptr) {  
        instance = new LogManager();  
    }  
    return *instance;  
}  
  
void LogManager::init() {  
    if (initialized) {  
        return;  
    }  
      
    logQueue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(log_message_t));  
    if (logQueue == nullptr) {  
        SAFE_ESP_LOGE("LogManager", "Failed to create log queue");  
        return;  
    }  
      
    initialized = true;  
    ESP_LOGI("LogManager", "Log Manager initialized");  
}  
  
void LogManager::start() {  
    if (!initialized) {  
        SAFE_ESP_LOGE("LogManager", "LogManager not initialized");  
        return;  
    }  
      
    if (logTaskHandle != nullptr) {  
        ESP_LOGW("LogManager", "Log task already running");  
        return;  
    }  
      
    int res = xTaskCreate(  
        logTask,  
        "Log Manager Task",  
        4096,  
        nullptr,  
        0, 
        &logTaskHandle);  
          
    if (res != pdPASS) 
    {  
        SAFE_ESP_LOGE("LogManager", "Failed to create log task: %d!", res);  
    } 
    else 
    {  
        SAFE_ESP_LOGI("LogManager", "Log Manager Task started.");  
    }  
}  
  
void LogManager::stop() {  
    if (logTaskHandle != nullptr) {  
        vTaskDelete(logTaskHandle);  
        logTaskHandle = nullptr;  
    }  
}  
  
void LogManager::logTask(void* parameter) {
    log_message_t logMsg;
    const char* levelStrings[] = {"E", "W", "I", "D", "V"};

    ESP_LOGI("LogManager", "Log processing task started");

    for (;;) {
        if (xQueueReceive(logQueue, &logMsg, portMAX_DELAY) == pdTRUE) 
        {
        	uint32_t total_ms = logMsg.timestamp;
			uint32_t minutes = total_ms / 60000;
			uint32_t seconds = (total_ms % 60000) / 1000;
			uint32_t millis  = total_ms % 1000;
            // 格式化输出，带对齐
            printf("[%-1s] [%02u:%02u:%03u] [%-24s:%-4d] [%-24s] %s\n",
		       	levelStrings[logMsg.level],
		       	minutes, seconds, millis,
		       	logMsg.fileName,
		       	logMsg.lineNumber,
		       	logMsg.tag,
		       	logMsg.message);

            fflush(stdout);
        }
    }
}
  
void LogManager::safeLog(log_level_t level, const char* tag, const char* fileName, int lineNumber, const char* format, ...) {  
    if (!initialized || logQueue == nullptr) {  
        // 回退到原有ESP_LOG系统  
        va_list args;  
        va_start(args, format);  
        switch (level) {  
            case LOG_LEVEL_ERROR:  
                esp_log_writev(ESP_LOG_ERROR, tag, format, args);  
                break;  
            case LOG_LEVEL_WARN:  
                esp_log_writev(ESP_LOG_WARN, tag, format, args);  
                break;  
            case LOG_LEVEL_INFO:  
                esp_log_writev(ESP_LOG_INFO, tag, format, args);  
                break;  
            case LOG_LEVEL_DEBUG:  
                esp_log_writev(ESP_LOG_DEBUG, tag, format, args);  
                break;  
            case LOG_LEVEL_VERBOSE:  
                esp_log_writev(ESP_LOG_VERBOSE, tag, format, args);  
                break;  
        }  
        va_end(args);  
        return;  
    }  
      
    log_message_t logMsg;  
    logMsg.level = level;  
    logMsg.timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;  
    logMsg.lineNumber = lineNumber;  
      
    // 复制标签和文件名  
    strncpy(logMsg.tag, tag, sizeof(logMsg.tag) - 1);  
    logMsg.tag[sizeof(logMsg.tag) - 1] = '\0';  
      
    strncpy(logMsg.fileName, fileName, sizeof(logMsg.fileName) - 1);  
    logMsg.fileName[sizeof(logMsg.fileName) - 1] = '\0';  
      
    // 格式化消息  
    va_list args;  
    va_start(args, format);  
    vsnprintf(logMsg.message, sizeof(logMsg.message), format, args);  
    va_end(args);  
      
    // 发送到队列  
    if (xQueueSend(logQueue, &logMsg, 0) != pdTRUE) {  
        printf("LOG_QUEUE_FULL: %s: [%s:%d] %s\n", tag, fileName, lineNumber, logMsg.message);  
    }  
}  
  
// 便捷函数实现  
void LogManager::logError(const char* tag, const char* fileName, int lineNumber, const char* format, ...) {  
    va_list args;  
    va_start(args, format);  
    char buffer[LOG_MESSAGE_MAX_SIZE];  
    vsnprintf(buffer, sizeof(buffer), format, args);  
    va_end(args);  
    getInstance().safeLog(LOG_LEVEL_ERROR, tag, fileName, lineNumber, "%s", buffer);  
}  
  
void LogManager::logWarn(const char* tag, const char* fileName, int lineNumber, const char* format, ...) {  
    va_list args;  
    va_start(args, format);  
    char buffer[LOG_MESSAGE_MAX_SIZE];  
    vsnprintf(buffer, sizeof(buffer), format, args);  
    va_end(args);  
    getInstance().safeLog(LOG_LEVEL_WARN, tag, fileName, lineNumber, "%s", buffer);  
}  
  
void LogManager::logInfo(const char* tag, const char* fileName, int lineNumber, const char* format, ...) {  
    va_list args;  
    va_start(args, format);  
    char buffer[LOG_MESSAGE_MAX_SIZE];  
    vsnprintf(buffer, sizeof(buffer), format, args);  
    va_end(args);  
    getInstance().safeLog(LOG_LEVEL_INFO, tag, fileName, lineNumber, "%s", buffer);  
}  
  
void LogManager::logDebug(const char* tag, const char* fileName, int lineNumber, const char* format, ...) {  
    va_list args;  
    va_start(args, format);  
    char buffer[LOG_MESSAGE_MAX_SIZE];  
    vsnprintf(buffer, sizeof(buffer), format, args);  
    va_end(args);  
    getInstance().safeLog(LOG_LEVEL_DEBUG, tag, fileName, lineNumber, "%s", buffer);  
}  
  
void LogManager::logVerbose(const char* tag, const char* fileName, int lineNumber, const char* format, ...) {  
    va_list args;  
    va_start(args, format);  
    char buffer[LOG_MESSAGE_MAX_SIZE];  
    vsnprintf(buffer, sizeof(buffer), format, args);  
    va_end(args);  
    getInstance().safeLog(LOG_LEVEL_VERBOSE, tag, fileName, lineNumber, "%s", buffer);  
}
