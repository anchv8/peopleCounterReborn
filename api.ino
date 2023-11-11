void registerDevice() {
  // Подготавливаем данные для регистрации на сервере (например, в виде JSON)
  snprintf(registrationData, sizeof(registrationData), "{\"api_key\":\"%s\",\"sensor_name\":\"%s\"}", param.apiKey, param.sensName);
  DEBUG(registrationData);

  // Отправляем данные на сервер для регистрации
  HTTPClient http;
  http.begin(client, serverURL, serverPort, serverRegisterPath);
  http.addHeader("Content-Type", "application/json");

  // Отправляем данные на сервер для регистрации
  int httpResponseCode = http.POST(registrationData);
  http.end();

  if (httpResponseCode > 0) {
    DEBUG("Device registration successful. HTTP Response code: ");
    DEBUG(httpResponseCode);
  } else {
    DEBUG("Device registration failed. Error code: ");
    DEBUG(httpResponseCode);
  }
}


void sendData() {
  // Подготавливаем данные для отправки на сервер
  char sendData[100];
  snprintf(sendData, sizeof(sendData), "{\"api_key\":\"%s\",\"visitor_count\":\"%d\",\"sensor_name\":\"%s\"}", param.apiKey, eventCounter, param.sensName);
  DEBUG(sendData);

  // Настройка HTTP POST запроса
  HTTPClient http;
  http.begin(client, serverURL, serverPort, serverPath);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(sendData);
  http.end();

  // Проверяем код ответа
  if (httpResponseCode == 200) {
    DEBUG("Data sent successfully. HTTP Response code: ");
    eventCounter = 0;
  } else {
    DEBUG("Data sending failed. Error code: ");
  }
  DEBUG(httpResponseCode);
}
