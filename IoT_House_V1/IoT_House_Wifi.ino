#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WebServer.h>

//DHT11 Temp and Humidity Sensor intialization
#define DHT_PIN 25     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHT_PIN, DHTTYPE);

#define TOUCH_PIN 33
#define LDR_PIN 14
#define GAS_PIN 34
#define RAIN_PIN 2
#define PIR_PIN 35
#define LED_LIGHT_PIN 12
#define RELAY_PIN 32
#define BUZZER_PIN 13
#define IR_PIN 18
#define LED_STRIP_PIN 15

// Store current states
bool ledState = false;
bool buzzerState = false;
bool relayState = false;
bool rainDetected = false;
bool nightDetected = false;
bool motionDetected = false;
bool allowToggle = true;

// LED Strip pattern mode
String stripMode = "off";

// Automation rules
String rainRule = "none";
String nightRule = "none";
String touchRule = "none";
String motionRule = "none";

//LCD Display screen size and address intilization
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 

Adafruit_NeoPixel strip(8, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

WebServer server(80);

enum State {
  STANDBY,
  GAS_LEAKAGE,
  AUTO_NIGHT_LIGHT,
  TOUCH_RELAY,
  RAIN_DETECTION,
  MOTION_DETECTION_ALARM,
  TEMP_AND_HUMID,
  LED_STRIP,
  BUZZER_BEEP_ON,
  MOTION_LIGHT,
  ALL_ACTIVITY_WIFI
};

byte ch0[8] = {
  0x01,
  0x02,
  0x06,
  0x0D,
  0x0D,
  0x19,
  0x1B,
  0x1B
};

byte ch1[8] = {
  0x07,
  0x0F,
  0x1F,
  0x1C,
  0x18,
  0x11,
  0x11,
  0x11
};

byte ch2[8] = {
  0x10,
  0x18,
  0x1C,
  0x1E,
  0x06,
  0x17,
  0x13,
  0x1B
};

byte ch3[8] = {
  0x1B,
  0x19,
  0x1D,
  0x0C,
  0x0F,
  0x07,
  0x03,
  0x01
};

byte ch4[8] = {
  0x11,
  0x11,
  0x11,
  0x03,
  0x07,
  0x1F,
  0x1E,
  0x1C
};

byte ch5[8] = {
  0x1B,
  0x1B,
  0x13,
  0x16,
  0x16,
  0x0C,
  0x08,
  0x10
};

State currentState = STANDBY;
int current_led_strip_brightness = 150;
unsigned long last_called = 0;
unsigned long start_duration = 0;
bool auto_switch = false;
long IR_Hex_Code = 0;

int led_strip_pattern = 0;
float gas_analog_value = 0;
float dht11_humidity = 0;
float dht11_temperature = 0;

const char* ssid = "REL_IoT_House";
const char* password = "012345678";
bool wifiRunning  = false;

//ALL WEBPAGE REALTED FUNCTION START -----------------------------------------------------------------
void startWiFi() {
  if (!wifiRunning) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
    wifiRunning = true;
  }
}

void stopWiFi() {
  if (wifiRunning) {
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    wifiRunning = false;
  }
}

