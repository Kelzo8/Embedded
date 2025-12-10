/*
 * ESP32 MQTT Client for SmartSafe (ESP-IDF C)
 * 
 * This example shows how to:
 * - Connect to WiFi
 * - Connect to MQTT broker
 * - Subscribe to command topic: smartsafe/safe/command
 * - Publish telemetry to: smartsafe/safe/telemetry
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_mqtt_client.h"
#include "cJSON.h"

static const char *TAG = "MQTT_CLIENT";

// Configuration - edit config.h file
#include "config.h"

// MQTT topics (built from device ID)
#define TELEMETRY_TOPIC_FMT "smartsafe/%s/telemetry"
#define COMMAND_TOPIC_FMT   "smartsafe/%s/command"

static char telemetry_topic[64];
static char command_topic[64];

static esp_mqtt_client_handle_t mqtt_client = NULL;
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying WiFi connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "WiFi connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialization finished. Connecting to %s...", WIFI_SSID);
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            // Subscribe to command topic
            msg_id = esp_mqtt_client_subscribe(client, command_topic, 1);
            ESP_LOGI(TAG, "Subscribed to %s, msg_id=%d", command_topic, msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT subscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT unsubscribed, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT published, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT data received:");
            ESP_LOGI(TAG, "  Topic: %.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "  Data: %.*s", event->data_len, event->data);

            // Parse JSON command
            if (strncmp(event->topic, command_topic, event->topic_len) == 0) {
                char *json_str = malloc(event->data_len + 1);
                memcpy(json_str, event->data, event->data_len);
                json_str[event->data_len] = '\0';

                cJSON *json = cJSON_Parse(json_str);
                if (json != NULL) {
                    cJSON *command = cJSON_GetObjectItem(json, "command");
                    if (cJSON_IsString(command)) {
                        const char *cmd = command->valuestring;
                        
                        if (strcmp(cmd, "unlock") == 0) {
                            ESP_LOGI(TAG, "Command: UNLOCK");
                            // TODO: Implement unlock logic
                            
                        } else if (strcmp(cmd, "lock") == 0) {
                            ESP_LOGI(TAG, "Command: LOCK");
                            // TODO: Implement lock logic
                            
                        } else if (strcmp(cmd, "set_code") == 0) {
                            cJSON *code = cJSON_GetObjectItem(json, "code");
                            if (cJSON_IsString(code)) {
                                ESP_LOGI(TAG, "Command: SET_CODE to %s", code->valuestring);
                                // TODO: Implement set_code logic
                            }
                        }
                    }
                    cJSON_Delete(json);
                } else {
                    ESP_LOGE(TAG, "Failed to parse JSON");
                }
                free(json_str);
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT error");
            break;

        default:
            ESP_LOGI(TAG, "Other MQTT event id:%d", event->event_id);
            break;
    }
}

static void mqtt_app_start(void)
{
    char client_id[32];
    snprintf(client_id, sizeof(client_id), "ESP32-SmartSafe-%04X", esp_random() & 0xFFFF);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.hostname = MQTT_BROKER_URL,
        .broker.address.port = MQTT_BROKER_PORT,
        .credentials.client_id = client_id,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

static void publish_telemetry(void)
{
    if (mqtt_client == NULL) {
        return;
    }

    // Create JSON payload
    cJSON *json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "timestamp", esp_timer_get_time() / 1000000); // seconds
    cJSON_AddBoolToObject(json, "motion", false);  // TODO: Read from motion sensor
    cJSON_AddBoolToObject(json, "alarm", false);   // TODO: Read alarm state
    cJSON_AddStringToObject(json, "state", "locked"); // TODO: Read actual state
    cJSON_AddBoolToObject(json, "code_ok", false);  // TODO: Read code status

    char *json_string = cJSON_Print(json);
    if (json_string != NULL) {
        esp_mqtt_client_publish(mqtt_client, telemetry_topic, json_string, 0, 1, 0);
        ESP_LOGI(TAG, "Published telemetry: %s", json_string);
        free(json_string);
    }
    cJSON_Delete(json);
}

static void telemetry_task(void *pvParameters)
{
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // Wait 10 seconds
        publish_telemetry();
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "SmartSafe MQTT Client Starting...");

    // Build topic strings from device ID
    snprintf(telemetry_topic, sizeof(telemetry_topic), TELEMETRY_TOPIC_FMT, DEVICE_ID);
    snprintf(command_topic, sizeof(command_topic), COMMAND_TOPIC_FMT, DEVICE_ID);
    ESP_LOGI(TAG, "Telemetry topic: %s", telemetry_topic);
    ESP_LOGI(TAG, "Command topic: %s", command_topic);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi
    wifi_init();

    // Start MQTT client
    mqtt_app_start();

    // Start telemetry publishing task
    xTaskCreate(&telemetry_task, "telemetry_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "SmartSafe MQTT Client Started");
}

