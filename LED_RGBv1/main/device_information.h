
#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define EXAMPLE_CHASE_SPEED_MS (1000)//10
static const char *TAG_DEVICE_INFORMATION = "DEVICE_INFORMATION";
uint32_t red = 0;
uint32_t green = 0;
uint32_t blue = 0;
uint16_t hue = 0;
uint16_t start_rgb = 0;
char group_from_device[50];
void task_LED( void *pvParameter );
void getMAC(unsigned char* mac_base);
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b);
void task_LED( void *pvParameter );
