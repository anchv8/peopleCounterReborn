void saveConfigCallback() {
  DEBUG("Configuration parameters saved");
  saveCustomParameters();
}

void wifiSetup() {
  // Чтение конфигурационных параметров из файла
  if (!SPIFFS.begin()) {
    DEBUG("Failed to mount FS for config");
    ESP.restart();
  }

  wifiManager.addParameter(&customApiKey);
  wifiManager.addParameter(&customSensName);
  wifiManager.addParameter(&customTimezone);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect("WiFi-Counter");

  DEBUG("Connected to Wi-Fi");
}
