void toggleLed(int blinkSpeed) {
  digitalWrite(5, HIGH);
  if (ledState) {
    strip.ClearTo(RgbColor(5, 0, 5));
  } else {
    strip.ClearTo(RgbColor(0, 0, 0));
  }
  strip.Show();
  ledState = !ledState;

  ticker.attach_ms(blinkSpeed, toggleLed, blinkSpeed);
}

void startBlinking(int blinkingSpeed) {
  ticker.attach_ms(blinkingSpeed, toggleLed, blinkingSpeed);
}

void stopBlinking() {
  ticker.detach();
  strip.ClearTo(RgbColor(0, 0, 0));
  strip.Show();
  delay(5);
  digitalWrite(5, LOW);
}

void goToSleep() {
  Serial.println("going to sleep");
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
    result += String(mac[i], HEX);
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

void mqtt_connect() {
  const char* mqtt_usr = json["mqttusr"].as<const char*>();
  const char* mqtt_pass = json["mqttpass"].as<const char*>();
  int i = 0;
  while (!client.connected() && i < 5) { // Try 5 times, then give up and go to sleep.
    Serial.println("Attempting MQTT connection...");
    if (mqtt_usr[0] != '\0' && mqtt_pass[0] != '\0') {
      if (client.connect(String("hugo_" + macLastThreeSegments(mac)).c_str(), mqtt_usr, mqtt_pass)) {
        Serial.println("MQTT connected using credentials.");
        return;
      }
    } else {
      if (client.connect(String("hugo_" + macLastThreeSegments(mac)).c_str())) {
        Serial.println("MQTT connected anonymously.");
        return;
      }
    }
    Serial.print("MQTT connection attempt failed, rc=");
    Serial.println(client.state());
    ++i;
    delay(10);
  }
  goToSleep();
}

bool publishButtonData(String topic, String payload) {
  topic.replace("[id]", macLastThreeSegments(mac));
  if (topic.length() > 0 && payload.length() > 0) {
    return client.publish(topic.c_str(), payload.c_str());
  } else {
    Serial.println("Button target is not defined. Set it in config portal.");
  }
}

void publishBatteryLevel() {
  String batTopic = json["batt"].as<String>();
  batTopic.replace("[id]", macLastThreeSegments(mac));
  if (batTopic.length() > 0) {
    delay(20); // lets give the broker little breath time
    client.publish(batTopic.c_str(), String(batteryPercentage()).c_str());
    Serial.print("Battery percentage: ");
    Serial.print(batteryPercentage());
    Serial.println("%");
  }
}

int readButtons() {
  if (digitalRead(button1_pin) == HIGH && digitalRead(button2_pin) == HIGH) {
    return 5;
  }
  if (digitalRead(button2_pin) == HIGH && digitalRead(button3_pin) == HIGH) {
    return 6;
  }
  if (digitalRead(button3_pin) == HIGH && digitalRead(button4_pin) == HIGH) {
    return 7;
  }
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
uint8_t batteryPercentage() {
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
    json["ssid"] = json["pass"] = json["ip"] = json["gw"] = json["sn"] = json["broker"] = json["port"] = json["mqttusr"] = json["mqttpass"] = json["b1t"] = json["b2t"] = json["b3t"] = json["b4t"] = json["b5t"] = json["b6t"] = json["b7t"] = json["b1p"] = json["b2p"] = json["b3p"] = json["b4p"] = json["b5p"] = json["b6p"] = json["b7p"] = json["batt"] = "";
    saveConfig();
    goToSleep();
    return false;
  }
  DeserializationError error = deserializeJson(json, stateFile.readString());
  stateFile.close();
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