const char webpage[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <title>Respire IoT House</title>

    <style>
    body {
      font-family: Arial, sans-serif;
      background: #f4f6f9;
      margin: 0;
      padding: 20px;
    }

    h3 {
      text-align: center;
    }

    h1{
      text-align: center;
      color: #c43e8b;
    }

    .center-image {
      display: block;
      margin-left: auto;
      margin-right: auto;
      width: 50%; 
    }

    .section {
      margin-bottom: 30px;
    }

    .container {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
    }

    .card {
      background: white;
      padding: 15px;
      border-radius: 10px;
      box-shadow: 0 4px 8px rgba(0,0,0,0.1);
    }

    /* Toggle Switch */
    .switch {
      position: relative;
      display: inline-block;
      width: 50px;
      height: 25px;
    }

    .switch input {
      display: none;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      background-color: #ccc;
      border-radius: 25px;
      top: 0; left: 0; right: 0; bottom: 0;
      transition: .4s;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 19px;
      width: 19px;
      left: 3px;
      bottom: 3px;
      background-color: white;
      border-radius: 50%;
      transition: .4s;
    }

    input:checked + .slider {
      background-color: #4CAF50;
    }

    input:checked + .slider:before {
      transform: translateX(25px);
    }

    select {
      padding: 6px;
      margin-top: 5px;
      width: 100%;
    }

    .value {
      font-weight: bold;
      font-size: 18px;
      margin-top: 8px;
    }
    </style>
    </head>

    <body>

    <img src="data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMTQ2IiBoZWlnaHQ9IjUwIiB2aWV3Qm94PSIwIDAgMTQ2IDUwIiBmaWxsPSJub25lIiB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciPgo8cGF0aCBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGNsaXAtcnVsZT0iZXZlbm9kZCIgZD0iTTgxLjE3NzkgNi41MDQ0MkM4Mi4wODU2IDYuMTc3NzQgODMuMDQyOSA2LjAxMDQgODQuMDA3NiA2LjAwOTc3QzkwLjA0NSA2LjAwOTc3IDk0LjkzOTQgMTIuNTMyNiA5NC45Mzk0IDIwLjU4MjNDOTQuOTM5NCAyOC42MzIxIDkwLjA0NSAzNS4xNTggODQuMDA3NiAzNS4xNThINzguMzQ1MUM3OS4zMTA4IDM1LjE1NzQgODAuMjY5MiAzNC45OTAxIDgxLjE3NzkgMzQuNjYzM0M4NS43ODY2IDMzLjAxMzQgODkuMjA5MyAyNy40MzM5IDg5LjI3MzggMjAuNzcyOEM4OS4yNzM4IDIwLjcwODMgODkuMjczOCAyMC42NDY5IDg5LjI3MzggMjAuNTgyM0M4OS4yNzM4IDIwLjUxNzggODkuMjczOCAyMC40NTY0IDg5LjI3MzggMjAuMzkxOUM4OS4yMDMxIDE2LjUyMzcgODYuOTgxOCAxMy40MTc0IDg0LjI1MDQgMTMuNDE3NEM4My42NjExIDEzLjQzNDggODMuMDgyNCAxMy41Nzc2IDgyLjU1MjYgMTMuODM2MUM4Mi4wMjI4IDE0LjA5NDYgODEuNTU0MiAxNC40NjMgODEuMTc3OSAxNC45MTY4Qzc5Ljk5MiAxNi4yMjg3IDc5LjIyNjkgMTguMjc4IDc5LjIyNjkgMjAuNTgyM0M3OS4yMjY5IDIyLjg4NjcgNzkuOTkyIDI0LjkzOTEgODEuMTc3OSAyNi4yNDc5QzgwLjgwMTEgMjYuNzAxMSA4MC4zMzIzIDI3LjA2OTEgNzkuODAyNyAyNy4zMjc1Qzc5LjI3MyAyNy41ODYgNzguNjk0NSAyNy43MjkxIDc4LjEwNTUgMjcuNzQ3M0M3NS4zNDAzIDI3Ljc0NzMgNzMuMDgyIDI0LjUzOTYgNzMuMDgyIDIwLjU4MjNDNzMuMDgyIDEzLjgzODMgNzYuNTIwMSA4LjE2NjYxIDgxLjE4NDEgNi41MDQ0MiIgZmlsbD0iI0ZGQTUzRCIvPgo8cGF0aCBmaWxsLXJ1bGU9ImV2ZW5vZGQiIGNsaXAtcnVsZT0iZXZlbm9kZCIgZD0iTTgxLjE5NTkgMzQuNjYzM0M4MC4yODc5IDM0Ljk4ODUgNzkuMzMwOCAzNS4xNTU4IDc4LjM2NjIgMzUuMTU4QzcyLjMyODkgMzUuMTU4IDY3LjQ0NjggMjguNjMyMSA2Ny40NDY4IDIwLjU4MjNDNjcuNDQ2OCAxMi41MzI2IDcyLjM0MTIgNi4wMDk3NyA3OC4zNzg1IDYuMDA5NzdIODQuMDM4QzgzLjA3MjYgNi4wMTA4MyA4Mi4xMTQ1IDYuMTc3MDkgODEuMjA1MiA2LjUwMTM1Qzc2LjU5NjUgOC4xNDgxOCA3My4xNzM4IDEzLjczMDggNzMuMTA5MyAyMC4zODg4QzczLjEwOTMgMjAuNDUzMyA3My4xMDkzIDIwLjUxNDggNzMuMTA5MyAyMC41NzkzQzczLjEwOTMgMjAuNjQzOCA3My4xMDkzIDIwLjcwNTIgNzMuMTA5MyAyMC43Njk4QzczLjE3OTkgMjQuNjM4IDc1LjQwMTMgMjcuNzQ0MiA3OC4xMzI3IDI3Ljc0NDJDNzguNzIxOSAyNy43MjY4IDc5LjMwMDcgMjcuNTg0MSA3OS44MzA1IDI3LjMyNTVDODAuMzYwMiAyNy4wNjcgODAuODI4OSAyNi42OTg2IDgxLjIwNTIgMjYuMjQ0OUM4Mi4zOTExIDI0LjkzNiA4My4xNTYyIDIyLjg4MzYgODMuMTU2MiAyMC41NzkzQzgzLjE1NjIgMTguMjc0OSA4Mi4zOTExIDE2LjIyNTYgODEuMjA1MiAxNC45MTM3QzgxLjU4MiAxNC40NjA1IDgyLjA1MDcgMTQuMDkyNiA4Mi41ODA0IDEzLjgzNDFDODMuMTEgMTMuNTc1NiA4My42ODg1IDEzLjQzMjUgODQuMjc3NiAxMy40MTQzQzg3LjA0MjggMTMuNDE0MyA4OS4zMDEgMTYuNjIyIDg5LjMwMSAyMC41NzkzQzg5LjMwMSAyNy4zMjMzIDg1Ljg2NjEgMzIuOTk1IDgxLjIwMjEgMzQuNjYwMyIgZmlsbD0iI0NBNDA5MCIvPgo8cGF0aCBkPSJNNjQuOTY3MyAyNy4zOTRDNjQuOTY3MyAyOS40ODYzIDY0LjAwMjUgMzEuMzIwNSA2Mi4wOTc2IDMyLjg0NDVDNjAuMTA5NyAzNC41MjIgNTcuNTg0MiAzNS4zMzkzIDU0LjM4MjcgMzUuMzM5M0M1Mi41NDM3IDM1LjMxOTcgNTAuNzIzNyAzNC45NjQ2IDQ5LjAxMjEgMzQuMjkxNkM0Ny43MSAzMy44MDk5IDQ2LjUxNDEgMzMuMDc5NCA0NS40OTExIDMyLjE0MDlDNDUuMzEwOCAzMS45Njc0IDQ1LjE3MDEgMzEuNzU3MSA0NS4wNzgzIDMxLjUyNDRDNDQuOTg2NiAzMS4yOTE3IDQ0Ljk0NjEgMzEuMDQxOSA0NC45NTk1IDMwLjc5MjFDNDQuOTc5OCAzMC40NzA4IDQ1LjA5MDYgMzAuMTYxOSA0NS4yNzkzIDI5LjkwMTFDNDUuNDY4IDI5LjY0MDQgNDUuNzI2OCAyOS40Mzg0IDQ2LjAyNTYgMjkuMzE4OEM0Ni4zMjQ0IDI5LjE5OTEgNDYuNjUxIDI5LjE2NjYgNDYuOTY3NSAyOS4yMjUxQzQ3LjI4NCAyOS4yODM2IDQ3LjU3NzUgMjkuNDMwNiA0Ny44MTM4IDI5LjY0OTFDNDguMjYzMyAzMC4wODM0IDQ4Ljc1NzYgMzAuNDY4NiA0OS4yODg2IDMwLjc5ODJDNTAuODQ3MyAzMS42NTMxIDUyLjYwNTcgMzIuMDc3MyA1NC4zODI3IDMyLjAyNzJDNTYuNzczMSAzMi4wMjcyIDU4LjYxNjUgMzEuNDUyNiA2MC4wMTc2IDMwLjI3OUM2MS4wOTkxIDI5LjM1NzIgNjEuNjQ5IDI4LjM4OTQgNjEuNjQ5IDI3LjM5N0M2MS42NDkgMjUuMjgwMSA2MC4yNjY0IDIzLjgxNzYgNTcuNDIxNCAyMi45MjY2QzU2LjczMDEgMjIuNzIwOCA1NS42NzMxIDIyLjM4OSA1NC4yNjYgMjEuOTUyN0w1MC42NTU5IDIwLjgzMTJDNDguNDM0NSAyMC4xMTIzIDQ2Ljc5MzggMTkuMDIxNiA0NS42MzI0IDE3LjQ5NDVDNDQuNjQ2NyAxNi4yNzY4IDQ0LjEwNDkgMTQuNzU5OCA0NC4wOTYyIDEzLjE5MzFDNDQuMDk2MiAxMS4xMDA4IDQ1LjA2MDkgOS4yNjY1NiA0Ni45NjI4IDcuNzQ1N0M0OC45NTA2IDYuMDY4MTUgNTEuNDc5MyA1LjI0NzggNTQuNjc3NyA1LjI0NzhDNTYuNTI1MiA1LjI2ODI1IDU4LjM1MzUgNS42MjUzNyA2MC4wNzI5IDYuMzAxNjVDNjEuMzY1NiA2Ljc4NzM5IDYyLjU1MjggNy41MTc2IDYzLjU2OTMgOC40NTIzNkM2My45MTEyIDguNzU3MTIgNjQuMTE3OSA5LjE4NTE5IDY0LjE0NDEgOS42NDI0QzY0LjE3MDQgMTAuMDk5NiA2NC4wMTM5IDEwLjU0ODUgNjMuNzA5MSAxMC44OTAzQzYzLjQwNDQgMTEuMjMyMiA2Mi45NzYzIDExLjQzODkgNjIuNTE5MSAxMS40NjUyQzYyLjA2MTkgMTEuNDkxNCA2MS42MTMgMTEuMzM0OSA2MS4yNzExIDExLjAzMDFDNjAuMTYyIDEwLjEwODQgNTcuODMzMSA4LjUyNjEgNTQuNzA1MyA4LjU3MjE5QzUyLjI2ODkgOC42MDkwNSA1MC40MjIzIDkuMTg2NjcgNDkuMDY3NCAxMC4zMjA0QzQ3Ljk4NTkgMTEuMjQyMSA0Ny40MzU5IDEyLjIxIDQ3LjQzNTkgMTMuMTk5M0M0Ny40MzU5IDE1LjI3OTMgNDguNzYwMiAxNi43MjM0IDUxLjQ4ODUgMTcuNjE0NEw1My4yMDI5IDE4LjE0MjhDNTQuNjY1NCAxOC41OTc2IDU2LjQxNjcgMTkuMTM4MyA1OC40MjYxIDE5Ljc2ODJDNjAuNjQ3NCAyMC40ODcxIDYyLjI4ODEgMjEuNTc3OCA2My40NDY0IDIzLjEwMThDNjQuNDI1NSAyNC4zMTg2IDY0Ljk2MTggMjUuODMyMSA2NC45NjczIDI3LjM5NFoiIGZpbGw9IiNDQTQwOTAiLz4KPHBhdGggZD0iTTQyLjQ4MzcgMTguODQ1NkM0Mi4xOTg2IDE1LjUxNDcgNDAuNzkwMiAxMi4zNzk1IDM4LjQ4OTUgOS45NTM5N0MzNS44MTY1IDcuMDMyMDggMzIuNTE2NyA1LjU1MTE3IDI4LjY4NTMgNS41NTExN0MyNi44MjUxIDUuNTE5NjQgMjQuOTgwMiA1Ljg5MTY4IDIzLjI3NzYgNi42NDE2OEMyMS41NzUgNy4zOTE2NyAyMC4wNTUyIDguNTAxOCAxOC44MjI4IDkuODk1NkMxNi4xNjIxIDEyLjgwNTIgMTQuODEwMiAxNi4zNzU0IDE0LjgxMDIgMjAuNTA0N0MxNC43NDU1IDI0LjQ0MzkgMTYuMTgyMyAyOC4yNiAxOC44Mjg5IDMxLjE3ODRDMjEuNTU3MyAzNC4wOTcyIDI0Ljg3NTUgMzUuNTc4MSAyOC42ODUzIDM1LjU3ODFDMzIuODk3NyAzNS41NzgxIDM2LjM5MSAzMy44Njk5IDM5LjA2NzEgMzAuNTAyNUMzOS4yMDc1IDMwLjMyNTQgMzkuMzA3NyAzMC4xMTk5IDM5LjM2MDggMjkuOTAwM0MzOS40MTM4IDI5LjY4MDYgMzkuNDE4NSAyOS40NTIxIDM5LjM3NDQgMjkuMjMwNUMzOS4zMzM1IDI5LjAxMTcgMzkuMjQ1NSAyOC44MDQ0IDM5LjExNjYgMjguNjIzQzM4Ljk4NzcgMjguNDQxNSAzOC44MjA5IDI4LjI5MDMgMzguNjI3OCAyOC4xNzk3TDM4LjU0NzkgMjguMTMzNkMzOC4yMjM3IDI3Ljk0NzggMzcuODQzNCAyNy44ODQ5IDM3LjQ3NjYgMjcuOTU2NkMzNy4xMDk5IDI4LjAyODIgMzYuNzgxMiAyOC4yMjk2IDM2LjU1MDggMjguNTIzOEMzNC40OTg0IDMxLjA4MzEgMzEuODUgMzIuMzc5NyAyOC42NzYxIDMyLjM3OTdDMjcuMzY1NCAzMi4zODYxIDI2LjA2NjcgMzIuMTMwNCAyNC44NTYzIDMxLjYyNzVDMjMuNjQ1OSAzMS4xMjQ3IDIyLjU0ODMgMzAuMzg0OSAyMS42Mjc5IDI5LjQ1MTdDMTkuNTE3MiAyNy40MzMxIDE4LjI4ODIgMjQuODAzMSAxOC4wMzkzIDIxLjYyNjJINDAuNDY4MUM0MC43NDYyIDIxLjYyNTYgNDEuMDIxMiAyMS41Njg5IDQxLjI3NjcgMjEuNDU5NEM0MS41MzIzIDIxLjM0OTkgNDEuNzYzMSAyMS4xOSA0MS45NTU0IDIwLjk4OTFDNDIuMTQ3NiAyMC43ODgzIDQyLjI5NzMgMjAuNTUwNyA0Mi4zOTU1IDIwLjI5MDZDNDIuNDkzNiAyMC4wMzA1IDQyLjUzODMgMTkuNzUzMiA0Mi41MjY3IDE5LjQ3NTVDNDIuNTI2NyAxOS4yMTQzIDQyLjUwNTIgMTguOTgzOSA0Mi40ODk4IDE4Ljg0MjVMNDIuNDgzNyAxOC44NDU2Wk0zOS4zNDk4IDE4LjQzMDhIMTguMDExN0MxOC40Mzg4IDE1LjUyNzQgMTkuNzEwNyAxMy4xNjQ3IDIxLjggMTEuMzk0OUMyMy44ODkzIDkuNjI1MjIgMjYuMTk5NyA4LjY5MTIgMjguNjg1MyA4LjY5MTJDMzEuMTcwOSA4LjY5MTIgMzMuNDI5MiA5LjYxMjkzIDM1LjUwOTIgMTEuMzk0OUMzNy41ODUxIDEzLjIwOTEgMzguOTQ2NyAxNS43MDM2IDM5LjM0OTggMTguNDMwOFoiIGZpbGw9IiNDQTQwOTAiLz4KPHBhdGggZD0iTTE3LjAxMzUgNy43MDI5MUMxNi44OTU0IDguMDM2NTYgMTYuNjY4NCA4LjMyMDc0IDE2LjM2OTEgOC41MDk2OEMxNi4wNjk4IDguNjk4NjIgMTUuNzE1NiA4Ljc4MTMyIDE1LjM2MzYgOC43NDQ0N0MxNS4wMjc5IDguNzA4MjkgMTQuNjkwNCA4LjY5MDg1IDE0LjM1MjggOC42OTIyNEMxMS43OTYzIDguNjU5MjMgOS4zMTk0MSA5LjU4MDUzIDcuNDA1OTYgMTEuMjc2MkM2LjQyMjMyIDEyLjExOTMgNS42MzY5MiAxMy4xNjkyIDUuMTA1ODIgMTQuMzUwOUM0LjU3NDcyIDE1LjUzMjYgNC4zMTEwMiAxNi44MTcgNC4zMzM1MiAxOC4xMTIzVjMzLjMyNEM0LjMzMzUyIDMzLjc0NzcgNC4xNjUyIDM0LjE1NDEgMy44NjU1NyAzNC40NTM3QzMuNTY1OTUgMzQuNzUzMyAzLjE1OTU4IDM0LjkyMTcgMi43MzU4NSAzNC45MjE3QzIuMzEyMTIgMzQuOTIxNyAxLjkwNTc1IDM0Ljc1MzMgMS42MDYxMyAzNC40NTM3QzEuMzA2NTEgMzQuMTU0MSAxLjEzODE4IDMzLjc0NzcgMS4xMzgxOCAzMy4zMjRWNy42NzgzM0MxLjEzODE4IDcuMjU0NiAxLjMwNjUxIDYuODQ4MjMgMS42MDYxMyA2LjU0ODYxQzEuOTA1NzUgNi4yNDg5OSAyLjMxMjEyIDYuMDgwNjYgMi43MzU4NSA2LjA4MDY2QzMuMTU5NTggNi4wODA2NiAzLjU2NTk1IDYuMjQ4OTkgMy44NjU1NyA2LjU0ODYxQzQuMTY1MiA2Ljg0ODIzIDQuMzMzNTIgNy4yNTQ2IDQuMzMzNTIgNy42NzgzM1Y5LjgyOTA0QzUuMjU1MjYgOC42MjE1NyA2LjU5NDg0IDcuNjM4MzkgOC4zNTUzNSA2Ljg1MTg1QzEwLjM4MzIgNS45ODM1IDEyLjU2NzggNS41NDExMyAxNC43NzM3IDUuNTUyMkMxNS4wODA5IDUuNTUyMiAxNS4zNTc0IDUuNTUyMjEgMTUuNjIxNyA1LjU3Njc4QzE1Ljg2OCA1LjU5MTk1IDE2LjEwNzMgNS42NjQzOSAxNi4zMjA3IDUuNzg4MzZDMTYuNTM0MSA1LjkxMjMyIDE2LjcxNTYgNi4wODQzOSAxNi44NTA3IDYuMjkwODVDMTYuOTg1OSA2LjQ5NzMxIDE3LjA3MSA2LjczMjQ3IDE3LjA5OTIgNi45Nzc2MkMxNy4xMjc1IDcuMjIyNzYgMTcuMDk4MSA3LjQ3MTExIDE3LjAxMzUgNy43MDI5MVoiIGZpbGw9IiNDQTQwOTAiLz4KPHBhdGggZD0iTTY4LjU3NDkgNi4wODgzOEM2OC4xNTA2IDYuMDg5MTkgNjcuNzQzOSA2LjI1ODEgNjcuNDQzOSA2LjU1ODEyQzY3LjE0MzggNi44NTgxNCA2Ni45NzQ5IDcuMjY0ODIgNjYuOTc0MSA3LjY4OTEyVjQ1Ljg2MTFDNjYuOTkyMiA0Ni4yNzMxIDY3LjE2ODYgNDYuNjYyMyA2Ny40NjY1IDQ2Ljk0NzRDNjcuNzY0NCA0Ny4yMzI2IDY4LjE2MDkgNDcuMzkxOCA2OC41NzMzIDQ3LjM5MThDNjguOTg1NyA0Ny4zOTE4IDY5LjM4MjIgNDcuMjMyNiA2OS42ODAyIDQ2Ljk0NzRDNjkuOTc4MSA0Ni42NjIzIDcwLjE1NDUgNDYuMjczMSA3MC4xNzI1IDQ1Ljg2MTFWNy42NzY4M0M3MC4xNjkzIDcuMjU0OTUgNjkuOTk5NiA2Ljg1MTQxIDY5LjcwMDUgNi41NTM5NUM2OS40MDEzIDYuMjU2NDkgNjguOTk2OCA2LjA4OTE4IDY4LjU3NDkgNi4wODgzOFoiIGZpbGw9IiNDQTQwOTAiLz4KPHBhdGggZD0iTTEyMC4xNDcgNy42Nzg0QzEyMC4yMzEgNy40NDY0MiAxMjAuMjU5IDcuMTk4MSAxMjAuMjMgNi45NTMyNEMxMjAuMiA2LjcwODM5IDEyMC4xMTQgNi40NzM3OSAxMTkuOTc4IDYuMjY4MTVDMTE5Ljg0MyA2LjA2MjI4IDExOS42NjEgNS44OTEyMyAxMTkuNDQ4IDUuNzY4OTVDMTE5LjIzNCA1LjY0NjY3IDExOC45OTUgNS41NzY2MyAxMTguNzQ5IDUuNTY0NTZDMTE4LjQ5NCA1LjU0NjEyIDExOC4yMiA1LjUzOTk4IDExNy44ODYgNS41Mzk5OEMxMTUuNjgxIDUuNTI5NSAxMTMuNDk3IDUuOTcxODUgMTExLjQ3IDYuODM5NjJDMTA5LjcxIDcuNjI2MTcgMTA4LjM3OSA4LjYwNjI3IDEwNy40NDggOS44MTM3NFY3LjY2MzAzQzEwNy40NDggNy4yMzk1NiAxMDcuMjc5IDYuODMzNjYgMTA2Ljk4IDYuNTM0MjFDMTA2LjY4IDYuMjM0NzcgMTA2LjI3NCA2LjA2NjE4IDEwNS44NTEgNi4wNjUzN0MxMDUuNDI3IDYuMDY1MzcgMTA1LjAyIDYuMjMzNTkgMTA0LjcyIDYuNTMzMTJDMTA0LjQyIDYuODMyNjYgMTA0LjI1MSA3LjIzOTAyIDEwNC4yNSA3LjY2MzAzVjMzLjI5MzNDMTA0LjI0MSAzMy41MDkyIDEwNC4yNzUgMzMuNzI0OCAxMDQuMzUxIDMzLjkyNzFDMTA0LjQyNyAzNC4xMjk0IDEwNC41NDMgMzQuMzE0MSAxMDQuNjkzIDM0LjQ3MDNDMTA0Ljg0MiAzNC42MjY0IDEwNS4wMjIgMzQuNzUwNyAxMDUuMjIxIDM0LjgzNTVDMTA1LjQxOSAzNC45MjA0IDEwNS42MzMgMzQuOTY0MiAxMDUuODQ5IDM0Ljk2NDJDMTA2LjA2NSAzNC45NjQyIDEwNi4yNzkgMzQuOTIwNCAxMDYuNDc4IDM0LjgzNTVDMTA2LjY3NyAzNC43NTA3IDEwNi44NTYgMzQuNjI2NCAxMDcuMDA2IDM0LjQ3MDNDMTA3LjE1NSAzNC4zMTQxIDEwNy4yNzEgMzQuMTI5NCAxMDcuMzQ3IDMzLjkyNzFDMTA3LjQyNCAzMy43MjQ4IDEwNy40NTggMzMuNTA5MiAxMDcuNDQ4IDMzLjI5MzNWMTguMTI0N0MxMDcuNDI2IDE2LjgyOTQgMTA3LjY5IDE1LjU0NTIgMTA4LjIyMSAxNC4zNjM1QzEwOC43NTIgMTMuMTgxOSAxMDkuNTM4IDEyLjEzMiAxMTAuNTIxIDExLjI4ODVDMTEyLjE4MSA5LjgxNjI3IDExNC4yNzMgOC45MjI0NyAxMTYuNDg1IDguNzQxNDZDMTE3LjE1NSA4LjY4NjczIDExNy44MjkgOC42ODY3MyAxMTguNSA4Ljc0MTQ2QzExOC44NTQgOC43NzYyMyAxMTkuMjEgOC42OTAwOCAxMTkuNTA5IDguNDk3MDZDMTE5LjgwOCA4LjMwNDA0IDEyMC4wMzMgOC4wMTU1IDEyMC4xNDcgNy42Nzg0WiIgZmlsbD0iI0ZGQTUzRCIvPgo8cGF0aCBkPSJNOTkuMzE5MSA2LjA5MDgyQzk4Ljg5NTEgNi4wOTA4MiA5OC40ODg0IDYuMjU5MDUgOTguMTg4MyA2LjU1ODU4Qzk3Ljg4ODEgNi44NTgxMiA5Ny43MTkxIDcuMjY0NDggOTcuNzE4MyA3LjY4ODQ5VjMzLjMxODhDOTcuNzA4OSAzMy41MzQ3IDk3Ljc0MzIgMzMuNzUwMyA5Ny44MTkzIDMzLjk1MjZDOTcuODk1NCAzNC4xNTQ4IDk4LjAxMTcgMzQuMzM5NiA5OC4xNjExIDM0LjQ5NTdDOTguMzEwNSAzNC42NTE5IDk4LjQ5IDM0Ljc3NjEgOTguNjg4OCAzNC44NjFDOTguODg3NSAzNC45NDU5IDk5LjEwMTQgMzQuOTg5NiA5OS4zMTc1IDM0Ljk4OTZDOTkuNTMzNiAzNC45ODk2IDk5Ljc0NzUgMzQuOTQ1OSA5OS45NDYzIDM0Ljg2MUMxMDAuMTQ1IDM0Ljc3NjEgMTAwLjMyNSAzNC42NTE5IDEwMC40NzQgMzQuNDk1N0MxMDAuNjIzIDM0LjMzOTYgMTAwLjc0IDM0LjE1NDggMTAwLjgxNiAzMy45NTI2QzEwMC44OTIgMzMuNzUwMyAxMDAuOTI2IDMzLjUzNDcgMTAwLjkxNyAzMy4zMTg4VjcuNjc2MkMxMDAuOTEzIDcuMjU0ODYgMTAwLjc0MyA2Ljg1MjEgMTAwLjQ0NCA2LjU1NTNDMTAwLjE0NCA2LjI1ODUgOTkuNzQwNCA2LjA5MTYxIDk5LjMxOTEgNi4wOTA4MloiIGZpbGw9IiNGRkE1M0QiLz4KPHBhdGggZD0iTTE0NS4xOCAxOC44NDg0QzE0NC44OTUgMTUuNTE3NCAxNDMuNDg2IDEyLjM4MjIgMTQxLjE4NiA5Ljk1NjczQzEzOC41MTYgNy4wMzQ4NCAxMzUuMjE2IDUuNTUzOTIgMTMxLjM4NSA1LjU1MzkyQzEyOS41MjUgNS41MjE4IDEyNy42OCA1Ljg5MzI4IDEyNS45NzcgNi42NDI3NUMxMjQuMjc1IDcuMzkyMjIgMTIyLjc1NSA4LjUwMTg4IDEyMS41MjIgOS44OTUyOEMxMTguODU4IDEyLjgwNDkgMTE3LjUwOSAxNi4zNzUxIDExNy41MDkgMjAuNTA0NEMxMTcuNDQ0IDI0LjQ0MjYgMTE4Ljg3OSAyOC4yNTgzIDEyMS41MjIgMzEuMTc4MUMxMjQuMjUzIDM0LjA5NjkgMTI3LjU2OSAzNS41Nzc4IDEzMS4zODIgMzUuNTc3OEMxMzUuNTk0IDM1LjU3NzggMTM5LjA4NyAzMy44Njk1IDE0MS43NjMgMzAuNTAyMUMxNDEuOTA0IDMwLjMyNTEgMTQyLjAwNCAzMC4xMTk2IDE0Mi4wNTcgMjkuOUMxNDIuMTEgMjkuNjgwMyAxNDIuMTE1IDI5LjQ1MTggMTQyLjA3MSAyOS4yMzAxQzE0Mi4wMyAyOS4wMTE0IDE0MS45NDIgMjguODA0MSAxNDEuODEzIDI4LjYyMjdDMTQxLjY4NCAyOC40NDEyIDE0MS41MTcgMjguMjkgMTQxLjMyNCAyOC4xNzk0TDE0MS4yNDQgMjguMTMzM0MxNDAuOTIgMjcuOTQ3NCAxNDAuNTQgMjcuODg0NiAxNDAuMTczIDI3Ljk1NjNDMTM5LjgwNiAyOC4wMjc5IDEzOS40NzcgMjguMjI5MyAxMzkuMjQ3IDI4LjUyMzVDMTM3LjE5NSAzMS4wODI4IDEzNC41NDYgMzIuMzc5NCAxMzEuMzcyIDMyLjM3OTRDMTMwLjA2MiAzMi4zODU4IDEyOC43NjMgMzIuMTMgMTI3LjU1MyAzMS42MjcyQzEyNi4zNDIgMzEuMTI0NCAxMjUuMjQ0IDMwLjM4NDYgMTI0LjMyNCAyOS40NTE0QzEyMi4xOTUgMjcuNDMyOCAxMjAuOTg0IDI0LjgwMjggMTIwLjcyNiAyMS42MjU5SDE0My4xNTVDMTQzLjQzMyAyMS42MjUzIDE0My43MDggMjEuNTY4NSAxNDMuOTY0IDIxLjQ1OTFDMTQ0LjIxOSAyMS4zNDk2IDE0NC40NSAyMS4xODk3IDE0NC42NDIgMjAuOTg4OEMxNDQuODM1IDIwLjc4OCAxNDQuOTg0IDIwLjU1MDQgMTQ1LjA4MiAyMC4yOTAzQzE0NS4xODEgMjAuMDMwMiAxNDUuMjI1IDE5Ljc1MjkgMTQ1LjIxNCAxOS40NzUxQzE0NS4yMTcgMTkuMjIwMSAxNDUuMjAxIDE4Ljk4OTcgMTQ1LjE4IDE4Ljg0ODRaTTEyNC40OTYgMTEuMzkxNkMxMjYuNTc5IDkuNTk3MjUgMTI4Ljg5NiA4LjY4NzgxIDEzMS4zODIgOC42ODc4MUMxMzMuODY3IDguNjg3ODEgMTM2LjEyNSA5LjYwOTU0IDEzOC4yMDIgMTEuMzkxNkMxNDAuMjgyIDEzLjIwNTYgMTQxLjY0OCAxNS43MDEgMTQyLjA1NSAxOC40MzA1SDEyMC43MTRDMTIxLjE0NCAxNS41MjQgMTIyLjQxNiAxMy4xNjEzIDEyNC41MDUgMTEuMzkxNkgxMjQuNDk2WiIgZmlsbD0iI0ZGQTUzRCIvPgo8cGF0aCBkPSJNOTkuMzEyNiAwLjA4MjI3NTRDOTguODYxMSAwLjA4MjI3NTQgOTguNDE5OCAwLjIxNjE2MSA5OC4wNDQzIDAuNDY3MDAxQzk3LjY2ODkgMC43MTc4NDEgOTcuMzc2MyAxLjA3NDM3IDk3LjIwMzYgMS40OTE1Qzk3LjAzMDggMS45MDg2MyA5Ni45ODU2IDIuMzY3NjMgOTcuMDczNiAyLjgxMDQ1Qzk3LjE2MTcgMy4yNTMyOCA5Ny4zNzkyIDMuNjYwMDQgOTcuNjk4NCAzLjk3OTNDOTguMDE3NyA0LjI5ODU2IDk4LjQyNDQgNC41MTU5OCA5OC44NjczIDQuNjA0MDZDOTkuMzEwMSA0LjY5MjE0IDk5Ljc2OTEgNC42NDY5NCAxMDAuMTg2IDQuNDc0MTVDMTAwLjYwMyA0LjMwMTM3IDEwMC45NiA0LjAwODc4IDEwMS4yMTEgMy42MzMzN0MxMDEuNDYyIDMuMjU3OTYgMTAxLjU5NSAyLjgxNjYgMTAxLjU5NSAyLjM2NTFDMTAxLjU5NSAxLjc1OTY2IDEwMS4zNTUgMS4xNzkwMSAxMDAuOTI3IDAuNzUwODk4QzEwMC40OTkgMC4zMjI3ODUgOTkuOTE4IDAuMDgyMjc1NCA5OS4zMTI2IDAuMDgyMjc1NFoiIGZpbGw9IiNGRkE1M0QiLz4KPHBhdGggZD0iTTUuNDM2MTEgNDYuNjMyM0gxLjEwMDI5VjM5LjUyMzlINS40MzYxMVY0MC40MDg4SDIuMDYzOFY0Mi42MzA4SDUuMTAxODNWNDMuNDg2MUgyLjA2MzhWNDUuNzM3Nkg1LjQzNjExVjQ2LjYzMjNaTTguMTQ5NzcgNDYuNjMyM0g3LjA4NzkzTDguNzY5MTcgNDQuMjUzTDcuMDk3NzcgNDEuODI0Nkg4LjE2OTQzTDkuMzg4NTggNDMuNjE0TDEwLjU3ODIgNDEuODI0NkgxMS42MzAyTDkuOTU4ODIgNDQuMjUzTDExLjYwMDcgNDYuNjMyM0gxMC41MjkxTDkuMzI5NTkgNDQuODcyNEw4LjE0OTc3IDQ2LjYzMjNaTTEzLjM4NDQgNDguNzg1NVY0MS44MjQ2SDE0LjIyMDFMMTQuMjg5IDQyLjY4OThDMTQuNjEzNCA0Mi4wMzEgMTUuMjYyMyA0MS42ODY5IDE2LjAyOTIgNDEuNjg2OUMxNy40MDU2IDQxLjY4NjkgMTguMjYxIDQyLjcxOTMgMTguMjYxIDQ0LjE5NEMxOC4yNjEgNDUuNjU5IDE3LjQ1NDggNDYuNzYwMSAxNi4wMjkyIDQ2Ljc2MDFDMTUuMjYyMyA0Ni43NjAxIDE0LjYyMzIgNDYuNDM1NyAxNC4zMDg2IDQ1Ljg0NThWNDguNzg1NUgxMy4zODQ0Wk0xNC4zMTg1IDQ0LjIzMzRDMTQuMzE4NSA0NS4yMTY1IDE0Ljg4ODcgNDUuOTI0NCAxNS44MzI2IDQ1LjkyNDRDMTYuNzc2NCA0NS45MjQ0IDE3LjMzNjggNDUuMjE2NSAxNy4zMzY4IDQ0LjIzMzRDMTcuMzM2OCA0My4yNDA0IDE2Ljc3NjQgNDIuNTMyNSAxNS44MzI2IDQyLjUzMjVDMTQuODg4NyA0Mi41MzI1IDE0LjMxODUgNDMuMjMwNSAxNC4zMTg1IDQ0LjIzMzRaTTIyLjMzNzQgNDYuNzUwM0MyMC45MjE2IDQ2Ljc1MDMgMTkuOTQ4MiA0NS43Mjc4IDE5Ljk0ODIgNDQuMjMzNEMxOS45NDgyIDQyLjcyOTEgMjAuOTAxOSA0MS42ODY5IDIyLjI5OCA0MS42ODY5QzIzLjY2NDcgNDEuNjg2OSAyNC41NTk0IDQyLjYzMDggMjQuNTU5NCA0NC4wNTY0VjQ0LjQwMDVMMjAuODQyOSA0NC40MTAzQzIwLjkxMTggNDUuNDIzIDIxLjQ0MjcgNDUuOTgzNCAyMi4zNTcgNDUuOTgzNEMyMy4wNzQ4IDQ1Ljk4MzQgMjMuNTQ2NyA0NS42ODg1IDIzLjcwNCA0NS4xMzc5SDI0LjU2OTJDMjQuMzMzMiA0Ni4xNzAyIDIzLjUyNyA0Ni43NTAzIDIyLjMzNzQgNDYuNzUwM1pNMjIuMjk4IDQyLjQ2MzZDMjEuNDkxOCA0Mi40NjM2IDIwLjk4MDYgNDIuOTQ1NCAyMC44NjI2IDQzLjc5MDlIMjMuNjM1MkMyMy42MzUyIDQyLjk5NDYgMjMuMTE0MSA0Mi40NjM2IDIyLjI5OCA0Mi40NjM2Wk0yOS4zMjc3IDQxLjc4NTJWNDIuNjMwOEgyOC45MTQ4QzI4LjAzOTcgNDIuNjMwOCAyNy40OTkgNDMuMTYxNyAyNy40OTkgNDQuMDg1OVY0Ni42MzIzSDI2LjU3NDhWNDEuODM0NEgyNy40NEwyNy40OTkgNDIuNTYyQzI3LjY5NTYgNDIuMDYwNSAyOC4xNzc0IDQxLjcyNjMgMjguODM2MSA0MS43MjYzQzI5LjAwMzMgNDEuNzI2MyAyOS4xNDA5IDQxLjc0NTkgMjkuMzI3NyA0MS43ODUyWk0zMS42MyA0MC42ODQxQzMxLjI5NTcgNDAuNjg0MSAzMS4wMjA0IDQwLjQwODggMzEuMDIwNCA0MC4wNzQ1QzMxLjAyMDQgMzkuNzMwNCAzMS4yOTU3IDM5LjQ2NDkgMzEuNjMgMzkuNDY0OUMzMS45NjQzIDM5LjQ2NDkgMzIuMjM5NSAzOS43MzA0IDMyLjIzOTUgNDAuMDc0NUMzMi4yMzk1IDQwLjQwODggMzEuOTY0MyA0MC42ODQxIDMxLjYzIDQwLjY4NDFaTTMxLjE3NzcgNDYuNjMyM1Y0MS44MjQ2SDMyLjEwMTlWNDYuNjMyM0gzMS4xNzc3Wk0zNi41MDEzIDQ2Ljc1MDNDMzUuMDg1NSA0Ni43NTAzIDM0LjExMjIgNDUuNzI3OCAzNC4xMTIyIDQ0LjIzMzRDMzQuMTEyMiA0Mi43MjkxIDM1LjA2NTkgNDEuNjg2OSAzNi40NjIgNDEuNjg2OUMzNy44Mjg2IDQxLjY4NjkgMzguNzIzMyA0Mi42MzA4IDM4LjcyMzMgNDQuMDU2NFY0NC40MDA1TDM1LjAwNjkgNDQuNDEwM0MzNS4wNzU3IDQ1LjQyMyAzNS42MDY2IDQ1Ljk4MzQgMzYuNTIxIDQ1Ljk4MzRDMzcuMjM4NyA0NS45ODM0IDM3LjcxMDYgNDUuNjg4NSAzNy44Njc5IDQ1LjEzNzlIMzguNzMzMUMzOC40OTcyIDQ2LjE3MDIgMzcuNjkxIDQ2Ljc1MDMgMzYuNTAxMyA0Ni43NTAzWk0zNi40NjIgNDIuNDYzNkMzNS42NTU4IDQyLjQ2MzYgMzUuMTQ0NSA0Mi45NDU0IDM1LjAyNjYgNDMuNzkwOUgzNy43OTkxQzM3Ljc5OTEgNDIuOTk0NiAzNy4yNzggNDIuNDYzNiAzNi40NjIgNDIuNDYzNlpNNDEuNjYyOSA0Ni42MzIzSDQwLjczODhWNDEuODI0Nkg0MS41NzQ1TDQxLjY3MjggNDIuNTYyQzQxLjk3NzYgNDIuMDExNCA0Mi41ODcxIDQxLjY4NjkgNDMuMjY1NSA0MS42ODY5QzQ0LjUzMzggNDEuNjg2OSA0NS4xMTM5IDQyLjQ2MzYgNDUuMTEzOSA0My42OTI2VjQ2LjYzMjNINDQuMTg5N1Y0My44OTkxQzQ0LjE4OTcgNDIuOTI1NyA0My43Mzc1IDQyLjUzMjUgNDMuMDI5NiA0Mi41MzI1QzQyLjE2NDQgNDIuNTMyNSA0MS42NjI5IDQzLjE1MTkgNDEuNjYyOSA0NC4wODU5VjQ2LjYzMjNaTTQ4LjczMzkgNDYuNjMyM0g0Ny44MDk3VjQyLjYwMTNINDYuODY1OVY0MS44MjQ2SDQ3LjgwOTdWNDAuMzIwM0g0OC43MzM5VjQxLjgyNDZINDkuNjc3OFY0Mi42MDEzSDQ4LjczMzlWNDYuNjMyM1pNNTEuOTQyNiA0MC42ODQxQzUxLjYwODQgNDAuNjg0MSA1MS4zMzMxIDQwLjQwODggNTEuMzMzMSA0MC4wNzQ1QzUxLjMzMzEgMzkuNzMwNCA1MS42MDg0IDM5LjQ2NDkgNTEuOTQyNiAzOS40NjQ5QzUyLjI3NjkgMzkuNDY0OSA1Mi41NTIyIDM5LjczMDQgNTIuNTUyMiA0MC4wNzQ1QzUyLjU1MjIgNDAuNDA4OCA1Mi4yNzY5IDQwLjY4NDEgNTEuOTQyNiA0MC42ODQxWk01MS40OTA0IDQ2LjYzMjNWNDEuODI0Nkg1Mi40MTQ2VjQ2LjYzMjNINTEuNDkwNFpNNTYuMTQ1NCA0Ni43NTAzQzU1LjExMzEgNDYuNzUwMyA1NC41MDM1IDQ2LjE3MDIgNTQuNTAzNSA0NS4yOTUyQzU0LjUwMzUgNDQuNDEwMyA1NS4xNjIyIDQzLjg1OTggNTYuMjkyOSA0My43NzEzTDU3LjgwNyA0My42NTMzVjQzLjUxNTZDNTcuODA3IDQyLjcwOTQgNTcuMzI1MiA0Mi40MjQzIDU2LjY3NjMgNDIuNDI0M0M1NS44OTk2IDQyLjQyNDMgNTUuNDU3MiA0Mi43Njg0IDU1LjQ1NzIgNDMuMzQ4NUg1NC42NTFDNTQuNjUxIDQyLjM0NTcgNTUuNDc2OSA0MS42ODY5IDU2LjcxNTcgNDEuNjg2OUM1Ny45MDUzIDQxLjY4NjkgNTguNzExNSA0Mi4zMTYyIDU4LjcxMTUgNDMuNjE0VjQ2LjYzMjNINTcuOTI1TDU3LjgyNjcgNDUuODU1NkM1Ny41ODA5IDQ2LjQwNjIgNTYuOTIyMSA0Ni43NTAzIDU2LjE0NTQgNDYuNzUwM1pNNTYuNDEwOSA0Ni4wMzI2QzU3LjI4NTkgNDYuMDMyNiA1Ny44MTY4IDQ1LjQ2MjMgNTcuODE2OCA0NC41NTc4VjQ0LjMwMjJMNTYuNTg3OSA0NC40MDA1QzU1Ljc3MTggNDQuNDc5MiA1NS40Mzc1IDQ0Ljc5MzggNTUuNDM3NSA0NS4yNjU3QzU1LjQzNzUgNDUuNzc3IDU1LjgxMTEgNDYuMDMyNiA1Ni40MTA5IDQ2LjAzMjZaTTYxLjkxODUgNDYuNjMyM0g2MC45OTQzVjM5LjM5NjFINjEuOTE4NVY0Ni42MzIzWiIgZmlsbD0iI0NBNDA5MCIvPgo8cGF0aCBkPSJNNzUuNTUwMSAzOS41MjM5VjQ1LjczNzZINzguNjc2NlY0Ni42MzIzSDc0LjU4NjZWMzkuNTIzOUg3NS41NTAxWk04Mi41Mjc3IDQ2Ljc1MDNDODEuMTExOSA0Ni43NTAzIDgwLjEzODYgNDUuNzI3OCA4MC4xMzg2IDQ0LjIzMzRDODAuMTM4NiA0Mi43MjkxIDgxLjA5MjMgNDEuNjg2OSA4Mi40ODg0IDQxLjY4NjlDODMuODU1IDQxLjY4NjkgODQuNzQ5NyA0Mi42MzA4IDg0Ljc0OTcgNDQuMDU2NFY0NC40MDA1TDgxLjAzMzMgNDQuNDEwM0M4MS4xMDIxIDQ1LjQyMyA4MS42MzMgNDUuOTgzNCA4Mi41NDc0IDQ1Ljk4MzRDODMuMjY1MSA0NS45ODM0IDgzLjczNyA0NS42ODg1IDgzLjg5NDMgNDUuMTM3OUg4NC43NTk1Qzg0LjUyMzYgNDYuMTcwMiA4My43MTc0IDQ2Ljc1MDMgODIuNTI3NyA0Ni43NTAzWk04Mi40ODg0IDQyLjQ2MzZDODEuNjgyMiA0Mi40NjM2IDgxLjE3MDkgNDIuOTQ1NCA4MS4wNTI5IDQzLjc5MDlIODMuODI1NUM4My44MjU1IDQyLjk5NDYgODMuMzA0NCA0Mi40NjM2IDgyLjQ4ODQgNDIuNDYzNlpNODguMTIzMSA0Ni43NTAzQzg3LjA5MDggNDYuNzUwMyA4Ni40ODEyIDQ2LjE3MDIgODYuNDgxMiA0NS4yOTUyQzg2LjQ4MTIgNDQuNDEwMyA4Ny4xMzk5IDQzLjg1OTggODguMjcwNiA0My43NzEzTDg5Ljc4NDcgNDMuNjUzM1Y0My41MTU2Qzg5Ljc4NDcgNDIuNzA5NCA4OS4zMDI5IDQyLjQyNDMgODguNjU0IDQyLjQyNDNDODcuODc3MyA0Mi40MjQzIDg3LjQzNDkgNDIuNzY4NCA4Ny40MzQ5IDQzLjM0ODVIODYuNjI4N0M4Ni42Mjg3IDQyLjM0NTcgODcuNDU0NSA0MS42ODY5IDg4LjY5MzMgNDEuNjg2OUM4OS44ODMgNDEuNjg2OSA5MC42ODkyIDQyLjMxNjIgOTAuNjg5MiA0My42MTRWNDYuNjMyM0g4OS45MDI3TDg5LjgwNDMgNDUuODU1NkM4OS41NTg1IDQ2LjQwNjIgODguODk5OCA0Ni43NTAzIDg4LjEyMzEgNDYuNzUwM1pNODguMzg4NiA0Ni4wMzI2Qzg5LjI2MzYgNDYuMDMyNiA4OS43OTQ1IDQ1LjQ2MjMgODkuNzk0NSA0NC41NTc4VjQ0LjMwMjJMODguNTY1NSA0NC40MDA1Qzg3Ljc0OTUgNDQuNDc5MiA4Ny40MTUyIDQ0Ljc5MzggODcuNDE1MiA0NS4yNjU3Qzg3LjQxNTIgNDUuNzc3IDg3Ljc4ODggNDYuMDMyNiA4OC4zODg2IDQ2LjAzMjZaTTk1LjcwNTIgNDEuNzg1MlY0Mi42MzA4SDk1LjI5MjJDOTQuNDE3MiA0Mi42MzA4IDkzLjg3NjUgNDMuMTYxNyA5My44NzY1IDQ0LjA4NTlWNDYuNjMyM0g5Mi45NTIzVjQxLjgzNDRIOTMuODE3NUw5My44NzY1IDQyLjU2MkM5NC4wNzMxIDQyLjA2MDUgOTQuNTU0OSA0MS43MjYzIDk1LjIxMzYgNDEuNzI2M0M5NS4zODA3IDQxLjcyNjMgOTUuNTE4NCA0MS43NDU5IDk1LjcwNTIgNDEuNzg1MlpNOTguNDc5NCA0Ni42MzIzSDk3LjU1NTJWNDEuODI0Nkg5OC4zOTA5TDk4LjQ4OTIgNDIuNTYyQzk4Ljc5NCA0Mi4wMTE0IDk5LjQwMzYgNDEuNjg2OSAxMDAuMDgyIDQxLjY4NjlDMTAxLjM1IDQxLjY4NjkgMTAxLjkzIDQyLjQ2MzYgMTAxLjkzIDQzLjY5MjZWNDYuNjMyM0gxMDEuMDA2VjQzLjg5OTFDMTAxLjAwNiA0Mi45MjU3IDEwMC41NTQgNDIuNTMyNSA5OS44NDYgNDIuNTMyNUM5OC45ODA4IDQyLjUzMjUgOTguNDc5NCA0My4xNTE5IDk4LjQ3OTQgNDQuMDg1OVY0Ni42MzIzWk0xMDQuNjQ2IDQwLjY4NDFDMTA0LjMxMiA0MC42ODQxIDEwNC4wMzYgNDAuNDA4OCAxMDQuMDM2IDQwLjA3NDVDMTA0LjAzNiAzOS43MzA0IDEwNC4zMTIgMzkuNDY0OSAxMDQuNjQ2IDM5LjQ2NDlDMTA0Ljk4IDM5LjQ2NDkgMTA1LjI1NSAzOS43MzA0IDEwNS4yNTUgNDAuMDc0NUMxMDUuMjU1IDQwLjQwODggMTA0Ljk4IDQwLjY4NDEgMTA0LjY0NiA0MC42ODQxWk0xMDQuMTk0IDQ2LjYzMjNWNDEuODI0NkgxMDUuMTE4VjQ2LjYzMjNIMTA0LjE5NFpNMTA4LjM2NyA0Ni42MzIzSDEwNy40NDNWNDEuODI0NkgxMDguMjc4TDEwOC4zNzcgNDIuNTYyQzEwOC42ODEgNDIuMDExNCAxMDkuMjkxIDQxLjY4NjkgMTA5Ljk2OSA0MS42ODY5QzExMS4yMzggNDEuNjg2OSAxMTEuODE4IDQyLjQ2MzYgMTExLjgxOCA0My42OTI2VjQ2LjYzMjNIMTEwLjg5NFY0My44OTkxQzExMC44OTQgNDIuOTI1NyAxMTAuNDQxIDQyLjUzMjUgMTA5LjczMyA0Mi41MzI1QzEwOC44NjggNDIuNTMyNSAxMDguMzY3IDQzLjE1MTkgMTA4LjM2NyA0NC4wODU5VjQ2LjYzMjNaTTExMy43NzYgNDQuMTA1NUMxMTMuNzc2IDQyLjc0ODggMTE0LjY1MSA0MS42ODY5IDExNi4wNDcgNDEuNjg2OUMxMTYuODI0IDQxLjY4NjkgMTE3LjQyNCA0Mi4wNDA5IDExNy43MjkgNDIuNjQwNkwxMTcuNzk3IDQxLjgyNDZIMTE4LjYyM1Y0Ni40MzU3QzExOC42MjMgNDcuOTQ5OCAxMTcuNjg5IDQ4LjkxMzMgMTE2LjIxNSA0OC45MTMzQzExNC45MDcgNDguOTEzMyAxMTQuMDEyIDQ4LjE3NTkgMTEzLjgzNSA0Ni45NTY4SDExNC43NTlDMTE0Ljg3NyA0Ny42NjQ3IDExNS40MDggNDguMDc3NiAxMTYuMjE1IDQ4LjA3NzZDMTE3LjExOSA0OC4wNzc2IDExNy43MDkgNDcuNDg3NyAxMTcuNzA5IDQ2LjU3MzNWNDUuNjA5OEMxMTcuMzk0IDQ2LjE3MDIgMTE2Ljc2NSA0Ni41MDQ1IDExNi4wMDggNDYuNTA0NUMxMTQuNjQxIDQ2LjUwNDUgMTEzLjc3NiA0NS40NTI1IDExMy43NzYgNDQuMTA1NVpNMTE0LjcgNDQuMDg1OUMxMTQuNyA0NC45OTA0IDExNS4yNjEgNDUuNjk4MyAxMTYuMTY1IDQ1LjY5ODNDMTE3LjA5OSA0NS42OTgzIDExNy42NyA0NS4wMjk3IDExNy42NyA0NC4wODU5QzExNy42NyA0My4xNjE3IDExNy4xMTkgNDIuNDkzMSAxMTYuMTc1IDQyLjQ5MzFDMTE1LjI1MSA0Mi40OTMxIDExNC43IDQzLjIwMSAxMTQuNyA0NC4wODU5WiIgZmlsbD0iI0NBNDA5MCIvPgo8L3N2Zz4K"width="150" height="75" class="center-image">

    <h1>IoT House</h1>
    <h3>Real Time Monitoring</h3>

    <!-- ============ MONITORING SECTION ============ -->

    <div class="section">
    <div class="container">

    <div class="card">
    <h3>Rain</h3>
    <div class="value" id="rainValue">--</div>
    </div>

    <div class="card">
    <h3>Night</h3>
    <div class="value" id="nightValue">--</div>
    </div>

    <div class="card">
    <h3>Gas</h3>
    <div class="value" id="gasValue">--</div>
    </div>

    <div class="card">
    <h3>Motion</h3>
    <div class="value" id="motionValue">--</div>
    </div>

    <div class="card">
    <h3>Temperature</h3>
    <div class="value" id="tempValue">-- °C</div>
    </div>

    <div class="card">
    <h3>Humidity</h3>
    <div class="value" id="humidValue">-- %</div>
    </div>

    </div>
    </div>


    <!-- ============ SETTINGS SECTION ============ -->

    <div class="section">
    <h3>Settings</h3>

    <div class="container">

    <div class="card">
    <p>LED
    <label class="switch">
    <input type="checkbox" onchange="toggleDevice('led', this.checked)">
    <span class="slider"></span>
    </label>
    </p>

    <p>Buzzer
    <label class="switch">
    <input type="checkbox" onchange="toggleDevice('buzzer', this.checked)">
    <span class="slider"></span>
    </label>
    </p>

    <p>Relay
    <label class="switch">
    <input type="checkbox" onchange="toggleDevice('relay', this.checked)">
    <span class="slider"></span>
    </label>
    </p>

    <p>LED Strip Pattern</p>
    <select onchange="setPattern(this.value)">
    <option value="off">OFF</option>
    <option value="CW">COLOR WIPE</option>
    <option value="RGB_FADE">RGB FADE</option>
    <option value="PC1">POLICE CHASE 1</option>
    <option value="PC2">POLICE CHASE 2</option>
    <option value="BB">BOUNCE BALL</option>
    <option value="RL">RUNNING LIGHT</option>
    <option value="RB">RAINBOW</option>
    </select>

    </div>

    </div>
    </div>


    <!-- ============ AUTOMATION SECTION ============ -->

    <div class="section">
    <h3>Automation Rules</h3>

    <div class="container">

    <div class="card">
    <p>If Rain then</p>
    <select onchange="setRule('rain', this.value)">
    <option value="none">Do Nothing</option>
    <option value="led">Turn ON LED</option>
    <option value="relay">Turn ON Relay</option>
    <option value="buzzer">Turn ON Buzzer</option>
    <option value="led_strip">Turn ON LED Strip</option>
    </select>
    </div>

    <div class="card">
    <p>If Night then</p>
    <select onchange="setRule('night', this.value)">
    <option value="none">Do Nothing</option>
    <option value="led">Turn ON LED</option>
    <option value="relay">Turn ON Relay</option>
    <option value="buzzer">Turn ON Buzzer</option>
    <option value="led_strip">Turn ON LED Strip</option>
    </select>
    </div>

    <div class="card">
    <p>If Touch then</p>
    <select onchange="setRule('touch', this.value)">
    <option value="none">Do Nothing</option>
    <option value="led">Turn ON LED</option>
    <option value="relay">Turn ON Relay</option>
    <option value="buzzer">Turn ON Buzzer</option>
    <option value="led_strip">Turn ON LED Strip</option>
    </select>
    </div>

    <div class="card">
    <p>If Motion then</p>
    <select onchange="setRule('motion', this.value)">
    <option value="none">Do Nothing</option>
    <option value="led">Turn ON LED</option>
    <option value="relay">Turn ON Relay</option>
    <option value="buzzer">Turn ON Buzzer</option>
    <option value="led_strip">Turn ON LED Strip</option>
    </select>
    </div>

    </div>
    </div>


    <script>

    // ==== Manual Toggle ====
    function toggleDevice(device, state) {
      let value = state ? "on" : "off";
      fetch("/control?device=" + device + "&state=" + value);
    }

    // ==== LED Strip Pattern ====
    function setPattern(pattern) {
      fetch("/setPattern?mode=" + pattern);
    }

    // ==== Automation Rule ====
    function setRule(trigger, action) {
      fetch("/setRule?trigger=" + trigger + "&action=" + action);
    }

    // ==== Live Sensor Update ====
    function updateSensors() {
      fetch("/sensorData")
        .then(response => response.json())
        .then(data => {
          document.getElementById("rainValue").innerText = data.rain;
          document.getElementById("nightValue").innerText = data.night;
          document.getElementById("gasValue").innerText = data.gas;
          document.getElementById("motionValue").innerText = data.motion;
          document.getElementById("tempValue").innerText = data.temp + " °C";
          document.getElementById("humidValue").innerText = data.humid + " %";
        });
    }

    setInterval(updateSensors, 1000);

    </script>

    </body>
    </html>
  )rawliteral";

