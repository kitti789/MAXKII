#include <Wire.h>
#include "SDM120.h"
#include <WiFi.h>
#include <WiFiManager.h> 
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <SPI.h>
#include <ArduinoJson.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LED           17
#define BAUD_RATE     9600
#define RS485_CONTROL 4
#define TICTAG_SDM_ID 1

AsyncWebServer server(80);
const int buttonPin = 4;
const int ledPin = 5;
WiFiClient client;
SDM TictagSDM(Serial, BAUD_RATE, RS485_CONTROL);
char tempdataprint[100];
int buttonState = 0;
//------------------------- AUTHENSERVER ----------------------------//
const char* authServerURL = "https://auth.macharge.co:8443/auth/realms/CSMS/protocol/openid-connect/token";
const char* realmName = "CSMS";
const char* clientID = "csmsMgtClient";
const char* clientSecret = "naIYW5t1if5ZeOaecFfNKl5doLIDywBd";
const char* accessToken = "eyJhbGciOiJSUzI1NiIsInR5cCIgOiAiSldUIiwia2lkIiA6ICI1MFRSNTJzQ3VGdHpoY3QtQTRIT0p1WVM4WFJ3aFg3MkdfeVltdmxnSkZjIn0.eyJleHAiOjE3MDg0MzU5MTIsImlhdCI6MTcwODM5MjcxMiwianRpIjoiYWVmYzI3NDktMmIyYS00NDU5LTgyZDYtNWZlZGU5MTU0YTFiIiwiaXNzIjoiaHR0cHM6Ly9hdXRoLm1hY2hhcmdlLmNvOjg0NDMvYXV0aC9yZWFsbXMvQ1NNUyIsImF1ZCI6ImFjY291bnQiLCJzdWIiOiJjYTZmNWFmOC00YmQzLTQ2M2MtYjZmYS1jYmFjYTFhMTczMDEiLCJ0eXAiOiJCZWFyZXIiLCJhenAiOiJjc21zTWd0Q2xpZW50Iiwic2Vzc2lvbl9zdGF0ZSI6ImZjYmVmZDU4LWQ4NDItNDk3MC1iOWM1LWFlMWRmMzRjZTc4NSIsImFjciI6IjEiLCJyZWFsbV9hY2Nlc3MiOnsicm9sZXMiOlsiZGVmYXVsdC1yb2xlcy1jc21zIiwib2ZmbGluZV9hY2Nlc3MiLCJ1bWFfYXV0aG9yaXphdGlvbiJdfSwicmVzb3VyY2VfYWNjZXNzIjp7ImNzbXNNZ3RDbGllbnQiOnsicm9sZXMiOlsiVEhBTVJIMDAwMDAyOlVQRCJdfSwiYWNjb3VudCI6eyJyb2xlcyI6WyJtYW5hZ2UtYWNjb3VudCIsIm1hbmFnZS1hY2NvdW50LWxpbmtzIiwidmlldy1wcm9maWxlIl19fSwic2NvcGUiOiJwcm9maWxlIGVtYWlsIiwic2lkIjoiZmNiZWZkNTgtZDg0Mi00OTcwLWI5YzUtYWUxZGYzNGNlNzg1IiwiZW1haWxfdmVyaWZpZWQiOnRydWUsInByZWZlcnJlZF91c2VybmFtZSI6ImFtcnNtMDAwMDEiLCJnaXZlbl9uYW1lIjoiIiwiZmFtaWx5X25hbWUiOiIifQ.WymOcg2NfQgEWxXOenh3RwQx1QMq-LkBXXw5B5oOO6dLAnO_PvG5KDuY4OG-HkR5wRk1elyez6b5-eNElazsLcWy46rle7M_wrRxYHHjHU-JlN66yHPN3S_r5OOgMlvED1qANf2qdXyXZ2hMa-_k6xFq3J4u7ggfZpCkWhg7gSRTwFwSy8WVvefSh7rV8K5EAYcz4WtMsbQHkDMHqa6tHnjvdY4neFB3ptOglSrILzYP3wEn7QmE0ZFaLb6A3WWBLXMCFWolX4nyQVkP3pPzGtj9cRuyR9d-y_yl534Hp_nogAwIqWcrA7-x5GrdvGWD3-ikoCY5tF6hVz17x0tsFA";
const char* meterUserName = "amrsm00001";
const char* meterPassword = "#4fgv6hjc32@";

