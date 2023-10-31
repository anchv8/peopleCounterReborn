
String getTime() {
  char buffer[6];  // "HH:MM" + нулевой символ
  snprintf(buffer, sizeof(buffer), "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
  return String(buffer);
}

String getDate() {

  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = localtime(&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  int currentYear = ptm->tm_year + 1900;
  char buffer[11];  // "YYYY-MM-DD" + нулевой символ
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", currentYear, currentMonth, monthDay);
  return String(buffer);
}