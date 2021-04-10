//
// Created by lukaszk on 03.04.2021.
//

#ifndef ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
#define ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
#include <cstdio>
#include <array>
#include <bitset>

#pragma pack(push, 1)
typedef std::bitset<8> coilRegParams_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef std::bitset<8> discreteRegParams_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef std::array<float, 8> inputRegParams_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef std::array<float, 8> holdingRegParams_t;
#pragma pack(pop)

#endif // ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
