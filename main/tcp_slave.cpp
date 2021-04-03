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

#define SLAVE_TAG "SLAVE_TEST"

static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;

#include "include/EthernetW5500.h"

#pragma pack(push, 1)
typedef struct {
    uint8_t coils_port0 : 1;
    uint8_t coils_port1 : 1;
    uint8_t coils_port2 : 1;
    uint8_t coils_port3 : 1;
    uint8_t coils_port4 : 1;
    uint8_t coils_port5 : 1;
    uint8_t coils_port6 : 1;
    uint8_t coils_port7 : 1;
} coil_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    uint8_t discrete_input0 : 1;
    uint8_t discrete_input1 : 1;
    uint8_t discrete_input2 : 1;
    uint8_t discrete_input3 : 1;
    uint8_t discrete_input4 : 1;
    uint8_t discrete_input5 : 1;
    uint8_t discrete_input6 : 1;
    uint8_t discrete_input7 : 1;
    uint8_t discrete_input_port1 : 8;
} discrete_reg_params_t;

#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    float input_data0; // 0
    float input_data1; // 2
    float input_data2; // 4
    float input_data3; // 6
    uint16_t data[150]; // 8 + 150 = 158
    float input_data4; // 158
    float input_data5;
    float input_data6;
    float input_data7;
} input_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    float holding_data0;
    float holding_data1;
    float holding_data2;
    float holding_data3;
    uint16_t test_regs[150];
    float holding_data4;
    float holding_data5;
    float holding_data6;
    float holding_data7;
} holding_reg_params_t;
#pragma pack(pop)

extern holding_reg_params_t holding_reg_params;
extern input_reg_params_t input_reg_params;
extern coil_reg_params_t coil_reg_params;
extern discrete_reg_params_t discrete_reg_params;


// Set register values into known state
static void setup_reg_data(void) {
    // Define initial state of parameters
    discrete_reg_params.discrete_input0 = 1;
    discrete_reg_params.discrete_input1 = 1;
    discrete_reg_params.discrete_input2 = 1;
    discrete_reg_params.discrete_input3 = 1;
    discrete_reg_params.discrete_input4 = 1;
    discrete_reg_params.discrete_input5 = 1;
    discrete_reg_params.discrete_input6 = 1;
    discrete_reg_params.discrete_input7 = 1;

    holding_reg_params.holding_data0 = 1.34;
    holding_reg_params.holding_data1 = 2.56;
    holding_reg_params.holding_data2 = 3.78;
    holding_reg_params.holding_data3 = 4.90;

    holding_reg_params.holding_data4 = 5.67;
    holding_reg_params.holding_data5 = 6.78;
    holding_reg_params.holding_data6 = 7.79;
    holding_reg_params.holding_data7 = 8.80;
    coil_reg_params.coils_port0 = 0x1;
    coil_reg_params.coils_port1 = 0x0;
    coil_reg_params.coils_port2 = 0x1;
    coil_reg_params.coils_port3 = 0x0;
    coil_reg_params.coils_port4 = 0x1;
    coil_reg_params.coils_port5 = 0x0;
    coil_reg_params.coils_port6 = 0x1;
    coil_reg_params.coils_port7 = 0x0;

    input_reg_params.input_data0 = 1.12;
    input_reg_params.input_data1 = 2.34;
    input_reg_params.input_data2 = 3.56;
    input_reg_params.input_data3 = 4.78;
    input_reg_params.input_data4 = 1.12;
    input_reg_params.input_data5 = 2.34;
    input_reg_params.input_data6 = 3.56;
    input_reg_params.input_data7 = 4.78;
}

//esp_netif_t* get_example_netif(void) {
//    return s_example_esp_netif;
//}

// An example application of Modbus slave. It is based on freemodbus stack.
// See deviceparams.h file for more information about assigned Modbus
// parameters. These parameters can be accessed from main application and also
// can be changed by external Modbus master host.
extern "C" {
void app_main();
}

