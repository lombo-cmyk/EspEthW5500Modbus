//
// Created by lukaszk on 28.03.2021.
//

#include "esp_netif.h"
#include "esp_log.h"
#include "esp_event.h"
#include "EthernetW5500.h"

static xSemaphoreHandle sem_ip;

static void onGotIpHandler(void* arg,
                           esp_event_base_t event_base,
                           int32_t event_id,
                           void* event_data) {
    auto event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(EthTag.c_str(),
             "Got IPv4 event: Interface \"%s\" address: " IPSTR,
             esp_netif_get_desc(event->esp_netif),
             IP2STR(&event->ip_info.ip));
    xSemaphoreGive(sem_ip);
    ESP_LOGI(EthTag.c_str(), "Releasing sem");
}

static void ethernetStopHandler() {
    esp_netif_t* tmp_netif = nullptr;
    std::string netif_description = EthTag + ": eth";
    while ((tmp_netif = esp_netif_next(tmp_netif)) != nullptr) {
        std::string str2 = esp_netif_get_desc(tmp_netif);
        if (netif_description == str2.substr(0, netif_description.length())) {
            break;
        }
    }
    esp_netif_t* eth_netif = tmp_netif;
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,
                                                 IP_EVENT_ETH_GOT_IP,
                                                 &onGotIpHandler));

    auto& ethInstance = EthernetW5500::getInstance();
    ESP_ERROR_CHECK(esp_eth_stop(ethInstance.s_eth_handle));
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(ethInstance.s_eth_glue));
    ESP_ERROR_CHECK(esp_eth_clear_default_handlers(eth_netif));
    ESP_ERROR_CHECK(esp_eth_driver_uninstall(ethInstance.s_eth_handle));

    esp_netif_destroy(eth_netif);
    ethInstance.netif = nullptr;
}