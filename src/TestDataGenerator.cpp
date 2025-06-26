#include "TestDataGenerator.h"  
  
TestDataGenerator* TestDataGenerator::instance = nullptr;  
TaskHandle_t TestDataGenerator::TestDataTask_TaskHandle = nullptr;
  
TestDataGenerator& TestDataGenerator::getInstance() {  
    if (instance == nullptr) {  
        instance = new TestDataGenerator();  
    }  
    return *instance;  
}  
  
void TestDataGenerator::begin() {  
    ESP_LOGI("testDataGenerator", "Initializing Test Data Generator");  
    testPacketCounter = 0;  
    isRunning = false;  
}  
  
void TestDataGenerator::start() {  
    if (isRunning) {  
        SAFE_ESP_LOGW("testDataGenerator", "Test Data Generator already running");  
        return;  
    }  
      
    SAFE_ESP_LOGI("testDataGenerator", "Starting Test Data Generator Task");  
      
    int res = xTaskCreate(  
        [](void* o) { static_cast<TestDataGenerator*>(o)->testDataGenerationTask(); },  
        "Test Data Generator",  
        4096,  
        this,  
        1, // 低优先级，避免影响网络通信  
        &TestDataTask_TaskHandle);  
          
    if (res != pdPASS) {  
        SAFE_ESP_LOGE("testDataGenerator", "Test Data Generator Task creation failed: %d", res);  
    } else {  
        isRunning = true;  
        SAFE_ESP_LOGI("testDataGenerator", "Test Data Generator Task created successfully");  
    }  
}  
  
void TestDataGenerator::stop() {  
    if (!isRunning) {  
        return;  
    }  
      
    ESP_LOGI("testDataGenerator", "Stopping Test Data Generator Task");  
      
    if (TestDataTask_TaskHandle != nullptr) {  
        vTaskDelete(TestDataTask_TaskHandle);  
        TestDataTask_TaskHandle = nullptr;  
    }  
      
    isRunning = false;  
}  
  
void TestDataGenerator::testDataGenerationTask() {  
    SAFE_ESP_LOGV("testDataGenerator", "Test Data Generation Task started");  
      
    // 任务启动时暂停，等待系统完全初始化  
    vTaskSuspend(NULL);  
      
    // 等待初始延时  
    vTaskDelay(5000 / portTICK_PERIOD_MS);  
      
    for (;;) {  
        SAFE_ESP_LOGV("testDataGenerator", "Test Data Task - Stack unused: %d, Free heap: %d",   
                 uxTaskGetStackHighWaterMark(NULL), getFreeHeap());  
          
        // 生成测试数据包  
        generateTestPacket();  
          
        // 等待下一次生成  
        vTaskDelay(TEST_DATA_INTERVAL * 1000 / portTICK_PERIOD_MS);  
    }  
}  
  
void TestDataGenerator::generateTestPacket() {  
    LoraMesher& loraMesher = LoraMesher::getInstance();  
      
    // 创建测试数据结构  
    TestData testData;  
    testData.sequenceNumber = testPacketCounter++;  
    testData.timestamp = millis();  
    testData.nodeId = loraMesher.getLocalAddress();  
    testData.testType = 0x01; // 测试数据类型标识  
      
    // 填充测试负载  
    for (int i = 0; i < sizeof(testData.payload); i++) {  
        testData.payload[i] = (uint8_t)(testData.sequenceNumber + i) & 0xFF;  
    }  
      
    SAFE_ESP_LOGI("testDataGenerator", "Generating test packet #%d from node %X",   
             testData.sequenceNumber, testData.nodeId);  

    RoutingTableService::decideHowToSendData(&testData, 1);
}