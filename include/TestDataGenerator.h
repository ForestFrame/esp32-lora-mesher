#ifndef _TEST_DATA_GENERATOR_H
#define _TEST_DATA_GENERATOR_H

#include "LoraMesher.h"
#include "BuildOptions.h"

class TestDataGenerator {
private:
    static TestDataGenerator* instance;
    TaskHandle_t TestDataTask_Handle = nullptr;
    bool isRunning = false;
    uint32_t testPacketCounter = 0;

    // 测试数据结构
    struct TestData {
        uint32_t sequenceNumber;
        uint32_t timestamp;
        uint16_t nodeId;
        uint8_t testType;
        uint8_t payload[32]; // 测试负载
    };

public:
    static TestDataGenerator& getInstance();

    /**
     * @brief 初始化测试数据生成器
     */
    void begin();

    /**
     * @brief 启动测试数据生成任务
     */
    void start();

    /**
     * @brief 停止测试数据生成任务
     */
    void stop();

    /**
     * @brief 测试数据生成任务函数
     */
    void testDataGenerationTask();

    /**
     * @brief 生成测试数据包
     */
    void generateTestPacket();

    /**
     * @brief 获取测试统计信息
     */
    uint32_t getGeneratedPacketCount() { return testPacketCounter; }
};

// 测试数据生成间隔（秒）
#define TEST_DATA_INTERVAL 30

#endif