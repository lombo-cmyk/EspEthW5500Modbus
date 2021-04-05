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

    netif = esp_netif_new(&netIfCfg);
    assert(netif);
    registerTcpHandlers();
}
void EthernetW5500::registerTcpHandlers() const {
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(netif));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,
                                               IP_EVENT_ETH_GOT_IP,
                                               &onGotIpHandler,
                                               nullptr));
}
void EthernetW5500::configureSpiBus() {
    gpio_install_isr_service(0); // probably done in final project?

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_handle));
}
void EthernetW5500::configureW5500Driver() {
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);
    w5500_config.int_gpio_num = ETH_INT_PIN; // 4 for now

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 1;
    phy_config.reset_gpio_num = -1;
    mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    phy = esp_eth_phy_new_w5500(&phy_config);
}
void EthernetW5500::installSpiEthernet() {
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &s_eth_handle));
    ESP_ERROR_CHECK(
        esp_eth_ioctl(s_eth_handle, ETH_CMD_S_MAC_ADDR, mac_arr.data()));
}
void EthernetW5500::startEthernet() {
    s_eth_glue = esp_eth_new_netif_glue(s_eth_handle);
    esp_netif_attach(netif, s_eth_glue);
    esp_eth_start(s_eth_handle);
}

void EthernetW5500::waitForIP() {
    sem_ip = xSemaphoreCreateCounting(1, 0);
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&ethernetStopHandler));
    ESP_LOGI(EthTag.c_str(), "Waiting for IP(s)");
    ESP_LOGI(EthTag.c_str(), "Sem waiting");
    xSemaphoreTake(sem_ip, portMAX_DELAY);
    ESP_LOGI(EthTag.c_str(), "After sem take");
    // iterate over active interfaces, and print out IPs of "our" netifs
    esp_netif_t* nnetif = nullptr;
    esp_netif_ip_info_t ip;
    // esp_netif_get_handle_from_ifkey!!!!
    for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
        ESP_LOGI(EthTag.c_str(), "Some shitty ESP netif FOR");
        nnetif = esp_netif_next(nnetif);
        if (isOurNetIf(EthTag, nnetif)) {
            ESP_LOGI(EthTag.c_str(),
                     "Connected to %s",
                     esp_netif_get_desc(nnetif));
            ESP_ERROR_CHECK(esp_netif_get_ip_info(nnetif, &ip));

            ESP_LOGI(EthTag.c_str(), "- IPv4 address: " IPSTR, IP2STR(&ip.ip));
        }
    }
    ESP_LOGI(EthTag.c_str(), "No connection sadge");
}

bool EthernetW5500::isOurNetIf(const std::string& str1, esp_netif_t* nnnetif) {
    std::string str2 = esp_netif_get_desc(nnnetif);
    return str1 == str2.substr(0, str1.length());
}