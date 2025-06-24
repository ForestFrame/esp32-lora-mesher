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
      
    // 获取路由表中的节点数量  
    size_t numOfNodes = loraMesher.routingTableSize();  
      
    if (numOfNodes > 0) {  
        // 获取所有网络节点  
        NetworkNode* nodes = RoutingTableService::getAllNetworkNodes();  
          
        // 随机选择一个目标节点（或广播）  
        uint16_t destination;  
        if (testData.sequenceNumber % 3 == 0) {  
            // 每三个包发送一次广播  
            destination = BROADCAST_ADDR;  
            SAFE_ESP_LOGI("testDataGenerator", "Sending test packet to BROADCAST");  
        } else {  
            // 选择一个随机节点  
            size_t targetIndex = testData.sequenceNumber % numOfNodes;  
            destination = nodes[targetIndex].address;  
            SAFE_ESP_LOGI("testDataGenerator", "Sending test packet to node %X", destination);  
        }  
          
        // 使用LoRaMesher的createPacketAndSend方法发送数据  
        // 这会自动将数据包加入到ToSendPackets队列中  
        loraMesher.createPacketAndSend(destination, &testData, 1);  
          
        // 清理节点数组  
        delete[] nodes;  
          
        SAFE_ESP_LOGI("testDataGenerator", "Test packet #%d queued for transmission", testData.sequenceNumber);  
    } else {  
        SAFE_ESP_LOGW("testDataGenerator", "No nodes in routing table, sending broadcast test packet");  
          
        // 如果没有其他节点，发送广播包  
        loraMesher.createPacketAndSend(BROADCAST_ADDR, &testData, 1); 
        SAFE_ESP_LOGV("testDataGenerator", "Test data size %d", sizeof(TestData));
    }  
}