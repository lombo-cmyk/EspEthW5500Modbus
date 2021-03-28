//
// Created by lukaszk on 28.03.2021.
//

#ifndef ESPETHW5500MODBUS_DEFINITIONS_H
#define ESPETHW5500MODBUS_DEFINITIONS_H
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string>
#define SPI_MOSI_PIN 23
#define SPI_MISO_PIN 19
#define SPI_CLK_PIN 19
#define SPI_CS_PIN 5
#define SPI_CLOCK_MHZ 20
#define ETH_INT_PIN 4
const std::string EthTag = "ETH Connection module";

#endif // ESPETHW5500MODBUS_DEFINITIONS_H
