#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Constants
const char* ssid = "ESP8266_AP";
const char* password = "password123";
const int relayPin = D1; // Assuming the relay is connected to GPIO pin D1
const int moisturePin = A0; // Assuming the moisture sensor is connected to analog pin A0
const int moistureThreshold = 500; // Adjust this threshold as needed
const int wifiConnectionAttempts = 10; // Number of attempts to connect to Wi-Fi
const int delayInterval = 1000; // Delay interval in milliseconds

ESP8266WebServer server(80);

// HTML form for entering Wi-Fi credentials
const char* htmlForm = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Wi-Fi Configuration</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 0;
      background-color: #f5f5f5;
    }

    .container {
      max-width: 400px;
      margin: 50px auto;
      background-color: #fff;
      border-radius: 8px;
      padding: 20px;
      box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1);
    }

    h2 {
      text-align: center;
      margin-bottom: 20px;
    }

    label {
      font-weight: bold;
    }

    input[type="text"],
    input[type="password"] {
      width: calc(100% - 20px);
      margin-bottom: 10px;
      padding: 10px;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
    }

    input[type="submit"] {
      width: 100%;
      background-color: #4CAF50;
      color: white;
      padding: 14px 20px;
      margin: 8px 0;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
    }

    input[type="submit"]:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>

<div class="container">
  <h2>Wi-Fi Configuration</h2>
  <form method="get" action="submit">
    <label for="ssid">SSID:</label><br>
    <input type="text" id="ssid" name="ssid" placeholder="Enter SSID"><br>
    <label for="password">Password:</label><br>
    <input type="password" id="password" name="password" placeholder="Enter password"><br><br>
    <input type="submit" value="Submit">
  </form>
</div>

</body>
</html>
)=====";

void setup() {
  Serial.begin(115200);

  pinMode(relayPin, OUTPUT);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.print("Access Point IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/submit", HTTP_GET, handleSubmit);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Read moisture level
  int moistureLevel = analogRead(moisturePin);

  // Control relay based on moisture level
  if (moistureLevel < moistureThreshold) {
    digitalWrite(relayPin, HIGH); // Turn on relay
  } else {
    digitalWrite(relayPin, LOW); // Turn off relay
  }

  delay(delayInterval); // Delay for stability
}

void handleRoot() {
  server.send(200, "text/html", htmlForm);
}

void handleSubmit() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Save WiFi credentials to EEPROM
  saveCredentials(ssid, password);

  // Attempt to connect to the main Wi-Fi network
  Serial.println("Connecting to main network...");
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < wifiConnectionAttempts) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    server.send(200, "text/plain", "Successfully connected to the main network!");
  } else {
    Serial.println("");
    Serial.println("Connection failed. Check your credentials.");
    server.send(200, "text/plain", "Connection failed. Check your credentials.");
  }
}

void saveCredentials(String ssid, String password) {
  // Clear previous credentials
  for (int i = 0; i < EEPROM.length(); ++i) {
    EEPROM.write(i, 0);
  }

  // Write new credentials to EEPROM
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.write(ssid.length(), '\0'); // Null-terminate the string

  for (int i = 0; i < password.length(); ++i) {
    EEPROM.write(ssid.length() + 1 + i, password[i]);
  }
  EEPROM.write(ssid.length() + 1 + password.length(), '\0'); // Null-terminate the string

  EEPROM.commit();
}