void handleRoot() {
  server.send_P(200, "text/html", webpage);
}

void handleControl() {

  if (!server.hasArg("device") || !server.hasArg("state") ) {
    server.send(400, "text/plain", "Missing parameters");
    return;
  }

  if ((rainRule == "none") && (nightRule == "none") && (touchRule == "none") && (motionRule == "none")) {
      server.send(200, "text/plain", "Toggle Allowed");
  }
  else {
    server.send(200, "text/plain", "Toggle Not Allowed");
    return;
  }

  String device = server.arg("device");
  String state  = server.arg("state");

  bool turnOn = (state == "on");

  if (device == "led") {
    digitalWrite(LED_LIGHT_PIN, turnOn);
    ledState = turnOn;
  }
  else if (device == "buzzer") {
    digitalWrite(BUZZER_PIN, turnOn);
    buzzerState = turnOn;
  }
  else if (device == "relay") {
    digitalWrite(RELAY_PIN, turnOn);
    relayState = turnOn;
  }
  else {
    server.send(400, "text/plain", "Invalid device");
    return;
  }

  server.send(200, "text/plain", "OK");
}

void handleSetRule() {

  if (!server.hasArg("trigger") || !server.hasArg("action")) {
    server.send(400, "text/plain", "Missing parameters");
    return;
  }
  
  String trigger = server.arg("trigger");
  String action  = server.arg("action");

  if (action != "none") {

    // Check if action already used by another trigger
    if ((rainRule == action && trigger != "rain") ||
        (nightRule == action && trigger != "night") ||
        (touchRule == action && trigger != "touch") ||
        (motionRule == action && trigger != "motion")) {

      server.send(400, "text/plain", "Action already assigned");
      return;
    }
  }

  if (trigger == "rain") {
    rainRule = action;
  }
  else if (trigger == "night") {
    nightRule = action;
  }
  else if (trigger == "touch") {
    touchRule = action;
  }
  else if (trigger == "motion") {
    motionRule = action;
  }
  else {
    server.send(400, "text/plain", "Invalid trigger");
    return;
  }

  server.send(200, "text/plain", "Rule Updated");
}

