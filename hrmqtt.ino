#include <ArduinoBLE.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>

// WiFi and MQTT settings
char ssid[] = "YourSSID";
char pass[] = "YourPassword";
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "broker.hivemq.com";
int port = 1883;
const char topic[] = "/sensor_data";

// Function Declarations
void initializeBLE();
void discoverAndConnect();
void handleUpdatedCharacteristics();
void printData(const unsigned char data[], int length);

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize BLE
  Serial.println("Step 1: Starting Service");
  initializeBLE();

  // Initialize WiFi and MQTT
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
}

void loop() {
  discoverAndConnect();
  BLE.poll();  // Poll for BLE events and communication updates.
  mqttClient.poll(); // Poll for MQTT messages
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

          BLEService service180D = peripheral.service("180D");
          if (service180D) {
            BLECharacteristic characteristic2A37 = service180D.characteristic("2A37");
            if (characteristic2A37) {
              characteristic2A37.subscribe();
              Serial.println("Step 9: Connected to Data Stream of Sensor. Started Fetching Data ....");
              while (true) {
                peripheral.poll();
                if (characteristic2A37.valueUpdated()) {
                  printData(characteristic2A37.value(), characteristic2A37.valueLength());
                }
              }
            }
          }
        } else {
          Serial.println("Step 8: Connecting to Data Stream Failed");
          peripheral.disconnect();
          return;
        }
      } else {
        Serial.println("Step 7: Connection Failed");
        return;
      }
    } else {
      Serial.println("Step 6: Sensor Not Found");
    }
  }
}

void printData(const unsigned char data[], int length) {
  if (length >= 2) {
    unsigned char bpm = data[1];
    Serial.print("Heart Rate Value (BPM) = ");
    Serial.println(bpm);
    String message = "Heart Rate: " + String(bpm);

    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.println(message);

    mqttClient.beginMessage(topic);
    mqttClient.print(message);
    mqttClient.endMessage();

    Serial.println();
  }
}
