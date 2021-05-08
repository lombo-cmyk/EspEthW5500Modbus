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
    void SelectSpiInterface(spi_host_device_t spiInterface);
    void ConfigureAndStart();
    esp_eth_handle_t ethHandle_{};
    esp_netif_t* pNetworkInterface_ = nullptr;
    void* pEthGlue_ = nullptr;
    void PrintShit();
    void reconfigureDriver();

private:
    void createNetworkInterface();
    void registerTcpHandlers() const;
    void configureSpiBus();
    void configureW5500Driver();
    void installSpiEthernet();
    void startEthernet();

    static void waitForIP();

    static bool isOurNetIf(const std::string& str1, esp_netif_t* pTempNetInterface);
    spi_host_device_t spiInterface_ = SPI1_HOST;
    spi_bus_config_t spiBusCfg_ {};
    spi_device_interface_config_t spiDeviceCfg_{};
    esp_eth_mac_t* pMac_ = nullptr;
    esp_eth_phy_s* pPhy_ = nullptr;
    spi_device_handle_t pSpiHandle_ = nullptr;
    std::array<uint8_t, 6> macAddr_ = {0x02, 0x00, 0x00, 0x12, 0x34, 0x56};
};

#endif // ESPETHW5500MODBUS_ETHERNETW5500_H