void handleSetPattern() {

  if (!server.hasArg("mode")) {
    server.send(400, "text/plain", "Missing mode");
    return;
  }

  stripMode = server.arg("mode");

    if (stripMode == "off") {
    led_strip_pattern = -1;
  } else if (stripMode == "CW") {
    led_strip_pattern = 0;
  } else if (stripMode == "RGB_FADE") {
    led_strip_pattern = 1;
  } else if (stripMode == "PC1") {
    led_strip_pattern = 2;
  } else if (stripMode == "PC2") {
    led_strip_pattern = 3;
  } else if (stripMode == "BB") {
    led_strip_pattern = 4;
  } else if (stripMode == "RL") {
    led_strip_pattern = 5;
  } else if (stripMode == "RB") {
    led_strip_pattern = 6;
  }

  server.send(200, "text/plain", "Pattern Updated");
}

void handleSensorData() {
  String json = "{";
  json += "\"rain\":\"" + String(rainDetected ? "Yes" : "No") + "\",";
  json += "\"night\":\"" + String(nightDetected ? "Yes" : "No") + "\",";
  json += "\"gas\":" + String(gas_analog_value) + ",";
  json += "\"motion\":\"" + String(motionDetected ? "Yes" : "No") + "\",";
  json += "\"temp\":" + String(dht11_temperature) + ",";
  json += "\"humid\":" + String(dht11_humidity);
  json += "}";

  server.send(200, "application/json", json);
}

