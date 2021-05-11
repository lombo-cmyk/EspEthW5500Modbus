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
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,
                                                 IP_EVENT_ETH_GOT_IP,
                                                 &onGotIpHandler));

    auto& ethInstance = EthernetW5500::getInstance();
    ESP_ERROR_CHECK(esp_eth_stop(ethInstance.ethHandle_));
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(ethInstance.pEthGlue_));
    ESP_ERROR_CHECK(esp_eth_clear_default_handlers(ethInstance.pNetworkInterface_));
    ESP_ERROR_CHECK(esp_eth_driver_uninstall(ethInstance.ethHandle_));
    esp_netif_destroy(ethInstance.pNetworkInterface_);
}