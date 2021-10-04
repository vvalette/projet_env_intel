#include <Arduino.h>

// For the screen
#include <TFT_eSPI.h>

// For bluetooth
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

//For MQTT 
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "Valentin's Galaxy S21 5G";
const char* password = "1234567890";
//const char* ssid = "B127-EIC";
//const char* password = "b127-eic";
// Add your MQTT Broker IP address, example:
const char* mqtt_server = "broker.mqttdashboard.com";
//const char* mqtt_server = "192.168.1.105";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//uncomment the following lines if you're using SPI
#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("test_channel");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int scanTime = 1.5; //In seconds
BLEScan *pBLEScan;

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h

// Result of ble mi temp
class BLEResult
{
public:
  double temperature = -200.0f;
  double humidity = -1.0f;
  int16_t battery_level = -1;
};

BLEResult result;

void displayResult()
{
  tft.fillScreen(TFT_BLACK);
   tft.setCursor(100, 60);
    tft.println("HOME");

  if (result.temperature > -200.0f)
  {
    Serial.printf("temperature: %.2f", result.temperature);
    Serial.println();    
    
    tft.setCursor(80, 90);
    tft.println( "T:");
    tft.setCursor(110, 90);
    tft.println(result.temperature);
  }
  if (result.humidity > -1.0f)
  {
    Serial.printf("humidity: %.2f", result.humidity);
    Serial.println();

    tft.setCursor(80, 120);
    tft.println( "H:");
    tft.setCursor(110, 120);
    tft.println(result.humidity);
  }
  if (result.battery_level > -1)
  {
    Serial.printf("battery_level: %d", result.battery_level);
    Serial.println();

    tft.setCursor(80, 150);
    tft.println( "Power:");
    tft.setCursor(150, 150);
    tft.println(result.battery_level);
  }
  if (true){tft.setCursor(65, 200);tft.println( "Connected");}
  else{tft.setCursor(65, 200);tft.println( "Disconnected");}
}



// Callback when find device ble
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    BLEAddress address = advertisedDevice.getAddress();

    // Filter by mac address of mi temp
    if (address.toString() == "58:2d:34:3b:7d:3c")
    {

      uint8_t *payloadRaw = advertisedDevice.getPayload();
      size_t payloadLength = advertisedDevice.getPayloadLength();

      // For each data of ble advertise
      for (int i = 0; i < payloadLength; i++)
      {
        // Show the data
        Serial.printf("%02X ", payloadRaw[i]);

        // Need min 3 char to start to check
        if (i > 3)
        {
          uint8_t raw = payloadRaw[i - 3];     // type
          uint8_t check = payloadRaw[i - 2];   // must always be 0x10
          int data_length = payloadRaw[i - 1]; // length of data

          if (check == 0x10)
          {
            // temperature, 2 bytes, 16-bit signed integer (LE), 0.1 °C
            if ((raw == 0x04) && (data_length == 2) && (i + data_length < payloadLength))
            {
              const int16_t temperature = uint16_t(payloadRaw[i + 0]) | (uint16_t(payloadRaw[i + 1]) << 8);
              result.temperature = temperature / 10.0f;
            }
            // humidity, 2 bytes, 16-bit signed integer (LE), 0.1 %
            else if ((raw == 0x06) && (data_length == 2) && (i + data_length < payloadLength))
            {
              const int16_t humidity = uint16_t(payloadRaw[i + 0]) | (uint16_t(payloadRaw[i + 1]) << 8);
              result.humidity = humidity / 10.0f;
            }
            // battery, 1 byte, 8-bit unsigned integer, 1 %
            else if ((raw == 0x0A) && (data_length == 1) && (i + data_length < payloadLength))
            {
              result.battery_level = payloadRaw[i + 0];
            }
            // temperature + humidity, 4 bytes, 16-bit signed integer (LE) each, 0.1 °C, 0.1 %
            else if ((raw == 0x0D) && (data_length == 4) && (i + data_length < payloadLength))
            {
              const int16_t temperature = uint16_t(payloadRaw[i + 0]) | (uint16_t(payloadRaw[i + 1]) << 8);
              const int16_t humidity = uint16_t(payloadRaw[i + 2]) | (uint16_t(payloadRaw[i + 3]) << 8);
              result.temperature = temperature / 10.0f;
              result.humidity = humidity / 10.0f;
            }
          }
        }
      }

      displayResult();
    }
  }
};

void initScreen(){
  tft.begin();
  tft.setRotation(1);
  tft.setTextSize(2);
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(60, 80);
  tft.println("Project by");
  tft.setCursor(75, 115);
  tft.println("Valentin");
  tft.setCursor(75, 150);
  tft.println("and Aykel");
}

void setup()
{
  Serial.begin(9600);
  
  // Initialise BLE scan
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  // Initialise screen
  initScreen();
  delay(1000);                

  // INitialise wifi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}
char temp[10], hum[10], bat[10];

void loop()
{

  if (!client.connected()) {
    reconnect();
  }

  long now = millis();
  if (now - lastMsg > 500) {
    lastMsg = now;
    
    //Envoi de la température
    sprintf(temp, "%.2f", result.temperature); 
    client.publish("sensor_data/temperature", temp);

    //Envoi de l'humidité'
    sprintf(hum, "%.2f", result.humidity); 
    client.publish("sensor_data/humidity", hum);

    //Envoi de la température
    sprintf(bat, "%d", result.battery_level); 
    client.publish("sensor_data/battery_level", bat);

    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    pBLEScan->clearResults();
  }

}
