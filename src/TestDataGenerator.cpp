#include "TestDataGenerator.h"  
  
TestDataGenerator* TestDataGenerator::instance = nullptr;  
  
TestDataGenerator& TestDataGenerator::getInstance() {  
    if (instance == nullptr) {  
        instance = new TestDataGenerator();  
    }  
    return *instance;  
}  
  
void TestDataGenerator::begin() {  
    ESP_LOGI(LM_TAG, "Initializing Test Data Generator");  
    testPacketCounter = 0;  
    isRunning = false;  
}  
  
void TestDataGenerator::start() {  
    if (isRunning) {  
        ESP_LOGW(LM_TAG, "Test Data Generator already running");  
        return;  
    }  
      
    ESP_LOGI(LM_TAG, "Starting Test Data Generator Task");  
      
    int res = xTaskCreate(  
        [](void* o) { static_cast<TestDataGenerator*>(o)->testDataGenerationTask(); },  
        "Test Data Generator",  
        4096,  
        this,  
        1, // 低优先级，避免影响网络通信  
        &TestDataTask_Handle);  
          
    if (res != pdPASS) {  
        ESP_LOGE(LM_TAG, "Test Data Generator Task creation failed: %d", res);  
    } else {  
        isRunning = true;  
        ESP_LOGI(LM_TAG, "Test Data Generator Task created successfully");  
    }  
}  
  
void TestDataGenerator::stop() {  
    if (!isRunning) {  
        return;  
    }  
      
    ESP_LOGI(LM_TAG, "Stopping Test Data Generator Task");  
      
    if (TestDataTask_Handle != nullptr) {  
        vTaskDelete(TestDataTask_Handle);  
        TestDataTask_Handle = nullptr;  
    }  
      
    isRunning = false;  
}  
  
void TestDataGenerator::testDataGenerationTask() {  
    ESP_LOGV(LM_TAG, "Test Data Generation Task started");  
      
    // 任务启动时暂停，等待系统完全初始化  
    vTaskSuspend(NULL);  
      
    // 等待初始延时  
    vTaskDelay(5000 / portTICK_PERIOD_MS);  
      
    for (;;) {  
        ESP_LOGV(LM_TAG, "Test Data Task - Stack unused: %d, Free heap: %d",   
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
      
    ESP_LOGI(LM_TAG, "Generating test packet #%d from node %X",   
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
            ESP_LOGI(LM_TAG, "Sending test packet to BROADCAST");  
        } else {  
            // 选择一个随机节点  
            size_t targetIndex = testData.sequenceNumber % numOfNodes;  
            destination = nodes[targetIndex].address;  
            ESP_LOGI(LM_TAG, "Sending test packet to node %X", destination);  
        }  
          
        // 使用LoRaMesher的createPacketAndSend方法发送数据  
        // 这会自动将数据包加入到ToSendPackets队列中  
        loraMesher.createPacketAndSend(destination, &testData, sizeof(TestData));  
          
        // 清理节点数组  
        delete[] nodes;  
          
        ESP_LOGI(LM_TAG, "Test packet #%d queued for transmission", testData.sequenceNumber);  
    } else {  
        ESP_LOGW(LM_TAG, "No nodes in routing table, sending broadcast test packet");  
          
        // 如果没有其他节点，发送广播包  
        loraMesher.createPacketAndSend(BROADCAST_ADDR, &testData, sizeof(TestData));  
    }  
}