//ALL ACTIVITIES FUNCTION START -----------------------------------------------------------------

//Led Strip Pattern/Animation Function Start -------------------------
void setAll(uint8_t r, uint8_t g, uint8_t b) {
  strip.fill(strip.Color(r, g, b));
  strip.show();
}

long firstPixelHue = 0;
void led_strip_rainbow(){
  strip.rainbow(firstPixelHue, 1, 255, 255, true);
  strip.show();

  firstPixelHue = (firstPixelHue > 5*65536) ? 0 : firstPixelHue+256;

  Serial.println("LED_STRIP_RAINBOW_CALLED");
}

int CW_last_pixel = 0;
int CW_last_color = 0;
uint32_t color;
void colorWipe() {

  switch (CW_last_color) {
    case 0: 
    color = strip.Color(255,   0,   0);
    break;

    case 1: 
    color = strip.Color(0,   255,   0);
    break;

    case 2: 
    color = strip.Color(0,   0,   255);
    break;
  }
  strip.setPixelColor(CW_last_pixel, color);         //  Set pixel's color (in RAM)
  strip.show();                          //  Update strip to match

  CW_last_pixel += 1;
  if (CW_last_pixel >= 8){
    CW_last_pixel = 0;
    CW_last_color = (CW_last_color >= 2) ? 0 : CW_last_color +1;
    delay(100);
    setAll(0,0,0);
  }
}

