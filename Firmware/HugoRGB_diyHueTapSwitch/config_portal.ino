
void startConfigPortal() {
  startBlinking(CONFIG_BLINK_SPEED);
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  IPAddress ap_ip(10, 10, 10, 1);
  WiFi.softAPConfig(ap_ip, ap_ip, IPAddress(255, 255, 255, 0));
  String ap_name = AP_NAME + macLastThreeSegments(mac);
  WiFi.softAP(ap_name.c_str());
  IPAddress ip = WiFi.softAPIP();
  Serial.print("Config portal IP address: ");
  Serial.println(ip);
  server.on("/", handleRoot);
  server.begin();

  delay(3000);

  while (deviceMode == CONFIG_MODE) { // BLOCKING INFINITE LOOP
    if (digitalRead(button1_pin) == HIGH || digitalRead(button2_pin) == HIGH || digitalRead(button3_pin) == HIGH || digitalRead(button4_pin) == HIGH || millis() - configTimer > CONFIG_TIMEOUT) {
      stopBlinking();
      goToSleep();
      return;
    }
    server.handleClient();
  }

}

void toggleConfigMode() {
  if (digitalRead(button1_pin) == HIGH && digitalRead(button4_pin) == HIGH) {
    int i = 0;
    while (digitalRead(button1_pin) == HIGH && digitalRead(button4_pin) == HIGH && i < 200) {
      delay(10);

      if (i > 100) {
        deviceMode = CONFIG_MODE;
        configTimer = millis(); // start counter
        return;
      }
      i++;
    }
  }
}

