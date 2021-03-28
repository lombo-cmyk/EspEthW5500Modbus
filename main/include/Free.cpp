//
// Created by lukaszk on 28.03.2021.
//

#include "esp_netif.h"
#include "Definitions.h"
#include "esp_log.h"
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