int RGBF_color = 0;
int RGBF_last_brightness = 0;
int RGBF_fadeinout = 0;
void rgbFade(){

  if (!RGBF_fadeinout){

    //FADE_IN -> 0; FADE_OUT -> 1;
  switch (RGBF_color) {
    case 0: 
    setAll(RGBF_last_brightness, 0, 0);
    break;

    case 1: 
    setAll(0, RGBF_last_brightness, 0);
    break;

    case 2: 
    setAll(0, 0, RGBF_last_brightness);
    break;
  }
  RGBF_last_brightness = (RGBF_last_brightness >= 200) ? 0 : RGBF_last_brightness +30;

    if (RGBF_last_brightness  == 0){
      RGBF_fadeinout = 1;
      RGBF_last_brightness = 250;
    }
  }

  else if (RGBF_fadeinout){

    //FADE_OUT
    switch (RGBF_color) {
      case 0: 
      setAll(RGBF_last_brightness, 0, 0);
      break;

      case 1: 
      setAll(0, RGBF_last_brightness, 0);
      break;

      case 2: 
      setAll(0, 0, RGBF_last_brightness);
      break;
    }

  RGBF_last_brightness = (RGBF_last_brightness <= 40) ? 0 : RGBF_last_brightness -30;

  if (RGBF_last_brightness  == 0){
      RGBF_fadeinout = 0;
      setAll(0, 0, 0);
      RGBF_color = (RGBF_color >= 2) ? 0 : RGBF_color +1;
    }
  }

}

