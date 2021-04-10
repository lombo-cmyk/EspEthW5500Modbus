# EspEthW5500Modbus
ESP Modbus TCP module C++ implementation based on example from ESP-IDF repo. 

EthernetW5500 connects automatically to any DHCP server present after calling 
`EthernetW5500::getInstance();`.
By default library is configured to run with SPI2 interface on pins:
> for ESP32 DEVKIT V1 (version with 30 GPIOs).  
* MOSI 23
* MISO 19
* CLK 18
* CS 5
* ETH_INT 4  

Calling `Modbus::getInstance()` initializes Modbus Registers and makes Protocol fully functional.  
Modbus public API enables user to:
* Start running slave task - responsible for handling Modbus events from Master
* Update registers:
    * `void UpdateHoldingRegs(const holdingRegParams_t& reg)`
    * `void UpdateInputRegs(const inputRegParams_t& reg)`
    * `void UpdateCoilRegs(const coilRegParams_t& reg);`
    * `void UpdateDiscreteRegs(const discreteRegParams_t& reg);`
* Read register values:
    * `auto GetHoldingRegs() const -> const holdingRegParams_t&`
    * `auto GetInputRegs() const -> const inputRegParams_t&`
    * `auto GetCoilRegs() const -> const coilRegParams_t&`
    * `auto GetDiscreteRegs() const -> const discreteRegParams_t&`

**Usage example**
main.cpp

```c++
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

    for (;;) {
        vTaskDelay(1000);
    }
}
```

Registers can be updating from outside i.e. by injecting code to the loop:
```c++
        vPortEnterCritical(&modbusMutex);
        holdingRegParams_t regHolding = modbusManager.GetHoldingRegs();
        for(auto& val: regHolding){
            val+=1;
        }
        modbusManager.UpdateHoldingRegs(regHolding);
        vPortExitCritical(&modbusMutex);
        vTaskDelay(1000);
```