//
// Created by lukaszk on 03.04.2021.
//

#ifndef ESPETHW5500MODBUS_MODBUS_H
#define ESPETHW5500MODBUS_MODBUS_H
#include "include/ModbusStructures.h"
#include "mbcontroller.h" // for mbcontroller defines and api
#include "ModbusStructures.h"
#include "esp_netif.h"
class Modbus {
public:
    explicit Modbus(esp_netif_t* networkInterface);
    void RunSlave();
private:
    void SetupSlave(esp_netif_t* networkInterface);
    void RegisterDescriptors();
    void SetHoldingReg();
    void SetInputReg();
    void SetCoildReg();

    void SetDiscreteReg();
    static void StartSlave();
    static void LogDetails(const mb_param_info_t& reg_info);

    holding_reg_params_t holding_reg_params{};
    input_reg_params_t input_reg_params{};
    coil_reg_params_t coil_reg_params{};
    discrete_reg_params_t discrete_reg_params{};


};

#endif // ESPETHW5500MODBUS_MODBUS_H