int POLICE_CH1_last_color = 0;
void police_chase1(){  

  switch (POLICE_CH1_last_color) {
    case 0:
    setAll(0,0,0);
    strip.fill(strip.Color(0, 0, 255), 0, 4);
    strip.show();
    break;

    case 1: 
    setAll(0,0,0);
    strip.fill(strip.Color(255, 0, 0), 4, 4);
    strip.show();
    break;
  }

  POLICE_CH1_last_color = (POLICE_CH1_last_color == 1) ? 0 : 1;
}

int POLICE_CH2_last_color = 0;
void police_chase2(){  

  switch (POLICE_CH2_last_color) {
    case 0:
    setAll(0,0,0);
    strip.fill(strip.Color(255, 0, 0), 0, 2);
    strip.fill(strip.Color(0, 0, 255), 6, 2);
    strip.show();
    break;

    case 1: 
    setAll(0,0,0);
    strip.fill(strip.Color(255, 255, 255), 2, 4);    
    strip.show();
    break;
  }  

  POLICE_CH2_last_color = (POLICE_CH2_last_color == 1) ? 0 : 1;
}

int BB_last_pixel = 0;
int BB_direction = 0;
void bounce_ball(){

  setAll(0,0,0);
  strip.setPixelColor(BB_last_pixel, strip.Color(0, 255, 255)); 
  strip.show(); 

  if (!BB_direction){
    BB_last_pixel = (BB_last_pixel >= 7) ? 0 : BB_last_pixel +1;
  }
  else{
    BB_last_pixel = (BB_last_pixel <= 0) ? 7 : BB_last_pixel -1;
  }

  if (!BB_direction && (BB_last_pixel == 0)){
    BB_direction = 1;
    BB_last_pixel = 7;
  }
  else if (BB_direction && (BB_last_pixel == 7)){
    BB_direction = 0;
    BB_last_pixel = 0;
  }

  
}

int RL_case_running = 0;
void running_light(){

  switch (RL_case_running) {
    case 0:
      setAll(0,0,0);
      strip.fill(strip.Color(187,122,122), 0, 2);
      strip.fill(strip.Color(187,122,122), 4, 2);
      strip.show();
    break;

    case 1:
      setAll(0,0,0);
      strip.fill(strip.Color(187,122,122), 1, 2);
      strip.fill(strip.Color(187,122,122), 5, 2);
      strip.show();
    break;

    case 2:
      setAll(0,0,0);
      strip.fill(strip.Color(187,122,122), 2, 2);
      strip.fill(strip.Color(187,122,122), 6, 2);
      strip.show();
    break;

    case 3:
      setAll(0,0,0);
      strip.fill(strip.Color(187,122,122), 0, 1);
      strip.fill(strip.Color(187,122,122), 3, 2);
      strip.fill(strip.Color(187,122,122), 7, 1);
      strip.show();
    break;
  }

  RL_case_running = (RL_case_running >= 3) ? 0 : RL_case_running +1;

}

unsigned long last_called_led_pattern = 0;
void led_pattern(){
  
  switch (led_strip_pattern) {
    case 0:
    if (millis() - last_called_led_pattern >= 150){
        colorWipe();
        last_called_led_pattern = millis();
      }
    break;

    case 1:
      if (millis() - last_called_led_pattern >= 150){
        rgbFade();
        last_called_led_pattern = millis();
      }
    break;

    case 2:
      if (millis() - last_called_led_pattern >= 150){
        police_chase1();
        last_called_led_pattern = millis();
      }
    break;

    case 3:
      if (millis() - last_called_led_pattern >= 150){
        police_chase2();
        last_called_led_pattern = millis();
      }
    break;

    case 4:
      if (millis() - last_called_led_pattern >= 150){
        bounce_ball();
        last_called_led_pattern = millis();
      }
    break;

    case 5:
      if (millis() - last_called_led_pattern >= 150){
        running_light();
        last_called_led_pattern = millis();
      }
    break;

    case 6:
      if (millis() - last_called_led_pattern >= 5){
        led_strip_rainbow();
        last_called_led_pattern = millis();
      }
    break;

    case -1:
      setAll(0,0,0);
    break;
  }
}

void led_pattern_name(){
  switch (led_strip_pattern) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("COLOR WIPE");
    break;

    case 1:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("RGB FADE");
    break;

    case 2:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("POLICE CHASE 1");
    break;

    case 3:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("POLICE CHASE 2");
    break;

    case 4:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("BOUNCE BALL");
    break;

    case 5:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("RUNNING LIGHT");
    break;

    case 6:
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("RAINBOW LIGHT");
    break;
  }
}

//Led Strip Pattern/Animation Function End ---------------------------

void gas_activity(){
  gas_analog_value = analogRead(GAS_PIN);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Gas Value: ");
  lcd.print(gas_analog_value, 2);

  Serial.println("GAS_CALLED");
  Serial.print(F("Gas Value: "));
  Serial.println(gas_analog_value);
}

