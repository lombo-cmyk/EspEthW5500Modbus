//
// Created by lukaszk on 03.04.2021.
//

#ifndef ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
#define ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
#include <cstdio>
#include <array>
#include <bitset>

#pragma pack(push, 1)
typedef std::bitset<8> coil_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef std::bitset<8> discrete_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef std::array<float, 8> input_reg_params_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef std::array<float, 8> holding_reg_params_t;
#pragma pack(pop)

#endif // ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
