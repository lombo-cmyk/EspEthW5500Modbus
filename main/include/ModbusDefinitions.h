//
// Created by lukaszk on 28.03.2021.
//

#ifndef ESPETHW5500MODBUS_DEFINITIONS_H
#define ESPETHW5500MODBUS_DEFINITIONS_H
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string>
#define VSPI_MOSI_PIN 23 //VSPI corresponds to SPI3_HOST
#define VSPI_MISO_PIN 19
#define VSPI_CLK_PIN 18
#define VSPI_CS_PIN 5

#define HSPI_MOSI_PIN 13 //HSPI corresponds to SPI2_HOST
#define HSPI_MISO_PIN 12
#define HSPI_CLK_PIN 14
#define HSPI_CS_PIN 15

#define SPI_CLOCK_MHZ 20
#define ETH_INT_PIN 4

#define MODBUS_TCP_PORT 502
#define MODBUS_PARAM_TIMEOUT 10
#define SECOND 100
const std::string EthTag = "ETH Connection module";
const std::string ModbusTag = "Modbus Slave module";
static portMUX_TYPE modbusMutex = portMUX_INITIALIZER_UNLOCKED;

static constexpr spi_bus_config_t spi_3_BusCfg = {
        .mosi_io_num = VSPI_MOSI_PIN,
        .miso_io_num = VSPI_MISO_PIN,
        .sclk_io_num = VSPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0,
        .intr_flags = 0
};
static constexpr spi_device_interface_config_t spi_3_DeviceCfg = {
        .command_bits = 16, // it's the address phase in W5500 SPI frame
        .address_bits = 8, // it's the control phase in W5500 SPI frame
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = SPI_CLOCK_MHZ * 1000 * 1000,
        .input_delay_ns = 0,
        .spics_io_num = VSPI_CS_PIN,
        .flags = 0,
        .queue_size = 20,
        .pre_cb = nullptr,
        .post_cb = nullptr};

static constexpr spi_bus_config_t spi_2_BusCfg = {
        .mosi_io_num = HSPI_MOSI_PIN,
        .miso_io_num = HSPI_MISO_PIN,
        .sclk_io_num = HSPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0,
        .intr_flags = 0
};
static constexpr spi_device_interface_config_t spi_2_DeviceCfg = {
        .command_bits = 16, // it's the address phase in W5500 SPI frame
        .address_bits = 8, // it's the control phase in W5500 SPI frame
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = SPI_CLOCK_MHZ * 1000 * 1000,
        .input_delay_ns = 0,
        .spics_io_num = HSPI_CS_PIN,
        .flags = 0,
        .queue_size = 20,
        .pre_cb = nullptr,
        .post_cb = nullptr};

#endif // ESPETHW5500MODBUS_DEFINITIONS_H
