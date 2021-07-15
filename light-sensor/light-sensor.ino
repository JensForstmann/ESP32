#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include "config.h"


const char* ssid = WIFI_NAME;
const char* password = WIFI_PASSWORD;
const char* url = HTTP_URL;
const String token = HTTP_TOKEN;

String my_ip;
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
  Serial.println("display done");
  display.display();

  Serial.print("init wlan");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("wlan done");
  Serial.print("my ip addresss: ");
  my_ip = WiFi.localIP().toString();
  Serial.println(my_ip);

  Serial.println("setup done");
}

void refreshDisplay() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(5);
  display.setCursor(0, 0);
  if (loopCounter == 29) {
    display.println("....");
  } else {
    display.println(value);  
  }
  display.setTextSize(1);
  display.print(" Avg: ");
  display.println(avg);
  display.print("Addr: ");
  display.println(my_ip);
  display.print("Code: ");
  display.println(lastHttpCode);
  
  display.display();
}

void sendToHost() {
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

void loop()
{
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
