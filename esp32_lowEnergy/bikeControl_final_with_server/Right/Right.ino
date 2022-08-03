#include <EEPROM.h>
#include <Arduino.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
/////////////
#include <ArduinoJson.h> // Using Version 5 of arduino json
#include "driver/adc.h"
#include <BLEDevice.h>

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
//////////////
const char* ssid = "microelab";
const char* ssidPassword = "microelab123";
//////////////////
String data1 = "a";
String data2 = "b";
String data3 = "c";
String data4 = "d";
///////////
int ledFlag;
boolean breakFlag;
boolean touchFlag;
boolean sendFlag;

boolean breakOn;
boolean sleepOff;

boolean breakPress;
boolean breakRelease;

boolean touchOn;
boolean touchOnSend;
boolean touchOffSend;

boolean touchDetected;
///////////////////////////////
void eepromSet(String name);
String eepromList();
String uuid_String;
////////////////
const char* SERVICE_UUID="4fafc201-1fb5-459e-8fcc-c5c9c331914b";// The remote service we wish to connect to.
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");// The characteristic of the remote service we are interested in.
/////////////////////////////////////////////////
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
//////////////////////////////////////////////////////////////
void reconnect();
/////////////////////////////////////////
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}
//////////////////////////////////////////////////////////
class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}
  void onDisconnect(BLEClient* pclient){connected = false;Serial.println("onDisconnect");
  reconnect();}
};
/////////-Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //Serial.print("BLE Advertised Device found: ");//Serial.println(advertisedDevice.toString().c_str());
    // We have found a device, let us now see if it contains the service we are looking for.
      BLEUUID serviceXUUID(SERVICE_UUID);////convert const char* to BLEUUID data type///Serial.println(serviceUUID.toString().c_str());
      if(advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceXUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;
      Serial.println("setp1");
    }// Found our server
  }// onResult
};// MyAdvertisedDeviceCallbacks
/////////////////////////////////////
#define Threshold 40 /* Greater the value, more the sensitivity */
RTC_DATA_ATTR int bootCount = 0;
touch_pad_t touchPin;
/////////////////////////////////////////////////
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
  //  case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO");break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL");breakFlag=HIGH;break;
  //  case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad");touchFlag=HIGH;touchDetected=HIGH; break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
////////////////////////////////////////////
void callback(){touchDetected =HIGH;}
//****************************************
String postValues(String url, String mac, String type);
String baseUrl = "http://192.168.100.247:8000";//network system IP or base url. CHANGE THIS
const size_t capacity = JSON_OBJECT_SIZE(3) + 110;
DynamicJsonBuffer jsonBuffer(capacity);
//****************************************
/////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  adc_power_off();
  //Serial.print("Boot Count:");Serial.println(bootCount);  
/////////////-EEPROM///////////////////////////
if(!EEPROM.begin(50)){Serial.println("failed to initialise EEPROM");}
//////////////////////////////  
 if(bootCount>0){Serial.println("Do Nothing...");}
 else
 { 
  ////////////////////////////////////////////////////// 
  WiFi.begin(ssid, ssidPassword);             // Connect to the network
  Serial.print("Connecting to ");
  Serial.print(ssid);
  int wifiBreak=0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(500);
    Serial.print('.');
    wifiBreak++;if(wifiBreak==5){Serial.println("....");break;}
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address      : ");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
  //Method 1: Ge's Information after device is added
  String mac_address = WiFi.macAddress();
  Serial.print("Device MAC      : ");Serial.println(mac_address);
  Serial.print("Server Responce : ");
  String device_type = "S"; // Define M for Master and S for Slave
  String url = baseUrl + "/api/bluetooth/add";
  //////-wait for WiFi connection
  String output = postValues(url, mac_address, device_type); //Output for Method 1 here.
  while(output == "\nresponse empty"){output = postValues(url, mac_address, device_type);}
  //   Method 2: Only Get uuid from MAc Address if device is already added.
  url = baseUrl + "/api/bluetooth/group";
  String output2 = postValues (url, mac_address, device_type); // = postValues (url, mac_address, "");
  while(output2 == "\nresponse empty"){output2 = postValues(url, mac_address, device_type);}
  
  JsonObject& root = jsonBuffer.parseObject(output2);
  const char* master_mac_address = root["master_mac_address"]; // "3C:71:BF:10:75:00"
  const char* devices            = root["devices"]; // "[11]"
  const char* uuid               = root["uuid"]; // "05c44d5f-643c-3fca-bf47-aba7ae311d1f"
  SERVICE_UUID = uuid;///**********************
  Serial.print("Service UUID from Server: "); Serial.println(SERVICE_UUID);//************************
  eepromSet(String(SERVICE_UUID));////write UUID on EEPROM
}
++bootCount;
///////////////////////////////////////////////////////////////////////////////////  
  Serial.println("Starting Arduino BLE Client application...");
