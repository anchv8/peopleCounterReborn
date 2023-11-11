void sensInit() {
  pinMode(VL53LOX_ShutdownPin, INPUT_PULLUP);
  pinMode(VL53LOX_InterruptPin, INPUT_PULLUP);

  while (!lox.begin(0x29, false, i2c, long_range)) {
    DEBUG(F("Failed to boot VL53L0X"));
    DEBUG("Adafruit VL53L0X XShut set Low to Force HW Reset");
    digitalWrite(VL53LOX_ShutdownPin, LOW);
    delay(100);
    digitalWrite(VL53LOX_ShutdownPin, HIGH);
    DEBUG("Adafruit VL53L0X XShut set high to Allow Boot");
    delay(100);
  }

  lox.setMeasurementTimingBudgetMicroSeconds(270000);

  uint16_t calibrated_value;
  VL53L0X_RangingMeasurementData_t measure;
  DEBUG("Reading a measurement for calibrate... ");
  // Калибровка расстояния
  for (int i = 0; i < 7; i++) {
    lox.rangingTest(&measure, false);
    calibrated_value = measure.RangeMilliMeter;
    DEBUG(calibrated_value);
  }

  lox.setGpioConfig(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
                    VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW,
                    VL53L0X_INTERRUPTPOLARITY_LOW);
  if (calibrated_value > 2000) {
    calibrated_value = 2000;
  } else {
    calibrated_value;
  }
  FixPoint1616_t LowThreashHold = ((calibrated_value - 100) * 65536.0);
  lox.setInterruptThresholds(LowThreashHold, false);

  // Enable Continous Measurement Mode
  lox.setDeviceMode(VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, false);

  DEBUG("StartMeasurement... ");
  lox.startMeasurement();
}
