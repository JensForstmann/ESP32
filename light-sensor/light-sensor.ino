#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include "config.h"

const char* ssid = WIFI_NAME;
const char* password = WIFI_PASSWORD;
const char* device_name = WIFI_DEVICE_NAME;
const char* url = HTTP_URL;
const String token = HTTP_TOKEN;

int value;
int values[30];
int avg;
int lastHttpCode;
int loopCounter;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup()
{
  Serial.begin(115200);

  Serial.println("init display");
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  Serial.println("display done");
  display.display();

  if (WIFI_ENABLED) {
    Serial.println("init wlan");
    if (strlen(device_name)) {
      WiFi.hostname(device_name);
    }
    WiFi.begin(ssid, password);
    Serial.println("wlan done");
  }
  
  Serial.println("setup done");
}

String pad(int width, String s) {
  int diff = width - s.length();
  for (int i = 0; i < diff; i++) {
    s = " " + s;
  }
  return s;
}

void refreshDisplay() {
  display.clearDisplay();

  // 1st (big) line: current ldr value
  display.setTextSize(5);
  display.setCursor(6, 1);
  if (loopCounter == 29 && WIFI_ENABLED && HTTP_ENABLED) {
    display.println("===>");
  } else {
    display.println(pad(4, String(value)));
  }
  
  display.setTextSize(1);

  // 2nd line: avg ldr value and possible http error message
  display.setCursor(display.getCursorX() + 1, display.getCursorY() - 1);
  display.print("AVG: ");
  display.print(pad(4, String(avg)));
  display.println(lastHttpCode == 200 || lastHttpCode == 0 ? "" : " - TX ERR");

  // 3rd line: wifi ip address or wifi status text if not connected
  display.setCursor(display.getCursorX() + 1, display.getCursorY());
  int wifi_status = WiFi.status();
  if (wifi_status == WL_CONNECTED) {
    display.print(" IP: ");
    display.println(WiFi.localIP().toString());
  } else {
    display.println(WiFiStautsText(wifi_status));
  }

  // 4th line: wifi signal strength (if connected)
  display.setCursor(display.getCursorX() + 1, display.getCursorY());
  if (wifi_status == WL_CONNECTED) {
    display.print("SIG: ");
    display.println(pad(4, String(WiFi.RSSI())));
  }

  // invert display in case of http error
  display.invertDisplay(lastHttpCode != 200 && lastHttpCode != 0);

  // render
  display.display();
}

void sendToHost() {
  if (!WIFI_ENABLED || !HTTP_ENABLED) {
    return;
  }
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
  String httpRequestData = "{\"token\":\"" + token + "\",\"ldr\":" + avg +"}";
  lastHttpCode = http.POST(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(lastHttpCode);
  http.end();
}

String WiFiStautsText(int n) {
  if (n == WL_CONNECTED) return "CONNECTED";
  if (n == WL_NO_SHIELD) return "NO_SHIELD";
  if (n == WL_IDLE_STATUS) return "IDLE_STATUS";
  if (n == WL_NO_SSID_AVAIL) return "NO_SSID_AVAIL";
  if (n == WL_SCAN_COMPLETED) return "SCAN_COMPLETED";
  if (n == WL_CONNECT_FAILED) return "CONNECT_FAILED";
  if (n == WL_CONNECTION_LOST) return "CONNECTION_LOST";
  if (n == WL_DISCONNECTED) return "DISCONNECTED";
  return "unkown";
}

void loop()
{
  Serial.println("-----loop-----");
  
  value = analogRead(A0);
  Serial.print("value: ");
  Serial.println(value);
  
  values[loopCounter] = value;
  int sum = 0;
  for (int i = 0; i < 30; i++) {
    sum += values[i];
  }
  Serial.print("sum: ");
  Serial.println(sum);

  avg = sum / 30;
  Serial.print("avg: ");
  Serial.println(avg);
  
  Serial.print("loopCounter: ");
  Serial.println(loopCounter);

  Serial.print("WiFi Status: ");
  Serial.println(WiFiStautsText(WiFi.status()));

  Serial.print("WiFi RSSI: ");
  Serial.println(WiFi.RSSI());
  
  refreshDisplay();

  if (loopCounter == 29) {
    sendToHost();
  }

  loopCounter = (loopCounter + 1) % 30;

  delay(1000);
}
