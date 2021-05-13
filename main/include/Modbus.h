//
// Created by lukaszk on 03.04.2021.
//

#ifndef ESPETHW5500MODBUS_MODBUS_H
#define ESPETHW5500MODBUS_MODBUS_H
#include "esp_netif.h"
#include "mbcontroller.h"
#include "esp_log.h"
#include "ModbusStructures.h"
#include "ModbusDefinitions.h"
#include "Singleton.h"

class Modbus final : public Singleton<Modbus> {
public:
    friend Singleton<Modbus>;

    Modbus();
    void static RunSlaveTask(void* pvParameters);

    void UpdateHoldingRegs(std::uint8_t index, const float& value);
    void UpdateInputRegs(std::uint8_t index, const float& value);
    template<std::size_t B>
    void UpdateCoilRegs(const std::array<std::uint8_t, B>& indexes,
                                const std::bitset<B>& values) {
        if(isInputSane(indexes, coilRegisters_.size())){
            vPortEnterCritical(&modbusMutex);
            for (std::size_t i=0; i<indexes.size(); i++) {
                coilRegisters_.set(indexes[i], values[i]);
            }
            vPortExitCritical(&modbusMutex);
        }
    }
    void UpdateDiscreteRegs(const discreteRegParams_t& reg);

    auto GetHoldingRegs() const -> const holdingRegParams_t& {
        return holdingRegisters_;
    }
    auto GetInputRegs() const -> const inputRegParams_t& {
        return inputRegisters_;
    }
    auto GetCoilRegs() const -> const coilRegParams_t& {
        return coilRegisters_;
    }
    auto GetDiscreteRegs() const -> const discreteRegParams_t& {
        return discreteRegisters_;
    }

private:
    void SetupSlave(esp_netif_t* networkInterface);
    void RegisterDescriptors();
    void SetHoldingReg();
    void SetInputReg();
    void SetCoildReg();
    template<std::size_t B>
    void FillTempBit(
        std::bitset<B>& bitField); // todo: this is only temp filling function
    template<std::size_t B>
    void FillTempArray(
        std::array<float, B>& arr); // todo: this is only temp filling function

    void SetDiscreteReg();
    template<std::size_t B>
    bool isInputSane(const std::array<std::uint8_t, B>& indexes,
                               std::uint8_t regSize) const{
        bool isSane = true;
        if (indexes.size() > regSize) {
            ESP_LOGE(ModbusTag.c_str(), "Too many indexes to unpack!");
            isSane = false;
        }
        for (auto& index : indexes) {
            if (index > (regSize - 1)) {
                ESP_LOGE(ModbusTag.c_str(), "Index number too big!");
                isSane = false;
            }
        }
        return isSane;
    }
    static void StartSlave();
    static void LogDetails(const mb_param_info_t& reg_info);

    holdingRegParams_t holdingRegisters_{};
    inputRegParams_t inputRegisters_{};
    coilRegParams_t coilRegisters_{};
    discreteRegParams_t discreteRegisters_{};
};

#endif // ESPETHW5500MODBUS_MODBUS_H
