//
// Created by lukaszk on 03.04.2021.
//

#ifndef ESPETHW5500MODBUS_MODBUS_H
#define ESPETHW5500MODBUS_MODBUS_H
#include "esp_netif.h"
#include "mbcontroller.h"
#include "ModbusStructures.h"
#include "Singleton.h"

class Modbus final : public Singleton<Modbus> {
public:
    friend Singleton<Modbus>;

    Modbus();
    void static RunSlaveTask(void* pvParameters);

    /* As mutex might need to be used before directly updating register values
     * (i.e. 1. Take reg values 2. change one value 3. Update register)
     * blocking functions are commented out and should be taken care by outside
     * application*/
    void UpdateHoldingRegs(std::uint8_t index, const float& value);
    void UpdateInputRegs(std::uint8_t index, const float& value);
    template<std::size_t B>
    void UpdateCoilRegs(std::array<std::uint8_t, B> indexes,
                        std::bitset<B> values);
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
    void SanitizeInput(std::array<std::uint8_t, B> indexes, std::uint8_t regSize);
    static void StartSlave();
    static void LogDetails(const mb_param_info_t& reg_info);

    holdingRegParams_t holdingRegisters_{};
    inputRegParams_t inputRegisters_{};
    coilRegParams_t coilRegisters_{};
    discreteRegParams_t discreteRegisters_{};
};

#endif // ESPETHW5500MODBUS_MODBUS_H
