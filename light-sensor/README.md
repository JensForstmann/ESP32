# light sensor

ESP32 based light sensor (light dependent resistor):

Every second:
- ensures the device is still connected to the wireless network
- reads the LDR connected to pin A0/GPIO36 (value ranges from 0 to 4095)
- calculates a rolling average from the last 30 LDR values
- displays some information on the 128*64 display
    - current LDR value
    - average
    - own IP address
    - wireless signal strength (RSSI) + the number of times the device connected to the network
    - Maybe: in case of an HTTP error: the HTTP status code
    - Maybe: if disconnected from the network a reconnect message will appear and the number of seconds it waited for a connection (display will also invert its color/pixels every second)

Every 30 seconds:
- sends the current average via HTTP POST request to an web server

## configuration

Example config.h

```cpp
#define WIFI_ENABLED true
#define WIFI_NAME "mywirelessnetwork"
#define WIFI_PASSWORD "supersecret"
#define WIFI_DEVICE_NAME "esp32-ldr-1"
#define HTTP_ENABLED true
#define HTTP_URL "http://10.0.0.1:6677/api/ldr"
#define HTTP_TOKEN "verysecret"
```

If `HTTP_ENABLED` is set to true, a json payload like this will be sent (POST) to the given address:

```json5
{
    "token": "verysecret", // HTTP_TOKEN
    "value": 1337, // the rolling average of the sensor
}
```

## circuit

ESP32 A0/ADC0/GPIO36 --+-- LDR ----- 3.3V (@ESP32)
                       |
                       +-- 10k Ohm ----- GND (@ESP32)

ESP32 I2C SCL/GPIO22 ----- SCL (@OLED display) ----- 10k Ohm ----- 3.3V (@ESP32)
ESP32 I2C SDA/GPIO21 ----- SDA (@OLED display) ----- 10k Ohm ----- 3.3V (@ESP32)

## components

- OLED display: AZDelivery 0,96 Zoll OLED Display I2C SSD1306 Chip 128 x 64 Pixel I2C
- ESP32: AZDelivery ESP32 NodeMCU Module WLAN WiFi Dev Kit C Development Board with CP2102
