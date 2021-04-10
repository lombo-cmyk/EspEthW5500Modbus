//
// Created by lukaszk on 03.04.2021.
//

#include "Modbus.h"
#include "Definitions.h"
#include "esp_log.h"
#include "EthernetW5500.h"

#define MB_READ_MASK \
    (MB_EVENT_INPUT_REG_RD | MB_EVENT_HOLDING_REG_RD | MB_EVENT_DISCRETE_RD | \
     MB_EVENT_COILS_RD)
#define MB_WRITE_MASK (MB_EVENT_HOLDING_REG_WR | MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK (MB_READ_MASK | MB_WRITE_MASK)

Modbus::Modbus() {
    void* mbcSlaveHandler = nullptr;
    ESP_ERROR_CHECK(mbc_slave_init_tcp(&mbcSlaveHandler));
    auto& ethManager = EthernetW5500::getInstance();
    SetupSlave(ethManager.pNetworkInterface_);
}
void Modbus::SetupSlave(esp_netif_t* networkInterface) {
    mb_communication_info_t commInfo{};
    commInfo.ip_mode = MB_MODE_TCP;
    commInfo.ip_port = MODBUS_TCP_PORT;
    commInfo.ip_addr_type = MB_IPV4;
    commInfo.ip_addr = nullptr;
    commInfo.ip_netif_ptr = (void*) networkInterface;

    ESP_ERROR_CHECK(mbc_slave_setup((void*) &commInfo));
    RegisterDescriptors();
    StartSlave();
}
void Modbus::RegisterDescriptors() {
    SetHoldingReg();
    SetInputReg();
    SetCoildReg();
    SetDiscreteReg();
    FillTempBit(coilRegisters_);
    FillTempBit(discreteRegisters_);
    FillTempArray(holdingRegisters_);
    FillTempArray(inputRegisters_);
}
void Modbus::SetHoldingReg() {
    mb_register_area_descriptor_t holdingRegArea{
        .start_offset = 0,
        .type = MB_PARAM_HOLDING,
        .address = (void*) &holdingRegisters_,
        .size = sizeof(holdingRegisters_)};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(holdingRegArea));
}
void Modbus::SetInputReg() {
    mb_register_area_descriptor_t inputRegArea{
        .start_offset = 0,
        .type = MB_PARAM_INPUT,
        .address = (void*) &inputRegisters_,
        .size = sizeof(inputRegisters_)};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(inputRegArea));
}
void Modbus::SetCoildReg() {
    mb_register_area_descriptor_t coildRegArea{
        .start_offset = 0,
        .type = MB_PARAM_COIL,
        .address = (void*) &coilRegisters_,
        .size = coilRegisters_.size() / 8};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(coildRegArea));
}
void Modbus::SetDiscreteReg() {
    mb_register_area_descriptor_t discreteRegArea{
        .start_offset = 0,
        .type = MB_PARAM_DISCRETE,
        .address = (void*) &discreteRegisters_,
        .size = discreteRegisters_.size() / 8};
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(discreteRegArea));
}
void Modbus::StartSlave() {
    ESP_ERROR_CHECK(mbc_slave_start());

    ESP_LOGI(ModbusTag.c_str(), "Modbus slave stack initialized.");
}
void Modbus::RunSlaveTask(void* pvParameters) {
    for (;;) {
        ESP_LOGI(ModbusTag.c_str(), "Checking slave events");
        mb_event_group_t event = mbc_slave_check_event(
            static_cast<mb_event_group_t>(MB_READ_WRITE_MASK));
        ESP_LOGI(ModbusTag.c_str(), "Checking slave events - done");

        if (event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event HOLDING REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MODBUS_PARAM_TIMEOUT));
            LogDetails(reg_info);
        } else if (event & MB_EVENT_INPUT_REG_RD) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event INPUT REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MODBUS_PARAM_TIMEOUT));
            LogDetails(reg_info);
        } else if (event & MB_EVENT_DISCRETE_RD) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event DISCRETE REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MODBUS_PARAM_TIMEOUT));
            LogDetails(reg_info);
        } else if (event & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR)) {
            ESP_LOGI(ModbusTag.c_str(), "Slave event COIL REG");
            mb_param_info_t reg_info;
            ESP_ERROR_CHECK(
                mbc_slave_get_param_info(&reg_info, MODBUS_PARAM_TIMEOUT));
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
void Modbus::UpdateHoldingRegs(const holdingRegParams_t& reg) {
    //    vPortEnterCritical(&modbusMutex);
    holdingRegisters_ = reg;
    //    vPortExitCritical(&modbusMutex);
}

void Modbus::UpdateInputRegs(const inputRegParams_t& reg) {
    //    vPortEnterCritical(&modbusMutex);
    inputRegisters_ = reg;
    //    vPortExitCritical(&modbusMutex);
}
void Modbus::UpdateCoilRegs(const coilRegParams_t& reg) {
    //    vPortEnterCritical(&modbusMutex);
    coilRegisters_ = reg;
    //    vPortExitCritical(&modbusMutex);
}
void Modbus::UpdateDiscreteRegs(const discreteRegParams_t& reg) {
    //    vPortEnterCritical(&modbusMutex);
    discreteRegisters_ = reg;
    //    vPortExitCritical(&modbusMutex);
}
