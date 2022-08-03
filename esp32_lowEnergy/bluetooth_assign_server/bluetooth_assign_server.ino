//#include <SoftwareSerial.h>
//SoftwareSerial swSer(14,12, false, 256);
#include <Arduino.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Using Version 5 of arduino json

#define USE_SERIAL Serial

const char* ssid = "microelab";
const char* ssidPassword = "microelab123";

//const char* ssid = "509 Pak Block";
//const char* ssidPassword = "IKnowSomethings12";

String postValues(String url, String mac, String type);
void setup() {

  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  WiFi.begin(ssid, ssidPassword);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
}


String baseUrl = "http://192.168.100.247:8000";//network system IP or base url. CHANGE THISS

const size_t capacity = JSON_OBJECT_SIZE(3) + 110;
DynamicJsonBuffer jsonBuffer(capacity);

void loop() {
  //Method 1: Ge's Information after device is added
  String mac_address = WiFi.macAddress();
  Serial.print(mac_address);
  String device_type = "M"; // Define M for Master and S for Slave
  String url = baseUrl + "/api/bluetooth/add";
  // wait for WiFi connection
  String output = postValues(url, mac_address, device_type); //Output for Method 1 here.

  //   Method 2: Only Get uuid from MAc Address if device is already added.
  url = baseUrl + "/api/bluetooth/group";
  String output2 = postValues (url, mac_address, device_type);
  JsonObject& root = jsonBuffer.parseObject(output2);
  const char* master_mac_address = root["master_mac_address"]; // "3C:71:BF:10:75:00"
  const char* devices = root["devices"]; // "[11]"
  const char* uuid = root["uuid"]; // "05c44d5f-643c-3fca-bf47-aba7ae311d1f"
  Serial.print(uuid);
  delay(10000);


}

/////////////////
String postValues(String url, String mac, String type) {


  HTTPClient http;
  http.begin(url); //HTTP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");//Specify content-type header
  http.addHeader("Accept", "application/json");                       //Specify content-type header

  String values = "";
  if (type.length() == 1) {
    values = "macAddress=" + mac + "&mode=" + type;
  } else {
    values = "macAddress=" + mac;
  }
  //  Serial.println(values);
  int httpCode = http.POST(values);
  String payload = "\nresponse empty";
  if (httpCode == 200) {
    payload = http.getString();
    //      USE_SERIAL.println(payload);
    http.end();
  }
  Serial.println(payload);
  return payload;

}

