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
    static void StartSlave();
    static void LogDetails(const mb_param_info_t& reg_info);

    holding_reg_params_t holding_reg_params{};
    input_reg_params_t input_reg_params{};
    coil_reg_params_t coil_reg_params{};
    discrete_reg_params_t discrete_reg_params{};
};

#endif // ESPETHW5500MODBUS_MODBUS_H