//------------------- Password -------------------------//
String seriesNumber = "";
String encryptPassword = "";
String key = "";
String generateRandom(int length);
String convert(String seriesNumber, String key);

//------------------- General------------------------------//
String selectedAmperage = "999";
int lastValue = 0;
void configureWiFi(const String& username, const String& password);
HTTPClient http;
bool ledOn = false;
float Current;  // Declare Current here
int httpCode; 

//--------------------------- ACCESS TOKEN -----------------------------//
  void obtainAccessToken(String& accessToken) {
    if (accessToken.isEmpty() || isTokenExpiringSoon(accessToken)) {
  http.begin(authServerURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Create the request body with the client credentials, username, and password
  String requestBody = "grant_type=password"
                       "&client_id=" + String(clientID) +
                       "&client_secret=" + String(clientSecret) +
                       "&realm=" + String(realmName) +
                       "&username=" + String(meterUserName) +
                       "&password=" + String(meterPassword);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    accessToken = http.getString(); // Store the access token in the variable
    Serial.println("HTTP Response Code (Obtain Token): " + String(httpResponseCode));
    Serial.println("Server Response (Obtain Token): " + accessToken);
  } else {
    Serial.println("Failed to obtain access token");
  }

  http.end();
}else {
    Serial.println("Using existing access token");
  }
}
   bool isTokenExpiringSoon(const String& accessToken) {
   int indexOfExpiresIn = accessToken.indexOf("\"expires_in\":");
      if (indexOfExpiresIn != -1) {
   int indexOfComma = accessToken.indexOf(',', indexOfExpiresIn);
      if (indexOfComma != -1) {
      String expiresInSecondsStr = accessToken.substring(indexOfExpiresIn + 14, indexOfComma); // 14 is the length of "\"expires_in\":"
      unsigned long expiresInSeconds = expiresInSecondsStr.toInt();
      unsigned long expirationTime = expiresInSeconds * 1000 + millis(); // Convert seconds to milliseconds

      return (expirationTime - millis() < 3600000);
    }
  }

  return true;  // Treat as expired if expiration information is not available
} 
//--------------------------- SETUP -------------------------/
void setup() {
   HTTPClient http; // Move the HTTPClient instantiation here as well

  Serial.begin(9600);
  delay(1000);

  WiFiManager wifiManager;// WIFI MANAGER

   if (!wifiManager.autoConnect("ESP32WiFiManager")) {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
  }
  Serial.println("Connected to Wi-Fi");

   String accessToken; // ACCESS TOKEN
   obtainAccessToken(accessToken);
   Serial.println("Access Token: " + accessToken);
   Serial.println("Connected to Wi-Fi");


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {// HTML WEBSERVER
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
    html += "Username: <input type='text' name='username' value='" + WiFi.SSID() + "'><br>";
    html += "Password: <input type='password' name='password' value='" + WiFi.psk() + "'><br>";
    html += "Static IP: <input type='text' name='static_ip' value='" + WiFi.localIP().toString() + "'><br>";
    html += "Amperage: <select name='amperage'>";
   // html += String("<option value='50' ") + (selectedAmperage == "50A" ? "selected" : "") + ">50A</option>";
    html += String("<option value='100' ") + (selectedAmperage == "100A" ? "selected" : "") + ">100A</option>";
    html += "</select><br>";
    html += "Series Number: <input type='text' name='series_number'><br>"; // Added series number input
    html += "Meter Number: <input type='text' name='meter_number'><br>";   // Added meter number input
    html += "<input type='submit' value='Save Configuration'>";
    html += "</form>";
    html += "<p><a href='/reset'>Reset to default settings</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/save-config", HTTP_POST, [](AsyncWebServerRequest *request) {// CONFIGURATE WEB
    String username = request->arg("username");
    String password = request->arg("password");
    String staticIPStr = request->arg("static_ip");
    selectedAmperage = request->arg("amperage");
    String seriesNumber = request->arg("series_number");
    String meterNumber = request->arg("meter_number");

    configureWiFi(username, password);
    
    String response = "WiFi credentials updated. New IP address: " + WiFi.localIP().toString();
    response += "<br><a href='/'>Go back to main page</a>";
    request->send(200, "text/html", response);

    IPAddress staticIP;
    if (staticIP.fromString(staticIPStr)) {
      Serial.print("Selected Amperage: ");
      Serial.println(selectedAmperage);

      key = generateRandom(10);//ENCRYPT NUMBER

      // Encrypt password using series number and key
      encryptPassword = convert(seriesNumber, key);
      // Handle saving to EEPROM or other storage
      // For simplicity, we just store it in memory

        Serial.print("Generated Encrypt Password: ");
      Serial.println(encryptPassword);
      // Redirect to the configuration page with updated values
      request->send(200, "text/plain", "Configuration saved.");
    } else {
      request->send(400, "text/plain", "Invalid Static IP");
    }
  });

  // Route for resetting to default settings
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {// WEBSERBER RESET
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    request->send(200, "text/plain", "Settings reset. Rebooting...");
    delay(2000);
    ESP.restart();
  });

 
  server.begin();// Start the server

  Serial.begin(BAUD_RATE);
  pinMode(LED, OUTPUT);
  TictagSDM.begin(TICTAG_SDM_ID);

  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "What the frick.");
  });

  AsyncElegantOTA.begin(&server);// OTA WEB
  server.begin();
  Serial.println("HTTP server started");
}

