//
// Created by lukaszk on 28.03.2021.
//

#include "EthernetW5500.h"
#include <tuple>

#include "esp_event.h"
#include "esp_log.h"
#include "Free.cpp"

EthernetW5500::EthernetW5500() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    createNetworkInterface();
}

void EthernetW5500::SelectSpiInterface(spi_host_device_t spiInterface) {
    spiInterface_ = spiInterface;
    if (spiInterface_ == SPI2_HOST) {
        spiBusCfg_ = spi_2_BusCfg;
        spiDeviceCfg_ = spi_2_DeviceCfg;
    } else if (spiInterface_ == SPI3_HOST) {
        spiBusCfg_ = spi_3_BusCfg;
        spiDeviceCfg_ = spi_3_DeviceCfg;
    } else {
        ESP_LOGE(EthTag.c_str(),
                 "Wrong SPI interface selected: %d. Supported interfaces: "
                 "SPI2_HOST(%d), SPI3_HOST(%d)",
                 spiInterface,
                 SPI2_HOST,
                 SPI3_HOST);
    }
}

void EthernetW5500::ConfigureAndStart() {
    configureSpiBus();
    configureW5500Driver();
    installSpiEthernet();
    startEthernet();
    waitForIP();
}

void EthernetW5500::createNetworkInterface() {
    esp_netif_init();
    esp_netif_inherent_config_t
        netIfBaseCfg = ESP_NETIF_INHERENT_DEFAULT_ETH(); // todo: replace with
                                                         // some custom
    std::string netIfTag = EthTag + ": " +
                           static_cast<std::string>(netIfBaseCfg.if_desc);
    netIfBaseCfg.if_desc = netIfTag.c_str();
    netIfBaseCfg.route_prio = 64;
    esp_netif_config_t netIfCfg = {.base = &netIfBaseCfg,
                                   .driver = nullptr,
                                   .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH};

    pNetworkInterface_ = esp_netif_new(&netIfCfg);
    assert(pNetworkInterface_);
    registerTcpHandlers();
}

void EthernetW5500::registerTcpHandlers() const {
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(pNetworkInterface_));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_ETH_GOT_IP,
                                               &onGotIpHandler,
                                               nullptr));
}

void EthernetW5500::configureSpiBus() {
    gpio_install_isr_service(0); // probably done in final project?

    ESP_ERROR_CHECK(spi_bus_initialize(spiInterface_, &spiBusCfg_, 1));
    ESP_ERROR_CHECK(
        spi_bus_add_device(spiInterface_, &spiDeviceCfg_, &pSpiHandle_));
}

void EthernetW5500::configureW5500Driver() {
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(pSpiHandle_);
    w5500_config.int_gpio_num = ETH_INT_PIN; // 4 for now

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = -1;
    pMac_ = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    pPhy_ = esp_eth_phy_new_w5500(&phy_config);
}

void EthernetW5500::installSpiEthernet() {
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(pMac_, pPhy_);
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &ethHandle_));
    ESP_ERROR_CHECK(
        esp_eth_ioctl(ethHandle_, ETH_CMD_S_MAC_ADDR, macAddr_.data()));
}

void EthernetW5500::startEthernet() {
    pEthGlue_ = esp_eth_new_netif_glue(ethHandle_);
    esp_netif_attach(pNetworkInterface_, pEthGlue_);
    esp_eth_start(ethHandle_);
}

