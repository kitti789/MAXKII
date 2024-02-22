#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize Wi-Fi
  WiFiManager wifiManager;

  // Uncomment the following line to reset Wi-Fi settings for testing
  // wifiManager.resetSettings();

  if (!wifiManager.autoConnect("ESP32WiFiManager")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }

  // If you reach here, you have successfully connected to Wi-Fi
  Serial.println("Connected to Wi-Fi");

  // Route for the main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "<html><head><style>";
    html += "body {";
    html += "  font-family: Arial, Helvetica, sans-serif;";
    html += "  background-color: #f2f2f2;"; // Background color
    html += "}";
    html += "h1 {color: #3333FF;}";
    html += "form {border: 2px solid #f2f2f2; padding: 20px; background-color: #ffffff;}";
    html += "input[type='text'], input[type='password'] {width: 100%; padding: 12px 20px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; box-sizing: border-box;}";
    html += "input[type='submit'] {background-color: #3333FF; color: white; padding: 14px 20px; margin: 8px 0; border: none; cursor: pointer;}";
    html += "a {color: #3333FF; text-decoration: none;}";
    html += "</style></head><body>";
    html += "<h1>ESP32 WiFi Configuration</h1>";
    html += "<form method='post' action='/save-config'>";
    html += "SSID: <input type='text' name='ssid' value='" + WiFi.SSID() + "'><br>";
    html += "Password: <input type='password' name='password' value='" + WiFi.psk() + "'><br>";
    html += "Static IP: <input type='text' name='static_ip' value='" + WiFi.localIP().toString() + "'><br>";
    html += "<input type='submit' value='Save Configuration'>";
    html += "</form>";
    html += "<p><a href='/reset'>Reset to default settings</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Route for handling the configuration form submission
  server.on("/save-config", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid = request->arg("ssid");
    String password = request->arg("password");
    String staticIPStr = request->arg("static_ip");

    // Validate the static IP address
    IPAddress staticIP;
    if (staticIP.fromString(staticIPStr)) {
      // Handle saving to EEPROM or other storage
      // For simplicity, we just store it in memory

      // Redirect to the configuration page with updated values
      request->send(200, "text/plain", "Configuration saved.");
    } else {
      request->send(400, "text/plain", "Invalid Static IP");
    }
  });

  // Route for resetting to default settings
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    request->send(200, "text/plain", "Settings reset. Rebooting...");
    delay(2000);
    ESP.restart();
  });

  // Start the server
  server.begin();
}

void loop() {
  // Your main loop code here
}