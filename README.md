# EspEthW5500Modbus
ESP Modbus TCP module C++ implementation based on example from ESP-IDF repo. 

EthernetW5500 connects automatically to any DHCP server present. 
It's capable of using both ESP32 SPI interfaces, one has to be provided at start.
Bare minimum to start running Ethernet connection:  

```c++
    auto& ethManager = EthernetW5500::getInstance();
    ethManager.SelectSpiInterface(SPI2_HOST); //or SPI3_HOST
    ethManager.ConfigureAndStart();
```

Library can run with SPI3(VSPI) interface on pins:  
> for ESP32 DEVKIT V1 (version with 30 GPIOs).  
* MOSI 23
* MISO 19
* CLK 18
* CS 5
* ETH_INT 4  

...or with SPI2(HSPI) interface on pins:  
> for ESP32 DEVKIT V1 (version with 30 GPIOs).  
* MOSI 13
* MISO 12
* CLK 14
* CS 15
* ETH_INT 4 

> todo: enable using any pin as ETH_INT

Calling `Modbus::getInstance()` initializes Modbus Registers and makes Protocol fully functional.  
Modbus public API enables user to:
* `static void StartSlave()`- responsible for handling Modbus events from Master
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
* component dependency:
    * [Singleton Template](https://github.com/lombo-cmyk/CppSingletonTemplate)
* idf.py menuconfig
    * Enabling W5500 Ethernet: Component config &#8594; Ethernet &#8594; Suppor SPI to Ethernet Module &#8594; Use W5500 (MAC RAW)
    * Enabling output logging: 
        * Component config &#8594; Log output &#8594; Default log verbosity:
            * Info - Shows all component related logs
            * Debug - Shows also SPI Ethernet related logs (from ESP framework)
* main.cpp

```c++
#include <cstdio>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "EthernetW5500.h"
#include "ModbusStructures.h"
#include "Modbus.h"

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
    ethManager.SelectSpiInterface(SPI2_HOST);
    ethManager.ConfigureAndStart();
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