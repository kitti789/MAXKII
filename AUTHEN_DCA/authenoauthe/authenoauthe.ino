#include <WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

const char* ssid = "belink_F5";
const char* password = "025917068";
const char* server = "auth.macharge.co";
const int serverPort = 8443;
const char* realm = "/auth/realms/CSMS/protocol/openid-connect/auth";
const char* userEndpoint = "/auth";
const char* username = "amrsm00001";
const char* Password = "#4fgv6hjc32@";

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, server, serverPort);

String token; // Store the token globally

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void login() {
  String loginData = "{\"username\":\"" + String(username) + "\",\"password\":\"" + String(Password) + "\"";

  client.beginRequest();
  int statusCode = client.post(realm, "application/json", loginData);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", String(loginData.length()));
  client.endRequest();

  if (statusCode == 200) {
    String response = client.responseBody();
    DynamicJsonDocument doc(256);
    deserializeJson(doc, response);
    const char* receivedToken = doc["token"];
    if (receivedToken) {
      token = receivedToken; // Store the received token globally
      Serial.println("Token obtained: " + token);
    }
  } else {
    Serial.println("Login failed with status code: " + String(statusCode));
  }
}

void getUserData() {
  if (token != "") {
    client.beginRequest();
    int statusCode = client.get(userEndpoint);
    client.sendHeader("Authorization", "Bearer " + token);
    client.endRequest();

    if (statusCode == 200) {
      String response = client.responseBody();
      Serial.println("HTTP Code: " + String(statusCode));
      Serial.println("Response: " + response);
      // Handle the user data here
    } else {
      Serial.println("Failed to get user data. HTTP Code: " + String(statusCode));
    }
  } else {
    Serial.println("No token available. Please log in first.");
  }
}

void loop() {
  login(); // Call login to obtain a token
  getUserData(); // Call getUserData to access a protected resource

  // Delay for some time before retrying (adjust as needed)
  delay(60000); // Wait for 60 seconds before retrying
}