void configureWiFi(const String& username, const String& password) {// CONFIGURE WIFI
  Serial.println("Configuring WiFi with new credentials...");
  Serial.print("Username: ");
  Serial.println(username);
  Serial.print("Password: ");
  Serial.println(password);

  WiFi.begin(username.c_str(), password.c_str());

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void updateMeterValue(float meterValue) {// POST VALUE TO URL
  // Build the URL with the updated meter value
   static unsigned long lastPostTime = 0;

  String updateMeterURL = "https://csms.macharge.co/api/updateMeter/THAMRH000002?mid=amrsm00001&value=" + String(meterValue);

  // Create an HTTP client
  HTTPClient http;

  // Begin the request
  http.begin(updateMeterURL);
  http.addHeader("Authorization", "Bearer " + String(accessToken)); // Use your access token here
  http.addHeader("Content-Type", "application/json");

  // Perform the POST request
  int updateResponseCode = http.POST("{}"); // You can replace "{}" with the actual data you want to send in the POST request


  if (updateResponseCode == 200) {
    Serial.println("Update Meter Response Code: " + String(updateResponseCode));
    Serial.println("Update Meter Response: " + http.getString());

    // Update the last successful post time
    lastPostTime = millis();

    if (!ledOn) {
      digitalWrite(LED, HIGH);
      ledOn = true;
    }
  } else {
    Serial.println("Update Meter request failed");
    for (int i = 0; i < 3; i++) { // Flash the LED 3 times
      digitalWrite(LED, HIGH);
      delay(500);  // You can adjust the delay as needed
      digitalWrite(LED, LOW);
      delay(500);
    }
 digitalWrite(LED, LOW);
    ledOn = false;
  }
  http.end();
}


void loop() {
  String accessToken;
  obtainAccessToken(accessToken);

  seriesNumber = "amrsm00001";
  encryptPassword = convert(seriesNumber, key);

  AsyncElegantOTA.loop();
  HTTPClient client;
  client.addHeader("Content-Type", "application/json");
  buttonState = digitalRead(buttonPin);


  byte fpPrecision = 2; // < floating point precision // CODE ABOUT SDM120M
  SDM::Type type;
  float Current;
  float Total_active_energy;
  float value;
  for (int i = 0; i <= (int)SDM::NODE_ID; i++) {
    delay(100);
    type = SDM::VOLTAGE;
    type = (SDM::Type)(int)i;

    String info = "";
    String unit = "";

    switch (type) {
      case SDM::VOLTAGE:
        info = "Voltage: ";
        unit = "V";
        break;
      case SDM::CURRENT:
        info = "Current: ";
        Current = TictagSDM.getValue((SDM::Type)type);
        unit = "A";
        break;
      case SDM::FREQUENCY:
        info = "Frequency: ";
        unit = "Hz";
        break;
      case SDM::TOTAL_ACTIVE_ENERGY:
        info = "Total_active_energy: ";
        unit = "KWh";
        break;
      case SDM::BAUDRATE:
        info = "Baud rate: ";
        unit = "";
        break;
      case SDM::NODE_ID:
        info = "Node ID: ";
        unit = "";
        break;
      default:
        info = "N/A";
        unit = "N/A";
        break;
    }

    value = TictagSDM.getValue((SDM::Type)type);
    Serial.print(info);
    Serial.print(value, fpPrecision);
    Serial.println(unit);
    delay(3000);
  }

  Serial.println("...................................................");

  Serial.println(Current);
  Serial.println(Current);

  float threshold = 80.0; // Set the threshold value for current (in amps)
const unsigned long postInterval = 60000; // 2 minutes in milliseconds
static unsigned long lastPostTime = 0;
static bool postAllowed = true; // Flag to track if posting is allowed

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
if (Current > threshold && postAllowed) {
  Serial.println("Current is above 80 amps. Posting data to server...");
  updateMeterValue(Current);
  postAllowed = false; // Set the flag to false to indicate that posting is not allowed
}

// Check if the current is below the threshold
if (Current < threshold && postAllowed) {
  Serial.println("Current is below 80 amps. Posting data to server...");
  updateMeterValue(Current);
  postAllowed = false; // Set the flag to false to indicate that posting is not allowed
}

// Check if 2 minutes have elapsed since the last successful post
if ((Current != threshold) && (millis() - lastPostTime >= postInterval)) {
  // Reset the flag to allow new posts
  postAllowed = true;
  lastPostTime = millis(); // Update the last post time
}
}

  ////  if (abs(Current - lastValue) >= selectedAmperage.toInt() * 0.80) {
   // client.begin("https://csms.macharge.co/api/updateMeter/THAMRH000002?mid=x00001&meterValue=" + String(Current));
   // httpCode = client.POST("mid=\"x00001\"&meterValue=");
   // delay(1000);
   // lastValue = Current;
 // } else {
  // client.begin("https://csms.macharge.co/api/updateMeter/THAMRH000002?mid=x00001&meterValue=" + String(Current));
   // httpCode = client.POST("mid=\"x00001\"&meterValue=");
   //// delay(1000);
   // lastValue = Current;
  //}

  //if (httpCode > 0) {
  //  String payload = client.getString();
  //  Serial.println("\nStatuscode: " + String(httpCode));
  //  Serial.println(payload);
  //  client.end();
  //} else {
 //   Serial.println("Error on HTTP request");
 // }
//delay(10000);
//}

String convert(String seriesNumber, String key) {
  String str = seriesNumber + key;
  String pass = "";
  for (int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    pass += String(c, HEX);
  }

  return pass;
}

String generateRandom(int length) {
  String ch = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  String rv = "";

  for (int i = 0; i < length; i++) {
    int ri = random(ch.length());
    rv += ch.charAt(ri);
  }

  return rv;
}
