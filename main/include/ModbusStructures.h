//
// Created by lukaszk on 03.04.2021.
//

#ifndef ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
#define ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
#include <cstdio>

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

#endif // ESPETHW5500MODBUS_MODBUSSTRUCTURES_H
