void saveCustomParameters() {
  JsonObject root = jsonConfig.to<JsonObject>();

  // Считываем значения параметров из WiFiManager
  root["apiKey"] = customApiKey.getValue();
  root["sensName"] = customSensName.getValue();
  root["timezone"] = customTimezone.getValue();

  // Сохраняем JSON в файл
  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    serializeJson(root, configFile);
    configFile.close();
  }
}

CustomParameters loadCustomParameters() {
  CustomParameters localParams;
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return localParams;
    }

    size_t size = configFile.size();
    if (size == 0) {
        Serial.println("Config file is empty");
        configFile.close();
        return localParams;
    }

    std::unique_ptr<char[]> buf(new char[size + 1]);
    configFile.readBytes(buf.get(), size);
    configFile.close();
    buf[size] = '\0';

    DynamicJsonDocument jsonConfig(512);
    DeserializationError error = deserializeJson(jsonConfig, buf.get());
    if (error) {
        Serial.println("Failed to parse config file");
        return localParams;
    }

    localParams.apiKey = jsonConfig["apiKey"].as<String>();
    localParams.sensName = jsonConfig["sensName"].as<String>();

    return localParams;
}

