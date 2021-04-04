/* FreeModbus Slave Example ESP32

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <cstdio>
#include <cstring>
#include "esp_err.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "mbcontroller.h" // for mbcontroller defines and api

#define MB_TCP_PORT_NUMBER (CONFIG_FMB_TCP_PORT_DEFAULT)
#define MB_MDNS_PORT (502)

// Defines below are used to define register start address for each type of
// Modbus registers
#define HOLD_OFFSET(field) \
    ((uint16_t)(offsetof(holding_reg_params_t, field) >> 1))
#define INPUT_OFFSET(field) \
    ((uint16_t)(offsetof(input_reg_params_t, field) >> 1))
#define MB_REG_DISCRETE_INPUT_START (0x0000)
#define MB_REG_COILS_START (0x0000)
#define MB_REG_INPUT_START_AREA0 \
    0 //(INPUT_OFFSET(input_data0)) // register offset input area 0
#define MB_REG_INPUT_START_AREA1 \
    158 //(INPUT_OFFSET(input_data4)) // register offset input area 1
#define MB_REG_HOLDING_START_AREA0 (HOLD_OFFSET(holding_data0))
#define MB_REG_HOLDING_START_AREA1 (HOLD_OFFSET(holding_data4))

#define MB_PAR_INFO_GET_TOUT (10) // Timeout for get parameter info
#define MB_CHAN_DATA_MAX_VAL (10)
#define MB_CHAN_DATA_OFFSET (0.1f)

#define MB_READ_MASK \
    (MB_EVENT_INPUT_REG_RD | MB_EVENT_HOLDING_REG_RD | MB_EVENT_DISCRETE_RD | \
     MB_EVENT_COILS_RD)
#define MB_WRITE_MASK (MB_EVENT_HOLDING_REG_WR | MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK (MB_READ_MASK | MB_WRITE_MASK)


#include "include/EthernetW5500.h"
#include "include/ModbusStructures.h"
#include "include/Modbus.h"


extern "C" {
void app_main();
}

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    ESP_LOGI(ModbusTag.c_str(), "FREE HEAP: %d", esp_get_free_heap_size());

    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES ||
        result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

//    ESP_ERROR_CHECK(example_connect());
    EthernetW5500 ethMenager = EthernetW5500();
    Modbus modbusManager = Modbus(EthernetW5500::netif);
    //    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_LOGI(ModbusTag.c_str(), "FREE HEAP: %d", esp_get_free_heap_size());
    modbusManager.RunSlave();

}
