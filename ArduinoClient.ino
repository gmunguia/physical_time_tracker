#include <SPI.h>
#include <WiFiNINA.h>

#include "arduino_secrets.h"

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
char token[] = SECRET_IFTTT;

int status = WL_IDLE_STATUS;
char server[] = "maker.ifttt.com";

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
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

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
  sendHttpRequest();

  // do nothing forevermore:
  while (true)
    ;
}

int sendHttpRequest() {
  Serial.println("Starting connection to server...");

  if (!client.connectSSL(server, 443)) {
    Serial.println("Couldn't connect to server");
    return -1;
  }

  Serial.println("Connected");

  char firstline[120];
  sprintf(firstline, "POST /trigger/cube_turned/json/with/key/%s HTTP/1.1",
          token);

  char value1[10] = "bar";
  char body[120];
  sprintf(body, "{\"foo\":\"%s\"}", value1);

  char contentLength[20];
  sprintf(contentLength, "Content-Length: %d", strlen(body));

  client.println(firstline);
  client.println("Host: maker.ifttt.com");
  client.println("Connection: close");
  client.println("Content-Type: application/json");

  client.println(contentLength);
  client.println("");
  client.println(body);

  int c;
  while (true) {
    if (!client.available()) {
      Serial.println("not available");
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

  Serial.println("end");
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
