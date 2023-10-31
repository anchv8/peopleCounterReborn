#ifdef ESPDebug
void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
}
 
void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
}
 
void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}
 
void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}
#endif

bool isVersionGreater(const String& newVersion, const String& currentVersion) {
    int newMajor, newMinor;
    int currMajor, currMinor;
  
    sscanf(newVersion.c_str(), "%d.%d", &newMajor, &newMinor);
    sscanf(currentVersion.c_str(), "%d.%d", &currMajor, &currMinor);
  
    if (newMajor > currMajor) return true;
    if (newMajor < currMajor) return false;

    if (newMinor > currMinor) return true;
    if (newMinor < currMinor) return false;
    
    return false;
}

void checkForUpdates() {
  HTTPClient http;
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  http.begin(client, serverURL, serverPort, versionCheckUrl);

  int httpCode = http.GET();
  DEBUG(httpCode);

  if (httpCode == HTTP_CODE_OK) {
    #ifdef ESPDebug
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);
    #endif
    String newFirmwareVersion = http.getString();
    newFirmwareVersion.replace("\"", "");
    DEBUG(newFirmwareVersion);
    // Обработка ответа от сервера (например, сравнение версий)
    bool updFlag = isVersionGreater(newFirmwareVersion, FIRMWARE_VERSION);
    DEBUG(updFlag);
    if (updFlag) {
    DEBUG("New firmware version available, starting update...");
    
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, serverURL, serverPort, fileUrl);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        ESP.restart();
        break;
        }
    }
    http.end();
  } else {
    DEBUG("Failed to check for firmware updates");
    http.end();
  }
}
