#include <WiFiNINA.h>
#include <WiFiSSLClient.h>

char ssid[] = "Apple"; // your network SSID
char pass[] = "123456789"; // your network password

char server[] = "ap-southeast-2.aws.data.mongodb-api.com";
int port = 443; // HTTPS port

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // HTTPS client setup
  WiFiSSLClient client;
  if (!client.connect(server, port)) {
    Serial.println("Connection failed!");
    return;
  }

  // HTTP request headers and body
  String headers =
    "POST /app/data-bhiqc/endpoint/data/v1/action/insertOne HTTP/1.1\r\n"
    "Host: ap-southeast-2.aws.data.mongodb-api.com\r\n"
    "Content-Type: application/json\r\n"
    "api-key: dVKaMfJKdc2ISUivvB6T12fxjvMIi90t4vOsgGPAT7uaJsBGANz32UsG2EsWYjYX\r\n"
    "Content-Length: ";
  
  String requestBody = 
    "{"
    "\"collection\":\"sensordata\","
    "\"database\":\"database\","
    "\"dataSource\":\"sit314\","
    "\"document\":{\"value\": 1}"
    "}";
  
  // Send the HTTP request
  client.print(headers);
  client.print(requestBody.length());
  client.print("\r\n\r\n");
  client.print(requestBody);

  // Read and print the response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  // Close the connection
  client.stop();
}

void loop() {
  // You can add other code here if needed
}
