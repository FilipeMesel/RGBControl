QueueHandle_t xQueue_mqtt_led_interface; 
QueueHandle_t xQueue_mqtt_connection_interface; 
/**
 * struct
 */
struct MQTT_LED_INTERFACE
{
	char ucMessageID;
	int red_value;
	int green_value;
	int blue_value;
	char *task_name;
};


struct MQTT_CONNECTION_INTERFACE
{
	char ucMessageID;
	int statemachine_state;
	char *task_name;
};