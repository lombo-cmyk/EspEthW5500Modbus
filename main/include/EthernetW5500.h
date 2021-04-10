//
// Created by lukaszk on 28.03.2021.
//

#ifndef ESPETHW5500MODBUS_ETHERNETW5500_H
#define ESPETHW5500MODBUS_ETHERNETW5500_H
#include <string>
#include <array>
#include <memory>
#include "ModbusDefinitions.h"
#include "Singleton.h"

#include "esp_netif.h"
#include "driver/spi_master.h"

class EthernetW5500 final : public Singleton<EthernetW5500> {
public:
    friend Singleton<EthernetW5500>;

    EthernetW5500();
    esp_eth_handle_t ethHandle_{};
    esp_netif_t* pNetworkInterface_ = nullptr;
    void* pEthGlue_ = nullptr;

private:
    void createNetworkInterface();
    void registerTcpHandlers() const;
    void configureSpiBus();
    void configureW5500Driver();
    void installSpiEthernet();
    void startEthernet();

    static void waitForIP();

    static bool isOurNetIf(const std::string& str1, esp_netif_t* pTempNetInterface);
    esp_eth_mac_t* pMac_ = nullptr;
    esp_eth_phy_s* pPhy_ = nullptr;
    spi_device_handle_t pSpiHandle_ = nullptr;
    static constexpr spi_bus_config_t spiBusCfg_ = {
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .sclk_io_num = SPI_CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
        .flags = 0,
        .intr_flags = 0
    };
    static constexpr spi_device_interface_config_t spiDeviceCfg_ = {
        .command_bits = 16, // it's the address phase in W5500 SPI frame
        .address_bits = 8, // it's the control phase in W5500 SPI frame
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = SPI_CLOCK_MHZ * 1000 * 1000,
        .input_delay_ns = 0,
        .spics_io_num = SPI_CS_PIN,
        .flags = 0,
        .queue_size = 20,
        .pre_cb = nullptr,
        .post_cb = nullptr};
    std::array<uint8_t, 6> macAddr_ = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};
};

#endif // ESPETHW5500MODBUS_ETHERNETW5500_H
