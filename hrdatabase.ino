#include <ArduinoBLE.h>
#include <WiFiNINA.h>
#include <WiFiSSLClient.h>

char ssid[] = "aa"; // your network SSID
char pass[] = "123456789"; // your network password

char server[] = "ap-southeast-2.aws.data.mongodb-api.com";
int port = 443; // HTTPS port

// Function Declarations
void initializeBLE();
void discoverAndConnect();
void handleUpdatedCharacteristics();
void printData(const unsigned char data[], int length);
void senddata(const unsigned char data[], int length);

WiFiSSLClient client;
String headers =
    "POST /app/data-bhiqc/endpoint/data/v1/action/insertOne HTTP/1.1\r\n"
    "Host: ap-southeast-2.aws.data.mongodb-api.com\r\n"
    "Content-Type: application/json\r\n"
    "api-key: dVKaMfJKdc2ISUivvB6T12fxjvMIi90t4vOsgGPAT7uaJsBGANz32UsG2EsWYjYX\r\n"
    "Content-Length: ";

// Define states for the state machine
enum State {
  STATE_IDLE,
  STATE_SCANNING,
  STATE_CONNECTING,
  STATE_CONNECTED
};

State currentState = STATE_IDLE;

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

  if (!client.connect(server, port)) {
    Serial.println("Connection failed!");
    while (1);
  }

  Serial.println("Step 1: Starting Service");
  initializeBLE();
}

void loop() {
  switch (currentState) {
    case STATE_IDLE:
      discoverAndConnect();
      break;

    case STATE_SCANNING:
      // Code for scanning and connecting to the BLE device...
      break;

    case STATE_CONNECTING:
      // Code for connecting to the BLE device...
      break;

    case STATE_CONNECTED:
      // Code for reading data from BLE and sending it to the database...
      handleUpdatedCharacteristics();
      break;
  }
}

void initializeBLE() {
  Serial.println("Step 2: Initializing BLE");

  if (!BLE.begin()) {
    Serial.println("Step 2: BLE failed!");
    while (1);
  }

  Serial.println("Step 3: BLE Successful");
  BLE.scanForUuid("180D");
}

void discoverAndConnect() {
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    Serial.println("Step 4: Scanning Device");
    if (peripheral.localName() == "TICKR 2EDC") {
      Serial.println("Step 5: Device Found");
      Serial.println("Step 6: Connecting ...");
      BLE.stopScan();

      if (peripheral.connect()) {
        Serial.println("Step 7: Connected To Sensor");

        if (peripheral.discoverAttributes()) {
          Serial.println("Step 8: Getting Data From Sensor");
          currentState = STATE_CONNECTED; // Transition to connected state
        } else {
          Serial.println("Step 8: Connecting to Data Stream Failed");
          peripheral.disconnect();
          currentState = STATE_IDLE; // Transition back to idle state
        }
      } else {
        Serial.println("Step 7: Connection Failed");
        currentState = STATE_IDLE; // Transition back to idle state
      }
    } else {
      Serial.println("Step 6: Sensor Not Found");
      currentState = STATE_IDLE; // Transition back to idle state
    }
  }
}

void handleUpdatedCharacteristics() {
  Serial.println("here");
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    BLEService service180D = peripheral.service("180D");
    if (service180D) {
      BLECharacteristic characteristic2A37 = service180D.characteristic("2A37");
      if (characteristic2A37) {
        characteristic2A37.subscribe();

        if (characteristic2A37.valueUpdated()) {
          const unsigned char *data = characteristic2A37.value();
          int length = characteristic2A37.valueLength();
          printData(data, length);
          senddata(data, length);
        }
      }
    }
  }
  else {
    return;
  }
}

void printData(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    if (i == 1) {
      unsigned char b = data[i];
      Serial.print("Heart Rate Value (BPM) = ");
      Serial.print(b);
      Serial.println();
    }
  }
}

unsigned char c; 
void senddata(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    if (i == 1) {
      c = data[i];
      Serial.print("Heart Rate Value (BPM) = ");
      Serial.print(c);
      Serial.println();
    }
  }


    String requestBody = 
    "{"
    "\"collection\":\"sensordata\","
    "\"database\":\"database\","
    "\"dataSource\":\"sit314\","
    "\"document\":{\"value\":" + String(c) + " }"
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

