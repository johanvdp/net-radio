// The author disclaims copyright to this source code.
#include "network.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "mdns.h"

static const char* TAG = "network.c";

static EventGroupHandle_t network_event_group;
static mdns_server_t* network_mdns;

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
	case SYSTEM_EVENT_AP_START:
		ESP_LOGD(TAG, "SYSTEM_EVENT_AP_START")
		xEventGroupSetBits(network_event_group, BIT0);
		break;
	case SYSTEM_EVENT_AP_STOP:
		ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STOP")
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STACONNECTED")
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STADISCONNECTED")
		break;
	case SYSTEM_EVENT_AP_PROBEREQRECVED:
		ESP_LOGD(TAG, "SYSTEM_EVENT_AP_PROBEREQRECVED")
		break;
	case SYSTEM_EVENT_AP_STA_GOT_IP6:
		ESP_LOGD(TAG, "SYSTEM_EVENT_AP_STA_GOT_IP6")
		break;
	default:
		// what else ends up here?
		ESP_LOGD(TAG, "SYSTEM_EVENT %d", event->event_id)
		break;
	}

	return ESP_OK;
}

void ap_begin() {
	ESP_LOGD(TAG, ">ap_begin")
	ESP_LOGD(TAG, "CONFIG_AP_SSID: %s", CONFIG_AP_SSID);
	// only visible in a debug build
	ESP_LOGD(TAG, "CONFIG_AP_KEY: %s", CONFIG_AP_KEY);

	// stop a lot of  built-in wifi logging
	esp_log_level_set("wifi", ESP_LOG_NONE);

	network_event_group = xEventGroupCreate();

	tcpip_adapter_init();

	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));

	tcpip_adapter_ip_info_t info;
	memset(&info, 0, sizeof(info));
	IP4_ADDR(&info.ip, 192, 168, 10, 1);
	IP4_ADDR(&info.gw, 192, 168, 10, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));

	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT()
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

	wifi_config_t ap_config = //
			{ //
			.ap = //
					{ .ssid = CONFIG_AP_SSID, //
							.password = CONFIG_AP_KEY, //
							.ssid_len = 0, //
							.channel = 1, //
							.authmode = WIFI_AUTH_WPA2_PSK, //
							.ssid_hidden = 0, //
							.max_connection = 4, //
							.beacon_interval = 500, //
					}, //
			};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));

	ESP_ERROR_CHECK(esp_wifi_start());

	xEventGroupWaitBits(network_event_group, BIT0, pdFALSE, pdTRUE, portMAX_DELAY);

	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip_info));
	ESP_LOGD(TAG, "ip: %s", ip4addr_ntoa(&ip_info.ip));
	ESP_LOGD(TAG, "netmask: %s", ip4addr_ntoa(&ip_info.netmask));
	ESP_LOGD(TAG, "gateway: %s", ip4addr_ntoa(&ip_info.gw));

	ESP_LOGD(TAG, "<ap_begin")
}

void ap_end() {
	ESP_LOGE(TAG, ">ap_end")
	ESP_ERROR_CHECK(esp_wifi_stop());
	tcpip_adapter_stop(TCPIP_ADAPTER_IF_AP);
	ESP_LOGE(TAG, "<ap_end")
}

void mdns_begin() {
	ESP_LOGD(TAG, ">mdns_begin")
	ESP_LOGD(TAG, "CONFIG_MDNS_HOSTNAME: %s", CONFIG_MDNS_HOSTNAME);

	ESP_ERROR_CHECK(mdns_init(TCPIP_ADAPTER_IF_AP, &network_mdns));
	ESP_ERROR_CHECK(mdns_set_hostname(network_mdns, CONFIG_MDNS_HOSTNAME));
	ESP_ERROR_CHECK(mdns_set_instance(network_mdns, "My network radio"));

	ESP_LOGD(TAG, "<mdns_begin")
}

void mdns_end() {
	ESP_LOGE(TAG, ">mdns_end")
	mdns_free(network_mdns);
	ESP_LOGE(TAG, "<mdns_end")
}
