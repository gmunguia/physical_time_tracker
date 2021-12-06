#include <Arduino_LSM6DS3.h>
#include <SPI.h>
#include <WiFiNINA.h>

#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char token[] = SECRET_TOKEN;

int status = WL_IDLE_STATUS;
char server[] = "hook.integromat.com";

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

enum Face { A = 0, B, C, D, E, F, None };

void setup() {
  Serial.begin(9600);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1)
      ;
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP
    // network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();
}

enum Face lastStableFace;
void loop() {
  enum Face currentFace = getNextStableFace();

  Serial.print("Current face is: ");
  Serial.println(currentFace);

  if (currentFace == lastStableFace) {
    return;
  }
  lastStableFace = currentFace;

  char body[100];
  sprintf(body, "{\"face\":%d}", currentFace);

  sendHttpRequest(body);
}

int sendHttpRequest(char body[]) {
  Serial.println("Starting connection to server...");

  if (!client.connectSSL(server, 443)) {
    Serial.println("Couldn't connect to server");
    return -1;
  }

  Serial.println("Connected");

  char firstline[120];
  sprintf(firstline, "POST /%s HTTP/1.1", token);

  char contentLength[20];
  sprintf(contentLength, "Content-Length: %d", strlen(body));

  char host[40];
  sprintf(host, "Host: %s", server);

  client.println(firstline);
  client.println(host);
  client.println("Connection: close");
  client.println("Content-Type: application/json");

  client.println(contentLength);
  client.println("");
  client.println(body);

  int c;
  while (true) {
    if (!client.available()) {
      Serial.print('.');
      delay(100);
      continue;
    }

    char c = client.read();

    Serial.print(c);

    if (c == '\n') {
      // Done with headers.
      break;
    }
  }
}

enum Face getCurrentFace() {
  while (true) {
    if (!IMU.accelerationAvailable()) {
      continue;
    }

    float x, y, z;
    IMU.readAcceleration(x, y, z);

    Serial.print("x:");
    Serial.print(x);
    Serial.print(",y:");
    Serial.print(y);
    Serial.print(",z:");
    Serial.println(z);

    if (x > .9)
      return A;
    if (x < -.9)
      return B;
    if (y > .9)
      return C;
    if (y < -.9)
      return D;
    if (z > .9)
      return E;
    if (z < -.9)
      return F;

    return None;
  }
}

enum Face getNextStableFace() {
  int consecutiveReadsRequired = 5;
  int readingCountdown = consecutiveReadsRequired;
  enum Face lastFace = getCurrentFace();

  while (readingCountdown > 0) {
    delay(100);

    enum Face currentFace = getCurrentFace();

    --readingCountdown;

    if (currentFace != lastFace) {
      readingCountdown = consecutiveReadsRequired;
      lastFace = currentFace;
    }
  }

  return lastFace;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