//  BLEUUID:BLEUUID(*uuid); //
  BLEDevice::init("");  

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);///1349
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  //////////////////////////////////
  print_wakeup_reason();
  touchAttachInterrupt(T3, callback, Threshold);  //Setup interrupt on Touch Pad 3 (GPIO15)
  esp_sleep_enable_touchpad_wakeup();  //Configure Touchpad as wakeup source
///////////////////////////////////////////////////////////////////////////////////
//Configure GPIO33 as ext1 wake up source for HIGH logic level
 esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);
//////////////////////////////////////////////////////////////////////////////////
pinMode(33,INPUT);
Serial.println(">>>>>RIGHT Side:");
} // End of setup.
/////////////////////////////////////////////////
bool connectToServer() {
//////////////////////////////  
uuid_String=eepromList();/////-read EEPROM
char s[uuid_String.length()+1] ;//////create array
uuid_String.toCharArray(s,uuid_String.length()+1);//Converts String into character array
SERVICE_UUID=s;
//////////////////////////////
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallback());
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);//**
/////////////////////////////////////////////////////////////    
    if (pRemoteService == nullptr){
      Serial.print("Failed to find our service UUID: ");
      Serial.println(SERVICE_UUID);
      pClient->disconnect();
      return false;
    }
  //Serial.println(" - Found our service");
/////////////////////////////////////////////////
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    //Serial.println(" - Found our characteristic");
    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()){
      std::string value = pRemoteCharacteristic->readValue();
      //Serial.print("The characteristic value was: ");
      //Serial.println(value.c_str());
    }
    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}
void loop(){
if(breakFlag==HIGH || touchFlag==HIGH){sleepOff=HIGH;/*Serial.println("awak...........");*/
if(breakOn==HIGH){if(digitalRead(33)==LOW){Serial.println(">>>>>RIGHT Break Release:");breakRelease=HIGH;breakOn=LOW;breakOn=LOW;}} else
if(breakOn==LOW){if(digitalRead(33)==HIGH){Serial.println(">>>>>RIGHT Break Press:");breakPress=HIGH;breakOn=HIGH;}}
//////////////////////
if(touchOn==HIGH){if(touchDetected==HIGH){Serial.println(">>>>>RIGHT Indicator OFF:");touchOffSend=HIGH;touchDetected=LOW;touchOn=LOW;}}else 
if(touchOn==LOW){if(touchDetected==HIGH){Serial.println(">>>>>RIGHT Indicator ON :");touchOnSend=HIGH;touchDetected=LOW;}}
}
///////////////////////////////
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if(doConnect == true){
  if(connectToServer()){Serial.println("We are now connected to the BLE Server.");}
  else{Serial.println("We have failed to connect to the server; there is nothin more we will do.");}
  doConnect = false;}
  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected){
 if(touchOnSend==HIGH){touchOnSend=LOW;touchOn=HIGH;pRemoteCharacteristic->writeValue(data1.c_str(),data1.length());delay(1000);touchDetected=LOW;}
 if(touchOffSend==HIGH){touchOffSend=LOW;touchOn=LOW;pRemoteCharacteristic->writeValue(data2.c_str(),data2.length());delay(1000);touchDetected=LOW;}
 if(breakPress==HIGH){breakPress=LOW;pRemoteCharacteristic->writeValue(data3.c_str(),data3.length());}
 if(breakRelease==HIGH){breakRelease=LOW;pRemoteCharacteristic->writeValue(data4.c_str(),data4.length());}
////////////Serial.print("Boot Time: ");Serial.println(millis());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
}
////////////////////////////////////
if(sleepOff==LOW){
 Serial.print("TIME: ");Serial.print( millis());Serial.println("  Mili second for wakeup");
 Serial.println("Going to sleep now");  
 esp_deep_sleep_start();
}
if(breakOn==LOW && touchOn==LOW){
  breakFlag=LOW;touchFlag=LOW;
  sleepOff=LOW;
}
} // End of loop

void reconnect(){
  Serial.println("ReConnect");
   BLEScan* pBLEScan = BLEDevice::getScan();
   pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
   pBLEScan->setActiveScan(true);
   pBLEScan->start(40);
}
///////////////////////////
String postValues(String url, String mac, String type)
{
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
/////////////////////////////////////////
void eepromSet(String name){
  for (int i = 0; i <name.length(); ++i){
    EEPROM.write(i,name.charAt(i));
  }
  EEPROM.commit();
  Serial.print("WRITE EEPROM: ");Serial.println(name);
}
/////////////////////////
String eepromList(){
  char letter;
  String list="";
  for (int i = 0; i <36; ++i){letter= char(EEPROM.read(i));list+=letter;}
Serial.print("READ EEPROM: ");Serial.println(list);
  return list;}
