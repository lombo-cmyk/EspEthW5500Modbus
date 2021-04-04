//
// Created by lukaszk on 03.04.2021.
//

#include "include/Modbus.h"
#include "include/Definitions.h"
#include "esp_log.h"


#define MB_PAR_INFO_GET_TOUT (10) // Timeout for get parameter info
#define MB_READ_MASK \
    (MB_EVENT_INPUT_REG_RD | MB_EVENT_HOLDING_REG_RD | MB_EVENT_DISCRETE_RD | \
     MB_EVENT_COILS_RD)
#define MB_WRITE_MASK (MB_EVENT_HOLDING_REG_WR | MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK (MB_READ_MASK | MB_WRITE_MASK)

Modbus::Modbus(esp_netif_t* networkInterface) {
    void* mbc_slave_handler = nullptr;
    ESP_ERROR_CHECK(mbc_slave_init_tcp(&mbc_slave_handler));
    SetupSlave(networkInterface);
}
void Modbus::SetupSlave(esp_netif_t* networkInterface) {
    mb_communication_info_t comm_info{};
    comm_info.ip_mode = MB_MODE_TCP;
    comm_info.ip_port = MODBUS_TCP_PORT;
    comm_info.ip_addr_type = MB_IPV4;
    comm_info.ip_addr = nullptr;
    comm_info.ip_netif_ptr = (void*) networkInterface;

    ESP_ERROR_CHECK(mbc_slave_setup((void*) &comm_info));
    RegisterDescriptors();
    StartSlave();
}
void Modbus::RegisterDescriptors() {
    SetHoldingReg();
    SetInputReg();
    SetCoildReg();
    SetDiscreteReg();
    FillTempBit(coil_reg_params);
    FillTempBit(discrete_reg_params);
    FillTempArray(holding_reg_params);
    FillTempArray(input_reg_params);
}
void Modbus::SetHoldingReg() {
    mb_register_area_descriptor_t reg_area{
        .start_offset = 0,
        .type = MB_PARAM_HOLDING,
        .address = (void*) &holding_reg_params,
        .size = sizeof(holding_reg_params)};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
}
void Modbus::SetInputReg() {
    mb_register_area_descriptor_t reg_area{
        .start_offset = 0,
        .type = MB_PARAM_INPUT,
        .address = (void*) &input_reg_params,
        .size = sizeof(input_reg_params)};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
}
void Modbus::SetCoildReg() {
    mb_register_area_descriptor_t reg_area{.start_offset = 0,
                                           .type = MB_PARAM_COIL,
                                           .address = (void*) &coil_reg_params,
                                           .size = coil_reg_params.size() / 8};
    ESP_LOGI(ModbusTag.c_str(), "COILD SIZE: %zu", reg_area.size);
    ESP_LOGI(ModbusTag.c_str(), "COILD SIZE: %u", coil_reg_params.size() / 8);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
}
void Modbus::SetDiscreteReg() {
    mb_register_area_descriptor_t reg_area{
        .start_offset = 0,
        .type = MB_PARAM_DISCRETE,
        .address = (void*) &discrete_reg_params,
        .size = discrete_reg_params.size() / 8};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
}
void Modbus::StartSlave() {
    ESP_ERROR_CHECK(mbc_slave_start());

    ESP_LOGI(ModbusTag.c_str(), "Modbus slave stack initialized.");
    ESP_LOGI(ModbusTag.c_str(), "Start modbus test...");
}
void Modbus::RunSlave(void* pvParameters) {
    for (;;) {
        ESP_LOGI(ModbusTag.c_str(), "Checking slave events");
        mb_event_group_t event = mbc_slave_check_event(
            static_cast<mb_event_group_t>(MB_READ_WRITE_MASK));
        ESP_LOGI(ModbusTag.c_str(), "Checking slave events - done");

        if (event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event HOLDING REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            LogDetails(reg_info);
        } else if (event & MB_EVENT_INPUT_REG_RD) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event INPUT REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            LogDetails(reg_info);
        } else if (event & MB_EVENT_DISCRETE_RD) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event DISCRETE REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            LogDetails(reg_info);
        } else if (event & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR)) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event COIL REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            LogDetails(reg_info);
        }
    }
    ESP_LOGI(ModbusTag.c_str(), "Modbus controller destroyed.");
    vTaskDelay(100);
    ESP_ERROR_CHECK(mbc_slave_destroy());
}
void Modbus::LogDetails(const mb_param_info_t& reg_info) {
    ESP_LOGI(ModbusTag.c_str(),
             "(%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, "
             "SIZE:%u",
             (uint32_t) reg_info.time_stamp,
             (uint32_t) reg_info.mb_offset,
             (uint32_t) reg_info.type,
             (uint32_t) reg_info.address,
             (uint32_t) reg_info.size);
}
template<std::size_t B>
void Modbus::FillTempBit(std::bitset<B>& bitField) {
    for (std::size_t i = 0; i < bitField.size(); i++) {
        bitField.set(i, i % 2);
    }
}
template<std::size_t B>
void Modbus::FillTempArray(std::array<float, B>& arr) {
    for (std::size_t i = 0; i < arr.size(); i++) {
        arr[i] = i;
    }
}
