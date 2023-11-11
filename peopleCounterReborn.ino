#define VL53LOX_ShutdownPin 12   // Пин вкл/выкл лидара
#define VL53LOX_InterruptPin 14  // Пин прерывания
#define ESPDebug                 // Включаем или выключаем дебаг режим
#define THRESHOLD_SPEED 35

#ifdef ESPDebug
//#define DEBUG(x) WebSerial.println(x)
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <Adafruit_VL53L0X.h>
#include "GyverTimer.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

const String FIRMWARE_VERSION = "1.3";

const long delayTime = 1000; // Задержка 5 секунд
long lastDetectionTime = 0;
bool objectDetected = false;

int buttonState = HIGH;
unsigned long buttonPressTime = 0;         // Время, когда кнопка была впервые нажата
int eventCounter = 0;                      // Счетчик событий
bool eventFlag = false;                    // Флаг события
const char* serverURL = "192.168.100.15";  // Адрес сервера
const int serverPort = 7000;               // Порт сервера
static const char* serverRegisterPath = "/api/register_sensor/";
static const char* serverPath = "/api/update_visitor_count/";
static const char* versionCheckUrl = "/update_firmware/";
static const char* fileUrl = "/download_firmware/firmware.bin";
char* apiKey;
char* sensName;
char timezone[5] = "+00";
char registrationData[64];
char jsonPayload[256];
StaticJsonDocument<512> jsonConfig;
struct CustomParameters {
  String apiKey;
  String sensName;
  const char* timezone;
};
CustomParameters param;  // глобальная переменная

const char timezone_option_list[] PROGMEM = R"(
    <label for="timezone">Timezone</label>
    <select name="timezone">
      <option value="-12">UTC-12:00 - Baker Island</option>
      <option value="-11">UTC-11:00 - American Samoa</option>
      <option value="-10">UTC-10:00 - Hawaii</option>
      <option value="-9">UTC-09:00 - Alaska</option>
      <option value="-8">UTC-08:00 - Pacific Time (US & Canada)</option>
      <option value="-7">UTC-07:00 - Mountain Time (US & Canada)</option>
      <option value="-6">UTC-06:00 - Central Time (US & Canada)</option>
      <option value="-5">UTC-05:00 - Eastern Time (US & Canada)</option>
      <option value="-4.5">UTC-04:30 - Venezuela</option>
      <option value="-4">UTC-04:00 - Atlantic Time (Canada)</option>
      <option value="-3">UTC-03:00 - Buenos Aires, Brasilia</option>
      <option value="0">UTC±00:00 - London, Lisbon</option>
      <option value="1">UTC+01:00 - Paris, Berlin</option>
      <option value="2">UTC+02:00 - Athens, Helsinki</option>
      <option value="3">UTC+03:00 - Moscow, Riyadh</option>
      <option value="3.5">UTC+03:30 - Tehran</option>
      <option value="4">UTC+04:00 - Dubai, Baku</option>
      <option value="4.5">UTC+04:30 - Kabul</option>
      <option value="5">UTC+05:00 - Islamabad, Karachi</option>
      <option value="5.5">UTC+05:30 - New Delhi, Colombo</option>
      <option value="5.75">UTC+05:45 - Kathmandu</option>
      <option value="6">UTC+06:00 - Almaty, Dhaka</option>
      <option value="6.5">UTC+06:30 - Yangon</option>
      <option value="7">UTC+07:00 - Bangkok, Jakarta</option>
      <option value="8">UTC+08:00 - Beijing, Singapore</option>
      <option value="9">UTC+09:00 - Tokyo, Seoul</option>
      <option value="9.5">UTC+09:30 - Adelaide, Darwin</option>
      <option value="10">UTC+10:00 - Sydney, Melbourne</option>
      <option value="11">UTC+11:00 - Solomon Islands</option>
      <option value="12">UTC+12:00 - Fiji, Marshall Islands</option>
    </select>
)";

AsyncWebServer server(80);
DNSServer dns;

WiFiUDP ntpUDP;  // Экземпляр класса для получения времени с интернетов
NTPClient timeClient(ntpUDP, "pool.ntp.org", 21600);

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
Adafruit_VL53L0X::VL53L0X_Sense_config_t long_range = Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE;
TwoWire* i2c = &Wire;

AsyncWiFiManager wifiManager(&server, &dns);
AsyncWiFiManagerParameter customApiKey("apiKey", "API Ключ", apiKey, 16);
AsyncWiFiManagerParameter customSensName("sensName", "Имя датчика", sensName, 32);
AsyncWiFiManagerParameter customTimezone("timezone", "Timezone", timezone, 5, timezone_option_list);

WiFiClient client;


//flag for saving data
bool shouldSaveConfig = false;

#ifndef ESPDebug
GTimer writeTimer(MS, 600000), sendTimer(MS, 600500);  // Таймер на 10(600000) минут
#endif

#ifdef ESPDebug
GTimer json_timer(MS, 4000), writeTimer(MS, 10000), sendTimer(MS, 10200);
#endif

void handle() {
  eventCounter++;
  DEBUG("Detected!");
  lox.clearInterruptMask(false);
}

void setup() {
#ifdef ESPDebug
  Serial.begin(115200);
  WebSerial.begin(&server);
#endif
  // Инициализация WiFiManager
  wifiSetup();
  param = loadCustomParameters();
#ifdef ESPDebug
  DEBUG("Firmware Version");
  DEBUG(FIRMWARE_VERSION);
  delay(5000);
#endif
  server.begin();
  registerDevice();
  DEBUG("Firmware Version");
  DEBUG(FIRMWARE_VERSION);
  int timezoneOffsetSeconds = atoi(param.timezone) * 3600;  // Преобразуйте часы в секунды
  timeClient.setTimeOffset(timezoneOffsetSeconds);
  timeClient.begin();
  timeClient.update();
  checkForUpdates();
  sensInit();  // Инициализируем лидар
  eventCounter = 0;
}

void loop() {
  //button();
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();

  if (sendTimer.isReady()) sendData();
  if (digitalRead(VL53LOX_InterruptPin) == LOW) {
    VL53L0X_RangingMeasurementData_t measure;
    lox.getRangingMeasurement(&measure, false);

    if (measure.RangeStatus != 4) {
      if (!objectDetected) {
        if ((millis() - lastDetectionTime) > delayTime) {
          objectDetected = true;
          lastDetectionTime = millis();
          handle();
        }
      }
    } else {
    objectDetected = false;
    }
  }

  if (currentHour == 23 && currentMinute == 0) {
    DEBUG("waiting for update check");
    checkForUpdates();
    delay(61000);  // Ждите минуту, чтобы функция не выполнялась несколько раз в 23:00
  }
}