void app_main(void) {
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

    //    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // Set UART log level
    esp_log_level_set(SLAVE_TAG, ESP_LOG_INFO);
    void* mbc_slave_handler = NULL;

    ESP_ERROR_CHECK(mbc_slave_init_tcp(
        &mbc_slave_handler)); // Initialization of Modbus controller

    mb_param_info_t reg_info; // keeps the Modbus registers access information
    mb_register_area_descriptor_t
        reg_area; // Modbus register area descriptor structure

    mb_communication_info_t comm_info{};
    comm_info.ip_port = MB_TCP_PORT_NUMBER;
    comm_info.ip_addr_type = MB_IPV4;
    comm_info.ip_mode = MB_MODE_TCP;
    comm_info.ip_addr = NULL;
    comm_info.ip_netif_ptr = (void*) EthernetW5500::netif;
    // Setup communication parameters and start stack
    ESP_ERROR_CHECK(mbc_slave_setup((void*) &comm_info));

    // The code below initializes Modbus register area descriptors
    // for Modbus Holding Registers, Input Registers, Coils and Discrete Inputs
    // Initialization should be done for each supported Modbus register area
    // according to register map. When external master trying to access the
    // register in the area that is not initialized by
    // mbc_slave_set_descriptor() API call then Modbus stack will send
    // exception response for this register area.
    reg_area.type = MB_PARAM_HOLDING; // Set type of register area
    reg_area
        .start_offset = MB_REG_HOLDING_START_AREA0; // Offset of register area
                                                    // in Modbus protocol
    reg_area.address = (void*) &holding_reg_params
                           .holding_data0; // Set pointer to storage instance
    reg_area.size = sizeof(float)
                    << 2; // Set the size of register storage instance
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    reg_area.type = MB_PARAM_HOLDING; // Set type of register area
    reg_area
        .start_offset = MB_REG_HOLDING_START_AREA1; // Offset of register area
                                                    // in Modbus protocol
    reg_area.address = (void*) &holding_reg_params
                           .holding_data4; // Set pointer to storage instance
    reg_area.size = sizeof(float)
                    << 2; // Set the size of register storage instance
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Input Registers area
    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = MB_REG_INPUT_START_AREA0;
    reg_area.address = (void*) &input_reg_params.input_data0;
    reg_area.size = sizeof(float) << 2;
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
    // Initialization of Input Registers area
    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = MB_REG_INPUT_START_AREA1;
    reg_area.address = (void*) &input_reg_params.input_data4;
    reg_area.size = sizeof(float) << 2;
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Coils register area
    reg_area.type = MB_PARAM_COIL;
    reg_area.start_offset = MB_REG_COILS_START;
    reg_area.address = (void*) &coil_reg_params;
    reg_area.size = sizeof(coil_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Discrete Inputs register area
    reg_area.type = MB_PARAM_DISCRETE;
    reg_area.start_offset = MB_REG_DISCRETE_INPUT_START;
    reg_area.address = (void*) &discrete_reg_params;
    reg_area.size = sizeof(discrete_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    setup_reg_data(); // Set values into known state

    // Starts of modbus controller and stack
    ESP_ERROR_CHECK(mbc_slave_start());

    ESP_LOGI(SLAVE_TAG, "Modbus slave stack initialized.");
    ESP_LOGI(SLAVE_TAG, "Start modbus test...");
    // The cycle below will be terminated when parameter holding_data0
    // incremented each access cycle reaches the CHAN_DATA_MAX_VAL value.
    for (; holding_reg_params.holding_data0 < MB_CHAN_DATA_MAX_VAL;) {
        // Check for read/write events of Modbus master for certain events
        mb_event_group_t event = mbc_slave_check_event(
            static_cast<mb_event_group_t>(MB_READ_WRITE_MASK));
        const char* rw_str = (event & MB_READ_MASK) ? "READ" : "WRITE";
        // Filter events and process them accordingly
        if (event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {
            // Get parameter information from parameter queue
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG,
                     "HOLDING %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, "
                     "SIZE:%u",
                     rw_str,
                     (uint32_t) reg_info.time_stamp,
                     (uint32_t) reg_info.mb_offset,
                     (uint32_t) reg_info.type,
                     (uint32_t) reg_info.address,
                     (uint32_t) reg_info.size);
            if (reg_info.address ==
                (uint8_t*) &holding_reg_params.holding_data0) {
                portENTER_CRITICAL(&param_lock);
                holding_reg_params.holding_data0 += MB_CHAN_DATA_OFFSET;
                if (holding_reg_params.holding_data0 >=
                    (MB_CHAN_DATA_MAX_VAL - MB_CHAN_DATA_OFFSET)) {
                    //                    coil_reg_params.coils_port1 = 0xFF;
                }
                portEXIT_CRITICAL(&param_lock);
            }
        } else if (event & MB_EVENT_INPUT_REG_RD) {
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG,
                     "INPUT READ (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, "
                     "SIZE:%u",
                     (uint32_t) reg_info.time_stamp,
                     (uint32_t) reg_info.mb_offset,
                     (uint32_t) reg_info.type,
                     (uint32_t) reg_info.address,
                     (uint32_t) reg_info.size);
        } else if (event & MB_EVENT_DISCRETE_RD) {
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG,
                     "DISCRETE READ (%u us): ADDR:%u, TYPE:%u, "
                     "INST_ADDR:0x%.4x, SIZE:%u",
                     (uint32_t) reg_info.time_stamp,
                     (uint32_t) reg_info.mb_offset,
                     (uint32_t) reg_info.type,
                     (uint32_t) reg_info.address,
                     (uint32_t) reg_info.size);
        } else if (event & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR)) {
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG,
                     "COILS %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, "
                     "SIZE:%u",
                     rw_str,
                     (uint32_t) reg_info.time_stamp,
                     (uint32_t) reg_info.mb_offset,
                     (uint32_t) reg_info.type,
                     (uint32_t) reg_info.address,
                     (uint32_t) reg_info.size);
            //            if (coil_reg_params.coils_port1 == 0xFF) break;
        }
    }
    // Destroy of Modbus controller on alarm
    ESP_LOGI(SLAVE_TAG, "Modbus controller destroyed.");
    vTaskDelay(100);
    ESP_ERROR_CHECK(mbc_slave_destroy());
}
