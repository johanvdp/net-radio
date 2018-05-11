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
#include "network_secret.h"

static const char* TAG = "network";

static EventGroupHandle_t network_event_group;
static mdns_server_t* network_mdns;

static void network_log_ip(tcpip_adapter_if_t tcp_if) {
	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(tcp_if, &ip_info));
	ESP_LOGI(TAG, "ip: %s", ip4addr_ntoa(&ip_info.ip));
	ESP_LOGI(TAG, "netmask: %s", ip4addr_ntoa(&ip_info.netmask));
	ESP_LOGI(TAG, "gateway: %s", ip4addr_ntoa(&ip_info.gw));
}

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
	case SYSTEM_EVENT_STA_START:
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		network_log_ip(TCPIP_ADAPTER_IF_STA);
		xEventGroupSetBits(network_event_group, BIT0);
		break;
	case SYSTEM_EVENT_STA_LOST_IP:
		xEventGroupClearBits(network_event_group, BIT0);
		break;
	case SYSTEM_EVENT_AP_START:
		xEventGroupSetBits(network_event_group, BIT0);
		network_log_ip(TCPIP_ADAPTER_IF_AP);
		break;
	case SYSTEM_EVENT_AP_STOP:
		xEventGroupClearBits(network_event_group, BIT0);
		break;
	case SYSTEM_EVENT_ETH_GOT_IP:
		network_log_ip(TCPIP_ADAPTER_IF_ETH);
		break;
	default:
		// see 'event' debug statements
		break;
	}

	return ESP_OK;
}

static bool sta_begin() {
	ESP_LOGD(TAG, ">sta_begin")

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	wifi_config_t wifi_config = //
			{ //
			.sta = //
					{ //
					.ssid = NETWORK_STA_SSID, //
							.password = NETWORK_STA_KEY, //
							.bssid_set = false, //
					}, //
			};

	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	EventBits_t status = xEventGroupWaitBits(network_event_group, BIT0, pdFALSE, pdTRUE,
			(CONFIG_STA_SEARCH_SECONDS * 1000) / portTICK_PERIOD_MS);
	bool found = (status & BIT0);

	ESP_LOGD(TAG, "<sta_begin")
	return found;
}

static void sta_end() {
	ESP_LOGD(TAG, ">sta_end")

	ESP_ERROR_CHECK(esp_wifi_stop());
	tcpip_adapter_stop(TCPIP_ADAPTER_IF_STA);

	ESP_LOGD(TAG, "<sta_end")
}

static void ap_begin() {
	ESP_LOGD(TAG, ">ap_begin")
	ESP_LOGD(TAG, "CONFIG_AP_SSID: %s", CONFIG_AP_SSID);
	// only visible in a debug build
	ESP_LOGD(TAG, "CONFIG_AP_KEY: %s", CONFIG_AP_KEY);

	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));

	tcpip_adapter_ip_info_t info;
	memset(&info, 0, sizeof(info));
	IP4_ADDR(&info.ip, 192, 168, 10, 1);
	IP4_ADDR(&info.gw, 192, 168, 10, 1);
	IP4_ADDR(&info.netmask, 255, 255, 255, 0);
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &info));

	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));

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

	ESP_LOGD(TAG, "<ap_begin")
}

static void ap_end() {
	ESP_LOGE(TAG, ">ap_end")
	ESP_ERROR_CHECK(esp_wifi_stop());
	tcpip_adapter_stop(TCPIP_ADAPTER_IF_AP);
	ESP_LOGE(TAG, "<ap_end")
}

static void mdns_begin(tcpip_adapter_if_t tcpip_if) {
	ESP_LOGD(TAG, ">mdns_begin")
	ESP_LOGD(TAG, "CONFIG_MDNS_HOSTNAME: %s", CONFIG_MDNS_HOSTNAME);

	ESP_ERROR_CHECK(mdns_init(tcpip_if, &network_mdns));
	ESP_ERROR_CHECK(mdns_set_hostname(network_mdns, CONFIG_MDNS_HOSTNAME));
	ESP_ERROR_CHECK(mdns_set_instance(network_mdns, "My network radio"));

	ESP_LOGD(TAG, "<mdns_begin")
}

static void mdns_end() {
	ESP_LOGE(TAG, ">mdns_end")
	mdns_free(network_mdns);
	ESP_LOGE(TAG, "<mdns_end")
}

void network_begin() {
	ESP_LOGD(TAG, ">network_begin")

	network_event_group = xEventGroupCreate();

	tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT()
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	bool connected = sta_begin();
	if (connected) {
		mdns_begin(TCPIP_ADAPTER_IF_STA);
	} else {
		// fallback start maintenance AP
		sta_end();
		ap_begin();
		mdns_begin(TCPIP_ADAPTER_IF_AP);
	}

	ESP_LOGD(TAG, "<network_begin")
}

void network_end() {
	ESP_LOGD(TAG, ">network_end")

	mdns_end();

	wifi_mode_t mode;
	esp_err_t err = esp_wifi_get_mode(&mode);
	if (err == ESP_OK) {
		if (mode == WIFI_MODE_AP) {
			ap_end();
		}
		if (mode == WIFI_MODE_STA) {
			sta_end();
		}
	}

	ESP_ERROR_CHECK(esp_wifi_deinit());
	esp_event_loop_set_cb(NULL, NULL);

	vEventGroupDelete(network_event_group);

	ESP_LOGD(TAG, "<network_end")
}