void EthernetW5500::waitForIP() {
    sem_ip = xSemaphoreCreateCounting(1, 0);
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&ethernetStopHandler));
    ESP_LOGI(EthTag.c_str(), "Waiting for IP(s)");
    ESP_LOGI(EthTag.c_str(), "Sem waiting");
    xSemaphoreTake(sem_ip, portMAX_DELAY);
    ESP_LOGI(EthTag.c_str(), "After sem take");
    // iterate over active interfaces, and print out IPs of "our" netifs
    esp_netif_t* pTempNetInterface = nullptr;
    esp_netif_ip_info_t ip;
    // esp_netif_get_handle_from_ifkey!!!!
    for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
        ESP_LOGI(EthTag.c_str(), "Some shitty ESP netif FOR");
        pTempNetInterface = esp_netif_next(pTempNetInterface);
        if (isOurNetIf(EthTag, pTempNetInterface)) {
            ESP_LOGI(EthTag.c_str(),
                     "Connected to %s",
                     esp_netif_get_desc(pTempNetInterface));
            ESP_ERROR_CHECK(esp_netif_get_ip_info(pTempNetInterface, &ip));

            ESP_LOGI(EthTag.c_str(), "- IPv4 address: " IPSTR, IP2STR(&ip.ip));
        }
    }
//    ESP_LOGI(EthTag.c_str(), "No connection sadge");
}

bool EthernetW5500::isOurNetIf(const std::string& str1,
                               esp_netif_t* pTempNetInterface) {
    std::string str2 = esp_netif_get_desc(pTempNetInterface);
    return str1 == str2.substr(0, str1.length());
}

