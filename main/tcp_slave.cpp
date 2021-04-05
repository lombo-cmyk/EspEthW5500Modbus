/* FreeModbus Slave Example ESP32

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <cstdio>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "include/EthernetW5500.h"
#include "include/ModbusStructures.h"
#include "include/Modbus.h"

extern "C" {
void app_main();
}

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    auto& ethManager = EthernetW5500::getInstance();
    auto& modbusManager = Modbus::getInstance();

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = nullptr;
    xTaskCreate(Modbus::RunSlaveTask,
                "Modbus_task",
                2048,
                &ucParameterToPass,
                5,
                &xHandle);
    //    Modbus::RunSlaveTask();

    for (;;) {
        modbusManager.UpdateHoldingRegs();
        modbusManager.UpdateInputRegs();
        modbusManager.UpdateCoilRegs();
        modbusManager.UpdateDiscreteRegs();
        vTaskDelay(1000);
    }
}
