void toggleLed(int blinkSpeed) {
  digitalWrite(5, ledState);
  ledState = !ledState;

  ticker.attach_ms(blinkSpeed, toggleLed, blinkSpeed);
}

void startBlinking(int blinkingSpeed) {
  ticker.attach_ms(blinkingSpeed, toggleLed, blinkingSpeed);
}

void blinkLed(int time_ms) {
  digitalWrite(5, HIGH);
  delay(time_ms);
  digitalWrite(5, LOW);
}

void stopBlinking() {
  ticker.detach();
  digitalWrite(5, LOW);
}

void lowBatteryAlert() {
  for (int i = 0; i < 6; i++ ) {
    if (i % 2 == 0) {
      digitalWrite(5, HIGH);
    } else {
      digitalWrite(5, LOW);
    }
    if (i < 5) delay(200);
  }
}

void goToSleep() {
  Serial.println("Going to sleep...");
  yield();
  delay(5);
  ESP.deepSleep(0);
  delay(100);
  yield();
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  result.toUpperCase();
  return result;
}

String macLastThreeSegments(const uint8_t* mac) {
  String result;
  for (int i = 3; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
  }
  result.toUpperCase();
  return result;
}

void sendHttpRequest(int requestCode) {
  const char* bridgeIp = json["bridge"].as<const char*>();
  if (bridgeIp[0] == '\0') {
    Serial.println("diyHue bridge is not specified. Check your configuration.");
    return;
  }
  int batteryPercent = batteryPercentage;
  //if (batteryPercent > 100) batteryPercent = 100;
  WiFiClient client;
  String url = "/switch?mac=" + macToStr(mac) + "&button=" + requestCode + "&battery=" + batteryPercent;
  client.connect(bridgeIp, 80);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + bridgeIp + "\r\n" +
               "Connection: close\r\n\r\n");
  Serial.println(url);
}

int readButtons() {
  if (digitalRead(button1_pin) == HIGH) {
    return 1;
  } else if (digitalRead(button2_pin) == HIGH) {
    return 2;
  } else if (digitalRead(button3_pin) == HIGH) {
    return 3;
  } else if (digitalRead(button4_pin) == HIGH) {
    return 4;
  }
  return 0;
}

/* Read analog input for battery measurement */
int ReadAIN()
{
  int Read = analogRead(A0);
  int num = 10;

  for (int i = 0 ; i < num ; i++)
  {
    int newRead = analogRead(A0);
    if (newRead > Read)
    {
      Read = newRead;
    }
    delay(1);
  }
  return (Read);
}

/* Battery percentage estimation, this is not very accurate but close enough */
uint8_t getBatteryPercentage() {
  int analogValue = ReadAIN();
  if (analogValue > 1000) return 101; // CHARGING
  if (analogValue > 960) return 100;
  if (analogValue > 940) return 90;
  if (analogValue > 931) return 80;
  if (analogValue > 922) return 70;
  if (analogValue > 913) return 60; // 3.8v ... 920
  if (analogValue > 904) return 50;
  if (analogValue > 895) return 40;
  if (analogValue > 886) return 30;
  if (analogValue > 877) return 20; // 3.65v ... 880
  if (analogValue > 868) return 10;
  return 0;
}

bool readConfig() {
  File stateFile = SPIFFS.open("/config.json", "r");
  if (!stateFile) {
    Serial.println("Failed to read config file... first run?");
    Serial.println("Creating file and going to sleep. Try again!");
    json["ssid"] = json["pass"] = json["ip"] = json["gw"] = json["sn"] = json["bridge"] = "";
    saveConfig();
    goToSleep();
    return false;
  }
  DeserializationError error = deserializeJson(json, stateFile.readString());
  stateFile.close();
  //Serial.println("json:");
  //serializeJson(json, Serial);
  return true;
}

bool saveConfig() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  serializeJson(json, configFile);
  //serializeJson(json, Serial);
  configFile.close();
  return true;
}