void EthernetW5500::PrintShit() {
    esp_netif_t* pTempNetInterface = nullptr;
    esp_netif_ip_info_t ip;
    uint8_t mac[6];
    for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
        pTempNetInterface = esp_netif_next(pTempNetInterface);
        if (isOurNetIf(EthTag, pTempNetInterface)) {
            ESP_ERROR_CHECK(esp_netif_get_ip_info(pTempNetInterface, &ip));
            esp_netif_get_mac(pTempNetInterface, mac);
            ESP_LOGW(EthTag.c_str(), "- IPv4 address: " IPSTR, IP2STR(&ip.ip));
            std::array<uint8_t, 6> data{};
            uint32_t phy_read;
//            pMac_->get_addr(pMac_, data.data());
//#define W5500_BSB_COM_REG        (0x00)    // Common Register
//#define W5500_ADDR_OFFSET (16) // Address length
//#define W5500_BSB_OFFSET  (3)  // Block Select Bits offset
//#define W5500_MAKE_MAP(offset, bsb) ((offset) << W5500_ADDR_OFFSET | (bsb) << W5500_BSB_OFFSET)
//#define W5500_REG_PHYCFGR   W5500_MAKE_MAP(0x002E, W5500_BSB_COM_REG) // PHY Configuration
//            pMac_->read_phy_reg(pMac_, 1,W5500_REG_PHYCFGR,&phy_read);
//            ESP_LOGW(EthTag.c_str(), "MAC: %d:%d:%d:%d:%d:%d", data[0],data[1],data[2],data[3],data[4],data[5]);
//            ESP_LOGW(EthTag.c_str(), "- PHY read %d", phy_read);

//            uint32_t phy_add=88;
//            pPhy_->get_addr(pPhy_, &phy_add);
//            ESP_LOGW(EthTag.c_str(), "- PHY %d", phy_add);
            if (ip.ip.addr == 0){
                ESP_LOGW(EthTag.c_str(), "***DANGER NO IP DANGER***");
                std::array<uint8_t, 6> macaddr{};
                uint32_t phy;
                eth_speed_t speed;
                eth_duplex_t duplex;
                esp_eth_ioctl(ethHandle_, ETH_CMD_G_MAC_ADDR, macaddr.data());
                esp_eth_ioctl(ethHandle_, ETH_CMD_G_PHY_ADDR, &phy);
                esp_eth_ioctl(ethHandle_, ETH_CMD_G_DUPLEX_MODE, &duplex);
                esp_eth_ioctl(ethHandle_, ETH_CMD_G_SPEED, &speed);
                ESP_LOGW(EthTag.c_str(), "MAC ADDR: %x:%x:%x:%x:%x:%x", macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
                ESP_LOGW(EthTag.c_str(), "PHY:      %d", phy);
                ESP_LOGW(EthTag.c_str(), "Duplex:   %d", duplex);
                ESP_LOGW(EthTag.c_str(), "Speed:    %d", speed);

                ethernetStopHandler();
                esp_unregister_shutdown_handler(&ethernetStopHandler);
//                esp_netif_deinit();
//                esp_eth_stop(ethHandle_);
//                esp_netif_destroy(pNetworkInterface_);
//                pNetworkInterface_ = nullptr;
                esp_netif_init();
                esp_netif_inherent_config_t
                    netIfBaseCfg = ESP_NETIF_INHERENT_DEFAULT_ETH(); // todo: replace with
                // some custom
                std::string netIfTag = EthTag + ": " +
                                       static_cast<std::string>(netIfBaseCfg.if_desc);
                netIfBaseCfg.if_desc = netIfTag.c_str();
                netIfBaseCfg.route_prio = 64;
                esp_netif_config_t netIfCfg = {.base = &netIfBaseCfg,
                    .driver = nullptr,
                    .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH};

                pNetworkInterface_ = esp_netif_new(&netIfCfg);
                assert(pNetworkInterface_);
                registerTcpHandlers();
//                spi_bus_add_device(spiInterface_, &spiDeviceCfg_, &pSpiHandle_);
                configureW5500Driver();
                installSpiEthernet();
                startEthernet();

                sem_ip = xSemaphoreCreateCounting(1, 0);
                ESP_ERROR_CHECK(esp_register_shutdown_handler(&ethernetStopHandler));
                ESP_LOGI(EthTag.c_str(), "Waiting for IP(s)");
                ESP_LOGI(EthTag.c_str(), "Sem waiting");
                xSemaphoreTake(sem_ip, portMAX_DELAY);
                ESP_LOGI(EthTag.c_str(), "After sem take");
                esp_netif_t* pTempNetInterfaceNEW = nullptr;
                esp_netif_ip_info_t ipNEW;
                for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
                    pTempNetInterfaceNEW = esp_netif_next(pTempNetInterfaceNEW);
                    if (isOurNetIf(EthTag, pTempNetInterfaceNEW)) {
                        ESP_ERROR_CHECK(esp_netif_get_ip_info(pTempNetInterfaceNEW, &ipNEW));

                        ESP_LOGW(EthTag.c_str(), "- IPv4 address: " IPSTR, IP2STR(&ipNEW.ip));
                    }
                }

            }
        }
    }
}
void EthernetW5500::reconfigureDriver() {
    std::array<uint8_t, 6> macaddr{};
    uint32_t phy;
    eth_speed_t speed;
    eth_duplex_t duplex;
    esp_eth_ioctl(ethHandle_, ETH_CMD_G_MAC_ADDR, macaddr.data());
    esp_eth_ioctl(ethHandle_, ETH_CMD_G_PHY_ADDR, &phy);
    esp_eth_ioctl(ethHandle_, ETH_CMD_G_DUPLEX_MODE, &duplex);
    esp_eth_ioctl(ethHandle_, ETH_CMD_G_SPEED, &speed);
    ESP_LOGW(EthTag.c_str(), "Normal work: MAC ADDR: %x:%x:%x:%x:%x:%x", macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
    ESP_LOGW(EthTag.c_str(), "Normal work: PHY:      %d", phy);
    ESP_LOGW(EthTag.c_str(), "Normal work: Duplex:   %d", duplex);
    ESP_LOGW(EthTag.c_str(), "Normal work: Speed:    %d", speed);
//    configureW5500Driver();
//    esp_eth_ioctl(ethHandle_, ETH_CMD_S_MAC_ADDR, macAddr_.data());
//    std::array<uint8_t, 6> mac{};
//    esp_eth_ioctl(ethHandle_, ETH_CMD_G_MAC_ADDR, mac.data());
//    ESP_LOGW(EthTag.c_str(), "SANDFKSJDNFKJSNDFKJSD: %d:%d:%d:%d:%d:%d", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    // todo: esp_eth_ioctl lekiem na cale zlo???
}
