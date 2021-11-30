/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "mqtt_implementation.h"

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    unsigned char mac_sta[6] = {0};
    getMAC(mac_sta);
    ESP_LOGI(TAG_MQTT, "%02X%02X%02X%02X%02X%02X", mac_sta[0],mac_sta[1],mac_sta[2],mac_sta[3],mac_sta[4],mac_sta[5]);
    char mac_from_device[15];
    memset(mac_from_device,0,sizeof(mac_from_device));
    sprintf(mac_from_device,"%02X%02X%02X%02X%02X%02X", mac_sta[0],mac_sta[1],mac_sta[2],mac_sta[3],mac_sta[4],mac_sta[5]);     
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_CONNECTED");
            // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            // ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
            char topic_For_Subscribe[35];
            memset(topic_For_Subscribe,0,sizeof(topic_For_Subscribe));
            sprintf(topic_For_Subscribe,"/FML_RGB_LED_FROM_SERVER_TO_DEVICE");
            msg_id = esp_mqtt_client_subscribe(client, topic_For_Subscribe, 0);
            ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);
            char mqtt_msg_to_publish[60];
            memset(mqtt_msg_to_publish,0,sizeof(mqtt_msg_to_publish));
            sprintf(mqtt_msg_to_publish,"{\"receiver\":\"%s\",\"cmd\":\"get grupo\"}",mac_from_device);
            msg_id = esp_mqtt_client_publish(client, "/FML_RGB_LED_FROM_DEVICE_TO_SERVER", mqtt_msg_to_publish, 0, 0, 0);
            ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
            // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            // ESP_LOGI(TAG_MQTT, "sent subscribe successful, msg_id=%d", msg_id);

            // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            // ESP_LOGI(TAG_MQTT, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            
            vTaskDelay( 100 / portTICK_PERIOD_MS );
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            msg_id = esp_mqtt_client_publish(client, "/FML_RGB_LED_FROM_DEVICE_TO_SERVER", "teste de volta", 0, 0, 0);
            ESP_LOGI(TAG_MQTT, "sent publish successful, msg_id=%d", msg_id);
            char mqtt_payload_as_array[100];
            memset(mqtt_payload_as_array,0,sizeof(mqtt_payload_as_array));
            sprintf(mqtt_payload_as_array,"%.*s",  event->data_len, event->data);
            printf("msg: %s\n", mqtt_payload_as_array);
            cJSON *mqtt_payload_as_json = cJSON_Parse(event->data);
            char *mqtt_cmd = cJSON_GetObjectItem(mqtt_payload_as_json,"cmd")->valuestring;
            char mqtt_cmd_as_array[50];
            memset(mqtt_cmd_as_array,0,sizeof(mqtt_cmd_as_array));
            sprintf(mqtt_cmd_as_array,"%s", mqtt_cmd);
            printf("mqtt cmd: %s\n", mqtt_cmd_as_array);

            char *mqtt_receiver = cJSON_GetObjectItem(mqtt_payload_as_json,"receiver")->valuestring;
            char mqtt_receiver_as_array[50];
            memset(mqtt_receiver_as_array,0,sizeof(mqtt_receiver_as_array));
            sprintf(mqtt_receiver_as_array,"%s", mqtt_receiver);
            printf("mqtt receiver as array: %s\n", mqtt_receiver_as_array);
            if(strcmp(mqtt_cmd_as_array,"set_values\0") == 0){
                if((strcmp(mqtt_receiver_as_array,mac_from_device) == 0) | (strcmp(mqtt_receiver_as_array,group_from_device) == 0)){
                    printf("mac ou grupo encontrado\n");
                    
                    int red_mqtt_value  = cJSON_GetObjectItem(mqtt_payload_as_json,"r_value")->valueint;
                    int green_mqtt_value = cJSON_GetObjectItem(mqtt_payload_as_json,"g_value")->valueint;
                    int blue_mqtt_value = cJSON_GetObjectItem(mqtt_payload_as_json,"b_value")->valueint;
                        struct MQTT_LED_INTERFACE xMessage = 
                        { 
                            .ucMessageID = 1 , 
                            .red_value = red_mqtt_value , 
                            .green_value = green_mqtt_value , 
                            .blue_value = blue_mqtt_value ,
                            .task_name = "mqtt_task"
                        };
                        /**
                         * Passa o endereço de xMessage para um Ponteiro;
                         */
                        struct MQTT_LED_INTERFACE * pxMessage = &xMessage;
                        if( xQueueSend( xQueue_mqtt_led_interface, ( void * ) &pxMessage, (10/portTICK_PERIOD_MS) ) == pdPASS )
                        {
                            ESP_LOGI( TAG_MQTT, "task_mqtt alterou os valores de xMessage %s, cujo ID: %d .\n", xMessage.task_name, xMessage.ucMessageID);
                            
                        }
                }
            }else if(strcmp(mqtt_cmd_as_array,"set_grupo\0") == 0){
                printf("set novo grupo\n");
                memset(group_from_device,0,sizeof(group_from_device));
                char *mqtt_new_group = cJSON_GetObjectItem(mqtt_payload_as_json,"grupo")->valuestring;
                memset(group_from_device,0,sizeof(group_from_device));
                sprintf(group_from_device,"%s", mqtt_new_group);
                printf("mqtt new group: %s\n", group_from_device);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG_MQTT, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG_MQTT, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG_MQTT, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG_MQTT, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void task_MQTT( void *pvParameter )
{
    printf("task mqtt\n");
    int sm_mqtt_task = 0;
    struct MQTT_CONNECTION_INTERFACE * msg_received_from_smart_config;
    for(;;){
        switch(sm_mqtt_task){
            case 0:
            {
                xQueueReceive( xQueue_mqtt_connection_interface, &msg_received_from_smart_config, portMAX_DELAY ); 	
                ESP_LOGI( TAG_MQTT, "Mensagem Recebida. Task: %s, ID = %d o valor foi: %d \n",msg_received_from_smart_config->task_name, msg_received_from_smart_config->ucMessageID,msg_received_from_smart_config->statemachine_state);
                if(msg_received_from_smart_config->statemachine_state == 1){
                    sm_mqtt_task = 1;
                }
            }
            break;
            case 1:
            {
                ESP_LOGI(TAG_MQTT, "[APP] Startup..");
                ESP_LOGI(TAG_MQTT, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
                ESP_LOGI(TAG_MQTT, "[APP] IDF version: %s", esp_get_idf_version());

                esp_log_level_set("*", ESP_LOG_INFO);
                esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
                esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
                esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
                esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
                esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
                esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

                ESP_ERROR_CHECK(nvs_flash_init());
                ESP_ERROR_CHECK(esp_netif_init());
                //ESP_ERROR_CHECK(esp_event_loop_create_default());

                /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
                * Read "Establishing Wi-Fi or Ethernet Connection" section in
                * examples/protocols/README.md for more information about this function.
                */
               // ESP_ERROR_CHECK(example_connect());    
                mqtt_app_start();
                sm_mqtt_task++;
                vTaskDelete( NULL );
            }
            break;
            case 2:
            {

            }
            break;
        }
        vTaskDelay( 100 / portTICK_PERIOD_MS );   
    }
}