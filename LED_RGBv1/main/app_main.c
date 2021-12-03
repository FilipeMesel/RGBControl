#include "mqtt_implementation.c"

static const char * TAG = "MAIN";
 void app_main(void)
 {
     memset(group_from_device,0,sizeof(group_from_device));
	/**
	 * Fila responsável em intercalar os dados do LED da task led para task mqtt;
	 */
	if( ( xQueue_mqtt_led_interface = xQueueCreate( 5,  sizeof( struct MQTT_LED_INTERFACE *  ) ) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar xQueue_mqtt_led_interface.\n" );
		return;
	} 
	/**
	 * Fila responsável em intercalar os dados da conecção da task MQTT para task Smart_config;
	 */
	if( ( xQueue_mqtt_connection_interface = xQueueCreate( 5,  sizeof( struct MQTT_CONNECTION_INTERFACE *  ) ) ) == NULL )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar xQueue_mqtt_connection_interface.\n" );
		return;
	} 
     /* configMINIMAL_STACK_SIZE é um #define que informa para o FreeRTOS qual o tamanho mínimo
	necessário para rodar uma task. Por meio do menuconfig é possível saber o tamanho de configMINIMAL_STACK_SIZE; 
	*/
    if( xTaskCreate( task_LED, "task_LED", 4048, NULL, 5, NULL ) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_led.\n" );	
		return;		
	}
    if( xTaskCreate( task_MQTT, "task_MQTT", 4048, NULL, 5, NULL ) != pdTRUE )
	{
		ESP_LOGI( TAG, "error - nao foi possivel alocar task_MQTT.\n" );	
		return;		
	}

    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
 }


