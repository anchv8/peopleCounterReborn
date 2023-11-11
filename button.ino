// void button() {
//   buttonState = digitalRead(buttonPin);
//   DEBUG(buttonState);

//   if (buttonState == LOW) {                                     // Если кнопка нажата
//     if (buttonPressTime == 0) {                                 // Если таймер еще не запущен
//       buttonPressTime = millis();                               // Запускаем таймер
//     } else if ((millis() - buttonPressTime) > longPressTime) {  // Если кнопка удерживается в течение 5 секунд
//       wifiManager.resetSettings();                              // Сброс настроек WiFiManager
//       ESP.restart();                                            // Перезагрузка устройства
//       buttonPressTime = 0;                                      // Сбрасываем таймер
//     }
//   } else {
//     buttonPressTime = 0;  // Сбрасываем таймер, если кнопка отпущена
//   }
// }