void touch_relay(){

  if (digitalRead(TOUCH_PIN) == HIGH && digitalRead(RELAY_PIN) == HIGH){
    digitalWrite(RELAY_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("RELAY: OFF");
  }
  else if (digitalRead(TOUCH_PIN) == HIGH && digitalRead(RELAY_PIN) == LOW){
    digitalWrite(RELAY_PIN, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("RELAY: ON");
  }
  Serial.println("TOUCH_RELAY_CALLED");
}

void auto_night_light(){
  if (digitalRead(LDR_PIN) == HIGH){
    digitalWrite(LED_LIGHT_PIN, LOW);
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("LIGHT: ON");
  }
  else {
    digitalWrite(LED_LIGHT_PIN, HIGH);
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("LIGHT: OFF");
  }
  Serial.println("AUTO_NIGHT_LIGHT_CALLED");
}

int prev_beep_state = 0;
void buzzerbeep(){
  //0 -> BUZZ_OFF, 1 -> BUZZ_ON
  if (!prev_beep_state){
    digitalWrite(BUZZER_PIN, HIGH);
    prev_beep_state = 1;
  }
  else if (prev_beep_state) {
    digitalWrite(BUZZER_PIN, LOW);
    prev_beep_state = 0;
  }
}

void rain_detection(){
  if (digitalRead(RAIN_PIN) == LOW){
    buzzerbeep();
    lcd.setCursor(5, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("RAIN: YES");
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
    lcd.setCursor(5, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("RAIN: NO");
  }
  Serial.println("RAIN_DETECTION_CALLED");
}

void temp_humid(){
  dht11_humidity = dht.readHumidity();
  dht11_temperature = dht.readTemperature();
  if (!isnan(dht11_humidity) && !isnan(dht11_temperature)){
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("H:");
    lcd.print(dht11_humidity, 1);
    lcd.print("%");
    lcd.setCursor(9, 1);
    lcd.print("T:");
    lcd.print(dht11_temperature, 1);
    lcd.print("C");
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("DHT11 SENSOR ERR");
  }
}

int prev_relay_state = 0;
void relay_on_off(){
  //0 -> RELAY_OFF, 1 -> RELAY_ON
  if (!prev_relay_state){
    digitalWrite(RELAY_PIN, HIGH);
    prev_relay_state = 1;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("RELAY: ON");
  }
  else if (prev_relay_state) {
    digitalWrite(RELAY_PIN, LOW);
    prev_relay_state = 0;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("RELAY: OFF");
  }
}

int prev_light_state = 0;
void light_on_off(){
  //0 -> LIGHT_OFF, 1 -> LIGHT_ON
  if (!prev_light_state){
    digitalWrite(LED_LIGHT_PIN, LOW);
    prev_light_state = 1;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("LIGHT: ON");
  }
  else if (prev_light_state) {
    digitalWrite(LED_LIGHT_PIN, HIGH);
    prev_light_state = 0;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("LIGHT: OFF");
  }
}

void buzzer_on_off(){
  //0 -> BUZZER_OFF, 1 -> BUZZER_ON
  if (currentState !=  BUZZER_BEEP_ON){
    currentState = BUZZER_BEEP_ON;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("BUZZER: ON");
  }
  else if (currentState ==  BUZZER_BEEP_ON) {
    currentState = STANDBY;
    digitalWrite(BUZZER_PIN, LOW);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("BUZZER: OFF");
  }
}

void motion_light(){
  if (digitalRead(PIR_PIN) == HIGH){
    digitalWrite(LED_LIGHT_PIN, LOW);
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("LIGHT: ON");
  }
  else {
    digitalWrite(LED_LIGHT_PIN, HIGH);
    
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("LIGHT: OFF");
  }
}

void motion_alarm(){
  if (digitalRead(PIR_PIN) == HIGH){
    buzzerbeep();
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("ALARM: ON");
  }
  else {
    digitalWrite(BUZZER_PIN, LOW);
    lcd.setCursor(6, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("ALARM: OFF");
  }
}

void all_off(){
  digitalWrite(LED_LIGHT_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  setAll(0,0,0);
}

unsigned long last_read_gas_DHT_value = 0;
void all_activity(){  

  if (millis() - last_read_gas_DHT_value >= 800){
    gas_analog_value = analogRead(GAS_PIN);
    dht11_humidity = dht.readHumidity();
    dht11_temperature = dht.readTemperature();
    lcd.clear();
    last_read_gas_DHT_value = millis();
  }

  //HUMIDITY, TEMPERATURE, GAS VALUE
  if (!isnan(dht11_humidity) && !isnan(dht11_temperature)){
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(dht11_temperature, 0);
    lcd.print(" H:");
    lcd.print(dht11_humidity, 0);
    lcd.print(" G:");
    lcd.print(gas_analog_value, 0);
  }
  else {
    lcd.setCursor(0, 0);
    lcd.print("DHT11 ERR");
    lcd.print(" G:");
    lcd.print(gas_analog_value, 0);
  }

  //MOTION DETECTION
  if (digitalRead(PIR_PIN) == HIGH){
    if (motionRule == "led") {
      digitalWrite(LED_LIGHT_PIN, LOW);
      ledState = true;
    } 
    else if (motionRule == "relay") {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
    } 
    else if (motionRule == "buzzer") {
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
    } 
    else if (motionRule == "led_strip") {
      setAll(255,255,255);
    }

    motionDetected = true;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("MOTION: YES");
  }
  else{
    if (motionRule == "led") {
      digitalWrite(LED_LIGHT_PIN, HIGH);
      ledState = false;
    } 
    else if (motionRule == "relay") {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
    } 
    else if (motionRule == "buzzer") {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
    } 
    motionDetected = false;
  }

  //RAIN DETECTION
  if (digitalRead(RAIN_PIN) == LOW){
    if (rainRule == "led") {
      digitalWrite(LED_LIGHT_PIN, LOW);
      ledState = true;
    } 
    else if (rainRule == "relay") {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
    } 
    else if (rainRule == "buzzer") {
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
    } 
    else if (rainRule == "led_strip") {
      setAll(255,255,255);
    }

    rainDetected = true;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("RAIN: YES");
  }
  else {
    if (rainRule == "led") {
      digitalWrite(LED_LIGHT_PIN, HIGH);
      ledState = false;
    } 
    else if (rainRule == "relay") {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
    } 
    else if (rainRule == "buzzer") {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
    }

    rainDetected = false;
  }

  //NIGHT LIGHT
  if (digitalRead(LDR_PIN) == HIGH){
    if (nightRule == "led") {
      digitalWrite(LED_LIGHT_PIN, LOW);
      ledState = true;
    } 
    else if (nightRule == "relay") {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
    } 
    else if (nightRule == "buzzer") {
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
    } 
    else if (nightRule == "led_strip") {
      setAll(255,255,255);
    }

    nightDetected = true;
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print("NIGHT: YES");
  }
  else{
    if (nightRule == "led") {
      digitalWrite(LED_LIGHT_PIN, HIGH);
      ledState = false;
    } 
    else if (nightRule == "relay") {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
    } 
    else if (nightRule == "buzzer") {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
    } 

    nightDetected = false;
  }

  //TOUCH SWITCH
  if (digitalRead(TOUCH_PIN) == HIGH){
    led_strip_pattern = (led_strip_pattern <= -1) ? 6 : led_strip_pattern -1;
    led_pattern_name();
    delay(500);
    if (touchRule == "led") {
      digitalWrite(LED_LIGHT_PIN, LOW);
      ledState = true;
    } 
    else if (touchRule == "relay") {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
    } 
    else if (touchRule == "buzzer") {
      digitalWrite(BUZZER_PIN, HIGH);
      buzzerState = true;
    } 
    else if (touchRule == "led_strip") {
      setAll(255,255,255);
    }
  }
  else {
    if (touchRule == "led") {
      digitalWrite(LED_LIGHT_PIN, HIGH);
      ledState = false;
    } 
    else if (touchRule == "relay") {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
    } 
    else if (touchRule == "buzzer") {
      digitalWrite(BUZZER_PIN, LOW);
      buzzerState = false;
    } 
    }
  led_pattern();
}

//ALL ACTIVITIES FUNCTION END -----------------------------------------------------------------

void setup() {
  dht.begin();
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  strip.begin();
  strip.setBrightness(10);
  strip.clear();
  strip.show();  

  lcd.init(); // initialize LCD
  lcd.backlight(); // turn on LCD backlight                      

  lcd.createChar(0, ch0);
  lcd.createChar(1, ch1);
  lcd.createChar(2, ch2);
  lcd.createChar(3, ch3);
  lcd.createChar(4, ch4);
  lcd.createChar(5, ch5);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.write(byte(1));
  lcd.write(byte(2));
  lcd.print("  IoT House");

  // Bottom row
  lcd.setCursor(0, 1);
  lcd.write(byte(3));
  lcd.write(byte(4));
  lcd.write(byte(5));
  lcd.print("  WELCOME..");
  delay(3000);

  server.on("/", handleRoot);
  server.on("/control", handleControl);
  server.on("/setPattern", handleSetPattern);
  server.on("/setRule", handleSetRule);
  server.on("/sensorData", handleSensorData);

  pinMode(TOUCH_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_LIGHT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_LIGHT_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("   Wifi Name:   ");
  lcd.setCursor(0,1);
  lcd.print(" REL_IoT_House  ");
  delay(4000);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" Wifi Password: ");
  lcd.setCursor(0,1);
  lcd.print("   012345678    ");
  delay(4000);

  start_duration = millis();
}

void loop() {

  if (IrReceiver.decode()) {
    IR_Hex_Code = IrReceiver.decodedIRData.decodedRawData;
    Serial.println(IR_Hex_Code, HEX);
    IrReceiver.resume();
    auto_switch = true;
  }

  if (currentState == ALL_ACTIVITY_WIFI && !wifiRunning) {
    startWiFi();
  }
  if (currentState != ALL_ACTIVITY_WIFI && wifiRunning) {
    stopWiFi();
  }
  if (currentState == ALL_ACTIVITY_WIFI && wifiRunning) {
    server.handleClient();
  }

  //GAS_LEAKAGE
  if (IR_Hex_Code == 0xBA45FF00) {
    currentState = GAS_LEAKAGE;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" GAS DETECTION  ");
    all_off();
    IR_Hex_Code = 0;
  }

  //AUTO_NIGHT_LIGHT
  else if (IR_Hex_Code == 0xB946FF00) {
    currentState = AUTO_NIGHT_LIGHT;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("AUTO NIGHT LIGHT");
    all_off();
    IR_Hex_Code = 0;
  }

  //TOUCH_RELAY
  else if (IR_Hex_Code == 0xB847FF00) {
    currentState = TOUCH_RELAY;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  TOUCH RELAY   ");
    all_off();
    IR_Hex_Code = 0;
  }

  //RAIN_DETECTION
  else if (IR_Hex_Code == 0xBB44FF00) {
    currentState = RAIN_DETECTION;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" RAIN DETECTION ");
    all_off();
    IR_Hex_Code = 0;
  }

  //LED_STRIP
  else if (IR_Hex_Code == 0xF807FF00) {
    currentState = LED_STRIP;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" LED STRIP MODE ");
    led_pattern_name();
    all_off();
    IR_Hex_Code = 0;
  }

  //TEMP_AND_HUMID
  else if (IR_Hex_Code == 0xBC43FF00) {
    currentState = TEMP_AND_HUMID;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TEMP. AND HUMID.");
    all_off();
    IR_Hex_Code = 0;
  }
  
  //REMOTE_RELAY
  else if (IR_Hex_Code == 0xF609FF00) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  REMOTE RELAY  ");
    relay_on_off();
    IR_Hex_Code = 0;
    currentState = STANDBY;
  }

  //REMOTE_BUZZER
  else if (IR_Hex_Code == 0xF20DFF00) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" REMOTE BUZZER ");
    buzzer_on_off();
    IR_Hex_Code = 0;
  }

  //REMOTE_LIGHT
  else if (IR_Hex_Code == 0xE916FF00) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  REMOTE LIGHT  ");
    light_on_off();
    IR_Hex_Code = 0;
    currentState = STANDBY;
  }

  //ESP_RESTART
  else if (IR_Hex_Code == 0xE31CFF00) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP Restart... 3");
    delay(950);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP Restart... 2");
    delay(950);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP Restart... 1");
    delay(950);

    ESP.restart();
  }

  //LED_STRIP_BRIGHTNESS_UP
  else if (IR_Hex_Code == 0xE718FF00) {
    if (current_led_strip_brightness <= 230){
        current_led_strip_brightness += 10;
    }
    strip.setBrightness(current_led_strip_brightness);
    IR_Hex_Code = 0;
  }

  //LED_STRIP_BRIGHTNESS_DOWN
  else if (IR_Hex_Code == 0xAD52FF00) {
    if (current_led_strip_brightness >= 11){
        current_led_strip_brightness -= 10;
    }
    strip.setBrightness(current_led_strip_brightness);
    IR_Hex_Code = 0;
  }

  //MOTION_LIGHT
  else if (IR_Hex_Code == 0xEA15FF00) {
    currentState = MOTION_LIGHT;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  MOTION LIGHT  ");
    all_off();
    IR_Hex_Code = 0;
  }

  //MOTION_DETECTION_ALARM
  else if (IR_Hex_Code == 0xBF40FF00) {
    currentState = MOTION_DETECTION_ALARM;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  MOTION ALARM  ");
    all_off();
    IR_Hex_Code = 0;
  }
 
  //LED_STRIP_PATTERN_DOWN
  else if (IR_Hex_Code == 0xA55AFF00 && currentState == LED_STRIP) {
    led_strip_pattern = (led_strip_pattern <= 0) ? 6 : led_strip_pattern -1;
    led_pattern_name();
    setAll(0,0,0);
    IR_Hex_Code = 0;
  }

  //LED_STRIP_PATTERN_UP
  else if (IR_Hex_Code == 0xF708FF00 && currentState == LED_STRIP) {
    led_strip_pattern = (led_strip_pattern >= 6) ? 0 : led_strip_pattern +1;
    led_pattern_name();
    setAll(0,0,0);
    IR_Hex_Code = 0;
  }

  //ALL_ACTIVITY_WIFI
  else if (IR_Hex_Code == 0XE619FF00) {
    currentState = ALL_ACTIVITY_WIFI;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   Wifi Name:   ");
    lcd.setCursor(0,1);
    lcd.print(" REL_IoT_House  ");
    delay(4000);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Wifi Password: ");
    lcd.setCursor(0,1);
    lcd.print("   012345678    ");
    delay(4000);

    all_off();
    IR_Hex_Code = 0;
  }

  //Auto Switch after 15sec of no input
  if (millis() - start_duration >= 9000 && !auto_switch){
    currentState = ALL_ACTIVITY_WIFI;
    auto_switch = true;
  }

  switch (currentState) {

    case GAS_LEAKAGE:
      if (millis() - last_called >= 1500){
        gas_activity();
        last_called = millis();
      }
    break;

    case AUTO_NIGHT_LIGHT:
      if (millis() - last_called >= 450){
        auto_night_light();
        last_called = millis();
      }
    break;

    case TOUCH_RELAY:
      if (millis() - last_called >= 450){
        touch_relay();
        last_called = millis();
      }
    break;

    case RAIN_DETECTION:
      if (millis() - last_called >= 450){
        rain_detection();
        last_called = millis();
      }
    break;

    case TEMP_AND_HUMID:
      if (millis() - last_called >= 450){
        temp_humid();
        last_called = millis();
      }
    break;

    case LED_STRIP:
      if (millis() - last_called >= 2){
        led_pattern();
        last_called = millis();
      }
    break;

    case BUZZER_BEEP_ON:
      if (millis() - last_called >= 150){
        buzzerbeep();
        last_called = millis();
      }
    break;

    case MOTION_LIGHT:
      if (millis() - last_called >= 100){
        motion_light();
        last_called = millis();
      }
    break;

    case MOTION_DETECTION_ALARM:
      if (millis() - last_called >= 450){
        motion_alarm();
        last_called = millis();
      }
    break;

    case ALL_ACTIVITY_WIFI:
      if (millis() - last_called >= 50){
        all_activity();
        last_called = millis();
      }
    break;
  }
}