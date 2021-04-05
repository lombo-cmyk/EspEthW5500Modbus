//
// Created by lukaszk on 28.03.2021.
//

#ifndef ESPETHW5500MODBUS_ETHERNETW5500_H
#define ESPETHW5500MODBUS_ETHERNETW5500_H
#include <string>
#include <array>
#include <memory>
#include "Definitions.h"
#include "Singleton.h"

#include "esp_netif.h"
#include "driver/spi_master.h"

class EthernetW5500 final : public Singleton<EthernetW5500> {
public:
    friend Singleton<EthernetW5500>;

    EthernetW5500();
    esp_eth_handle_t s_eth_handle{};
    esp_netif_t* netif = nullptr;
    void* s_eth_glue = nullptr;

private:
    void createNetworkInterface();
    void registerTcpHandlers() const;
    void configureSpiBus();
    void configureW5500Driver();
    void installSpiEthernet();
    void startEthernet();

    static void waitForIP();

    static bool isOurNetIf(const std::string& str1, esp_netif_t* netif);
    esp_eth_mac_t* mac = nullptr;
    esp_eth_phy_s* phy = nullptr;
    spi_device_handle_t spi_handle = nullptr;
    static constexpr spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .sclk_io_num = SPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    static constexpr spi_device_interface_config_t devcfg = {
        .command_bits = 16, // it's the address phase in W5500 SPI frame
        .address_bits = 8, // it's the control phase in W5500 SPI frame
        .mode = 0,
        .clock_speed_hz = SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = SPI_CS_PIN,
        .queue_size = 20};
    std::array<uint8_t, 6> mac_arr = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};
};

#endif // ESPETHW5500MODBUS_ETHERNETW5500_H
