 #include <BLEDevice.h>/////This library provides an implementation Bluetooth Low Energy support for the ESP32 using the Arduino platform.
//#include <BLEUtils.h>////This library provides GATT and Gap servies.
//#include <BLEServer.h>///For BLE Server


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


/////////-Universally unique identifier
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
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

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

pinMode(rightLight,OUTPUT);
pinMode(breakLight,OUTPUT);
pinMode(frontLight,OUTPUT);
pinMode(leftLight, OUTPUT);


///////////////////

  BLEDevice::init("Server");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
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
}
