#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include "config.h"

const String token = HTTP_TOKEN;

int value;
int values[30];
int avg;
int lastHttpCode;
int loopCounter;
int wifiConnectCounter;
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
  
  Serial.println("setup done");
}

void wlan() {
  if (WIFI_ENABLED && WiFi.status() != WL_CONNECTED) {
    wifiConnectCounter++;
    if (strlen(WIFI_DEVICE_NAME)) {
      WiFi.hostname(WIFI_DEVICE_NAME);
    }
    Serial.print("connect to wlan: ");
    Serial.print(WIFI_NAME);
    Serial.print(" (");
    Serial.print(wifiConnectCounter);
    Serial.println(". time)");
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);
    for (int n = 0; WiFi.status() != WL_CONNECTED; n++) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("Connecting to ");
      display.println(WIFI_NAME);
      display.print(wifiConnectCounter);
      display.println(". time");
      display.print("waiting... ");
      display.println(n);
      display.invertDisplay(n % 2);
      display.display();
      
      Serial.print(".");
      
      delay(1000);
    }
  }
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
  if (loopCounter == 29 && WIFI_ENABLED && HTTP_ENABLED) { // the moment when sending the value via http
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

  
  if (WIFI_ENABLED) {
    // 3rd line: wifi ip address or wifi status text if not connected
    display.setCursor(display.getCursorX() + 1, display.getCursorY());
    display.print(" IP: ");
    display.println(WiFi.localIP().toString());
    
    // 4th line: wifi signal strength (if connected)
    display.setCursor(display.getCursorX() + 1, display.getCursorY());
    display.print("SIG: ");
    display.print(pad(4, String(WiFi.RSSI())));
    display.print(" (");
    display.print(wifiConnectCounter);
    display.println(")");
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
  http.begin(client, HTTP_URL);
  http.addHeader("Content-Type", "application/json");
  String httpRequestData = "{\"token\":\"" + String(HTTP_TOKEN) + "\",\"value\":" + avg +"}";
  lastHttpCode = http.POST(httpRequestData);
  Serial.print("HTTP Response code: ");
  Serial.println(lastHttpCode);
  http.end();
}

void loop()
{
  Serial.println("-----loop-----");

  wlan();
  
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
  
  refreshDisplay();

  if (loopCounter == 29) {
    sendToHost();
  }

  loopCounter = (loopCounter + 1) % 30;

  delay(1000);
}
