#include <EEPROM.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <HTTPClient.h>
/////////////
#include <ArduinoJson.h> // Using Version 5 of arduino json
#include <BLEDevice.h>/////This library provides an implementation Bluetooth Low Energy support for the ESP32 using the Arduino platform.
#include <BLEUtils.h>////This library provides GATT and Gap servies.
#include <BLEServer.h>///For BLE Server


const char* ssid = "microelab";
const char* ssidPassword = "microelab123";


uint32_t int16_uuid;

const int rightLight=25;
const int breakLight=26;
const int frontLight=27;
const int leftLight=14;


boolean rightBreakReleaseFlag;
boolean rightBreakPressFlag;
boolean rightLightFlag;


boolean leftBreakReleaseFlag;
boolean leftBreakPressFlag;
boolean leftLightFlag;

RTC_DATA_ATTR int bootCount = 0;


void eepromSet(String name);
String eepromList();
String uuid_String;
/////////-Universally unique identifier
const char* SERVICE_UUID="4fafc201-1fb5-459e-8fcc-c5c9c331914b";// The remote service we wish to connect to.
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
///////////////////////////
const int led=2;////-builtin LED 

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
 //       Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
{
//         Serial.print(value[i]);
        //////////////client1 is right
        if(value[i]=='a'){rightLightFlag=HIGH;}
        if(value[i]=='b'){rightLightFlag=LOW;}
        if(value[i]=='c'){rightBreakPressFlag=HIGH;}
        if(value[i]=='d'){rightBreakReleaseFlag=HIGH;}

        if(value[i]=='e'){leftLightFlag=HIGH;}
        if(value[i]=='f'){leftLightFlag=LOW;}
        if(value[i]=='g'){leftBreakPressFlag=HIGH;}
        if(value[i]=='h'){leftBreakReleaseFlag=HIGH;}



        
        
}       Serial.println();
        Serial.println("*********");
      }
    }
};

//****************************************
String postValues(String url, String mac, String type);
String baseUrl = "http://192.168.100.247:8000";//network system IP or base url. CHANGE THIS
const size_t capacity = JSON_OBJECT_SIZE(3) + 110;
DynamicJsonBuffer jsonBuffer(capacity);
//String uuid;

//****************************************
void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
////////////////////////////////////////////
if(!EEPROM.begin(50)){Serial.println("failed to initialise EEPROM");}

pinMode(rightLight,OUTPUT);
pinMode(breakLight,OUTPUT);
pinMode(frontLight,OUTPUT);
pinMode(leftLight,OUTPUT);

  if (bootCount > 0) {Serial.println("Do Nothing...");}
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
    wifiBreak++;if(wifiBreak==5){wifiBreak=0;Serial.println("....");break;}}

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address      : ");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
//////////////////////////
  //Method 1: Ge's Information after device is added
  String mac_address = WiFi.macAddress();
  Serial.print("Device MAC      : ");Serial.println(mac_address);
  Serial.print("Server Responce : ");
  String device_type = "M"; // Define M for Master and S for Slave
  String url = baseUrl + "/api/bluetooth/add";
  // wait for WiFi connection
  String output = postValues(url, mac_address, device_type); //Output for Method 1 here.
  while(output == "\nresponse empty"){
    output = postValues(url, mac_address, device_type); 
    }
  //   Method 2: Only Get uuid from MAc Address if device is already added.
  url = baseUrl + "/api/bluetooth/group";
  String output2 = postValues (url, mac_address, device_type);
  while(output2 == "\nresponse empty"){
    output2 = postValues(url, mac_address, device_type); 
    }
  JsonObject& root = jsonBuffer.parseObject(output2);
  const char* master_mac_address    = root["master_mac_address"]; // "3C:71:BF:10:75:00"
  const char* devices               = root["devices"]; // "[11]"
  const char* uuid                  = root["uuid"];
  //uuid = String((const char*) root["uuid"]); 
  SERVICE_UUID=uuid;//************************
  Serial.print("Service Device UUID from Server: ");Serial.println(SERVICE_UUID);
  eepromSet(String(SERVICE_UUID));////write UUID on EEPROM
}
++bootCount;

///////////////////////////////////////////////////////////////////////////////////
 //BLEUUID SERVICE_UUID( (const char*)root["uuid"]); //*****************
 /// BLEUUID SERVICE_UUID(SERVICE_UUID); //*****************
uuid_String=eepromList();/////-read EEPROM
char s[uuid_String.length()+1] ;//////create array
uuid_String.toCharArray(s,uuid_String.length()+1);//Converts String into character array
SERVICE_UUID=s;

  BLEDevice::init("SERVER");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,//serviceuuid,//CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Bike Indicators");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();
  Serial.println(" ");
  Serial.println("Characteristic defined");
  Serial.println("*************************************");

  //Serial.println(BLEUUID::toString());
}




void loop(){
 if(rightLightFlag==HIGH){Serial.println("Right indicator ON");digitalWrite(rightLight,!digitalRead(rightLight));digitalWrite(frontLight,LOW);} 
 if(rightLightFlag==LOW){digitalWrite(rightLight,LOW);} 


 if(leftLightFlag==HIGH){Serial.println("Left indicator ON");digitalWrite(leftLight,!digitalRead(leftLight));digitalWrite(frontLight,LOW);} 
 if(leftLightFlag==LOW){digitalWrite(leftLight,LOW);} 

 
 if(rightBreakPressFlag==HIGH) {Serial.println("Right Break Press");  digitalWrite(breakLight,HIGH);digitalWrite(frontLight,LOW);}
 if(leftBreakPressFlag==HIGH)  {Serial.println("Left Break Press");  digitalWrite(breakLight,HIGH);digitalWrite(frontLight,LOW);}
 
 if(rightBreakReleaseFlag==HIGH){Serial.println("Right Break Release");digitalWrite(breakLight,LOW);rightBreakReleaseFlag=LOW;rightBreakPressFlag=LOW;}
 if(leftBreakReleaseFlag==HIGH){Serial.println("Left Break Release");digitalWrite(breakLight,LOW);leftBreakReleaseFlag=LOW;leftBreakPressFlag=LOW;}

   
  delay(500);
  digitalWrite(frontLight,!digitalRead(frontLight));
//////////////////////




//////////////////////
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
  return list;}
