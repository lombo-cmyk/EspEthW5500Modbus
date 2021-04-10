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
        vPortEnterCritical(&modbusMutex);
        holdingRegParams_t regHolding = modbusManager.GetHoldingRegs();
        for(auto& val: regHolding){
            val+=1;
        }
        modbusManager.UpdateHoldingRegs(regHolding);
        vPortExitCritical(&modbusMutex);
        vTaskDelay(1000);

        vPortEnterCritical(&modbusMutex);
        inputRegParams_t regInput=modbusManager.GetInputRegs();
        for(auto& val: regInput){
            val+=1;
        }
        modbusManager.UpdateInputRegs(regInput);
        vPortExitCritical(&modbusMutex);
        vTaskDelay(1000);

        vPortEnterCritical(&modbusMutex);
        coilRegParams_t regCoil=modbusManager.GetCoilRegs();
        for (std::size_t i = 0; i < regCoil.size(); i++) {
            regCoil.set(i, !regCoil[i]);
        }
        modbusManager.UpdateCoilRegs(regCoil);
        vPortExitCritical(&modbusMutex);
        vTaskDelay(1000);

        vPortEnterCritical(&modbusMutex);
        discreteRegParams_t regDiscrete=modbusManager.GetDiscreteRegs();
        for (std::size_t i = 0; i < regDiscrete.size(); i++) {
            regDiscrete.set(i, !regDiscrete[i]);
        }
        modbusManager.UpdateDiscreteRegs(regDiscrete);
        vPortExitCritical(&modbusMutex);
        vTaskDelay(1000);
    }
}


