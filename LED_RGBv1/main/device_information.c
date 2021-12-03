#include "sdkconfig.h"
#include "driver/rmt.h"
#include "led_strip.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "protocol_examples_common.h"

#include "smart_config.c"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "cJSON.h"
#include "device_information.h"
/*
Função que pega o mac da esp
*/
void getMAC(unsigned char* mac_base){
    esp_efuse_mac_get_default(mac_base);
    esp_read_mac(mac_base, ESP_MAC_WIFI_STA);
    return mac_base;
}

/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

void task_LED( void *pvParameter )
{
    printf("task led\n");
   rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG_DEVICE_INFORMATION, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    // Show simple rainbow chasing pattern
    // Build RGB values
    hue = 0 * 360 / CONFIG_EXAMPLE_STRIP_LED_NUMBER + start_rgb;
    led_strip_hsv2rgb(hue, 100, 100, &red, &green, &blue);
    // Write RGB values to strip driver
    ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0, 0, 0));
    // Flush RGB values to LEDs
    ESP_ERROR_CHECK(strip->refresh(strip, 100));

    for(;;){
        //led_status_modify(0,0,255);
        // Clear LED strip (turn off all LEDs)
         //ESP_ERROR_CHECK(strip->clear(strip, 100));
         vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));     
        // Show simple rainbow chasing pattern
        // Build RGB values
         hue = 0 * 360 / CONFIG_EXAMPLE_STRIP_LED_NUMBER + start_rgb;
         led_strip_hsv2rgb(hue, 100, 100, &red, &green, &blue);
        // Write RGB values to strip driver
        struct MQTT_LED_INTERFACE * msg_received_from_mqtt;
        xQueueReceive( xQueue_mqtt_led_interface, &msg_received_from_mqtt, portMAX_DELAY ); 	
		ESP_LOGI( TAG_DEVICE_INFORMATION, "Mensagem Recebida. Task: %s, ID = %d \n",msg_received_from_mqtt->task_name, msg_received_from_mqtt->ucMessageID);
        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, msg_received_from_mqtt->red_value, msg_received_from_mqtt->green_value, msg_received_from_mqtt->blue_value));
        
        
        // Flush RGB values to LEDs
         ESP_ERROR_CHECK(strip->refresh(strip, 100));
        //vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
        vTaskDelay( 100 / portTICK_PERIOD_MS );           
    }
}