void handleRoot() {
  if (server.args()) {

    if (server.hasArg("ssid")) {
      json["ssid"] = server.arg("ssid");
    }
    if (server.hasArg("pass")) {
      json["pass"] = server.arg("pass");
    }
    if (server.hasArg("ip")) {
      json["ip"] = server.arg("ip");
    }
    if (server.hasArg("gw")) {
      json["gw"] = server.arg("gw");
    }
    if (server.hasArg("sn")) {
      json["sn"] = server.arg("sn");
    }
    if (server.hasArg("bridge")) {
      json["bridge"] = server.arg("bridge");
    }
    
    if (server.hasArg("b1r")) json["b1r"] = server.arg("b1r");
    if (server.hasArg("b1g")) json["b1g"] = server.arg("b1g");
    if (server.hasArg("b1b")) json["b1b"] = server.arg("b1b");
    
    if (server.hasArg("b2r")) json["b2r"] = server.arg("b2r");
    if (server.hasArg("b2g")) json["b2g"] = server.arg("b2g");
    if (server.hasArg("b2b")) json["b2b"] = server.arg("b2b");
    
    if (server.hasArg("b3r")) json["b3r"] = server.arg("b3r");
    if (server.hasArg("b3g")) json["b3g"] = server.arg("b3g");
    if (server.hasArg("b3b")) json["b3b"] = server.arg("b3b");
    
    if (server.hasArg("b4r")) json["b4r"] = server.arg("b4r");
    if (server.hasArg("b4g")) json["b4g"] = server.arg("b4g");
    if (server.hasArg("b4b")) json["b4b"] = server.arg("b4b");
    
    saveConfig();
  }

  const char* batteryColor = "#a53e3e"; // default RED
  int batteryPercent = batteryPercentage;
  if (batteryPercent >= 40) batteryColor = "#a57d3e";
  if (batteryPercent >= 60) batteryColor = "#9ea53e";
  if (batteryPercent >= 80) batteryColor = "#7ca53e";
  if (batteryPercent > 100) batteryColor = "#3e7ea5";
  String html = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>Hugo Configuration</title><style>html,body{margin:0;padding:0;font-size:16px;background:#444444;}body,*{box-sizing:border-box;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Roboto,\"Helvetica Neue\",Arial,sans-serif;}a{color:inherit;text-decoration:underline;}.wrapper{padding:30px 0;}.container{margin:auto;padding:40px;max-width:500px;color:#fff;background:#000;box-shadow:0 0 100px rgba(0,0,0,.5);border-radius:50px;}.row{margin-bottom:15px;}h1{margin:0 0 10px 0;font-family:Arial,sans-serif;font-weight:300;font-size:2rem;}h1 + p{margin-bottom:30px;}h2{margin:30px 0 0 0;font-family:Arial,sans-serif;font-weight:300;font-size:1.5rem;}p{font-size:.85rem;margin:0 0 20px 0;color:rgba(255,255,255,.7);}label{display:block;width:100%;margin-bottom:5px;}input[type=\"text\"],input[type=\"password\"],input[type=\"number\"]{display:inline-block;width:100%;height:42px;line-height:38px;padding:0 20px;color:#fff;border:2px solid #666;background:none;border-radius:5px;transition:.15s;box-shadow:none;outline:none;}input[type=\"text\"]:hover,input[type=\"password\"]:hover{border-color:#ababab;}input[type=\"text\"]:focus,input[type=\"password\"]:focus{border-color:#fff;}button{display:block;width:100%;padding:10px 20px;font-size:1rem;font-weight:700;text-transform:uppercase;background:#ff9c29;border:0;border-radius:5px;cursor:pointer;transition:.15s;outline:none;}button:hover{background:#ffba66;}.github{margin-top:15px;text-align:center;}.github a{color:#ff9c29;transition:.15s;}.github a:hover{color:#ffba66;}.bat p{margin:0 0 5px 0;text-align:center;text-transform:uppercase;font-size:.8rem;}.bat >div{position:relative;margin:0 auto 20px;width:300px;height:10px;background:#272727;border-radius:5px;}.bat >div >div{position:absolute;left:0;top:0;bottom:0;border-radius:5px;min-width:10px;}.mac{display:inline-block;margin-top:8px;padding:2px 5px;color:#fff;background:#444;border-radius:3px;}</style><style media=\"all and (max-width:520px)\">.wrapper{padding:20px 0;}.container{padding:25px 15px;border-radius:0;}</style></head><body><div class=\"wrapper\">";
  html += "<div class=\"bat\"><p>Battery level: " + ((batteryPercent > 100) ? "Charging" : (String)batteryPercent + "%") + "</p><div><div style=\"background: " + batteryColor + ";width: " + ((batteryPercent > 100) ? 100 : batteryPercent) + "%\"></div></div></div>";
  html += "<div class=\"container\"> <form method=\"post\" action=\"/\"> <h1>HugoRGB Configuration</h1> <p>Press any of the Hugo's buttons to shut down config AP and resume normal function.</p> <h2>Network settings</h2> <p>Select your network settings here.</p> <div class=\"row\"> <label for=\"ssid\">WiFi SSID</label> <input type=\"text\" id=\"ssid\" name=\"ssid\" value=\"";
  html += json["ssid"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"pass\">WIFI Password</label> <input type=\"password\" id=\"pass\" name=\"pass\" value=\"";
  html += json["pass"].as<const char*>();
  html += "\"> </div> <h2>Static IP settings (optional)</h2> <p>Optional settings for static IP, in some cases this might speed up response time. All 3 need to be set and IP should be reserved in router's DHCP settings.";
  html += "<br>MAC address: <span class=\"mac\">";
  html += macToStr(mac);
  html += "</span></p>";
  html += "<div class=\"row\"> <label for=\"ip\">IP Address (optional):</label> <input type=\"text\" id=\"ip\" name=\"ip\" value=\"";
  html += json["ip"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"gw\">Gateway IP (optional):</label> <input type=\"text\" id=\"gw\" name=\"gw\" value=\"";
  html += json["gw"].as<const char*>();
  html += "\"> </div> <div class=\"row\"> <label for=\"sn\">Subnet mask (optional):</label> <input type=\"text\" id=\"sn\" name=\"sn\" value=\"";
  html += json["sn"].as<const char*>();
  html += "\"> </div> <h2>diyHue settings</h2><p>For the remote to work, specify your diyHue bridge IP address.<br>For example: \"192.168.0.100\"</p> <div class=\"row\"> <label for=\"bridge\">Bridge IP</label>";
  html += "<input type=\"text\" id=\"bridge\" name=\"bridge\" value=\"";
  html += json["bridge"].as<const char*>();
  html += "\">";
  html += "</div> <h2>LED settings</h2><p>HugoRGB can do colors. Use that power.<br>You can use values 0-50 for each channel. Amber orange by default. I suggest against using values greater than 20 as you will get blinded. :)</p>";
  html += "<div class=\"row\"> <label for=\"bridge\">Button 1 color (R, G, B)</label>";
  html += "<input type=\"number\" id=\"b1r\" name=\"b1r\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b1r"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b1g\" name=\"b1g\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b1g"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b1b\" name=\"b1b\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b1b"].as<const char*>();
  html += "\">";
  html += "</div>";

  html += "<div class=\"row\"> <label for=\"bridge\">Button 2 color (R, G, B)</label>";
  html += "<input type=\"number\" id=\"b2r\" name=\"b2r\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b2r"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b2g\" name=\"b2g\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b2g"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b2b\" name=\"b2b\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b2b"].as<const char*>();
  html += "\">";
  html += "</div>";
  
  html += "<div class=\"row\"> <label for=\"bridge\">Button 3 color (R, G, B)</label>";
  html += "<input type=\"number\" id=\"b3r\" name=\"b3r\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b3r"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b3g\" name=\"b3g\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b3g"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b3b\" name=\"b3b\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b3b"].as<const char*>();
  html += "\">";
  html += "</div>";
  
  html += "<div class=\"row\"> <label for=\"bridge\">Button 4 color (R, G, B)</label>";
  html += "<input type=\"number\" id=\"b4r\" name=\"b4r\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b4r"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b4g\" name=\"b4g\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b4g"].as<const char*>();
  html += "\">";
  html += "<input type=\"number\" id=\"b4b\" name=\"b4b\" style=\"width: 20%;margin-right: 10px;\" min=\"0\" max=\"50\" value=\"";
  html += json["b4b"].as<const char*>();
  html += "\">";
  html += "</div>";

  html += "<div class=\"row\"> <button type=\"submit\">Save and reboot</button> </div> </form> </div>";
  html += "<div class=\"github\"> <p>diyHue firmware ";
  html += FW_VERSION;
  html += ", check out <a href=\"https://github.com/mcer12/Hugo-ESP8266\" target=\"_blank\"><strong>Hugo</strong> on GitHub</a></p> </div>";
  html += "</div> </body> </html>";
  server.send(200, "text/html", html);

  if (server.args()) {
    delay(1000);
    ESP.reset();
    delay(100);
  }
}
