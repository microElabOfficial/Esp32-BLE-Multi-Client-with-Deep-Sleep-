#include "BLEDevice.h"
//#include "BLEScan.h"

String data1 = "a";
String data2 = "b";

int ledFlag;

static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");// The remote service we wish to connect to.
static BLEUUID    charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");// The characteristic of the remote service we are interested in.
/////////////////////////////////////////////////
static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;
//////////////////////////////////////////////////////////////
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

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}
  void onDisconnect(BLEClient* pclient){connected = false;Serial.println("onDisconnect");}
};
bool connectToServer() {
 //   Serial.print("Forming a connection to ");
    //Serial.println(myDevice->getAddress().toString().c_str());
    BLEClient*  pClient  = BLEDevice::createClient();
//    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
//    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
  //  Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    //Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      //Serial.print("The characteristic value was: ");
      //Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //Serial.print("BLE Advertised Device found: ");
    //Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks
///////////////////////
#define Threshold 40 /* Greater the value, more the sensitivity */
RTC_DATA_ATTR int bootCount = 0;
touch_pad_t touchPin;
/////////////////////////////////////////////////

void print_wakeup_touchpad(){
  touch_pad_t pin;
  touchPin = esp_sleep_get_touchpad_wakeup_status();
  //Serial.println(":");
  Serial.println("");
  Serial.println("");
  Serial.println("//////////////////////////////////");

switch(touchPin){
    case 0  : Serial.println("LED ON"); 
    ledFlag=1;
    break;
    case 3  : Serial.println("LED OFF"); 
    ledFlag=2;   
    break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }
}








void callback(){
  //placeholder callback function
}
void setup() {
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Client application...");
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

//touchAttachInterrupt(T0, gotTouch1, threshold);
//touchAttachInterrupt(T4, gotTouch2, threshold);



} // End of setup.


// This is the Arduino main loop function.
void loop() {
 print_wakeup_touchpad();
  //Setup interrupt on Touch Pad 0 (GPIO4)
  touchAttachInterrupt(T0, callback, Threshold);
  //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(T3, callback, Threshold);
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected){


 if(ledFlag==1)
 {
 // Serial.print("---a");
  pRemoteCharacteristic->writeValue(data1.c_str(),data1.length());  
 }
 if(ledFlag==2)
 {
 // Serial.print("---b");
  pRemoteCharacteristic->writeValue(data2.c_str(),data2.length());  
 }

ledFlag=0; 
    /*String data1 = "a";
    String data2 = "b";
    if(touch1detected){
    touch1detected = false;
    Serial.println("Touch 1 detected");  
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(data1.c_str(),data1.length());

  }
  if(touch2detected){
    touch2detected = false;
    Serial.println("Touch 2 detected");
  
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(data2.c_str(), data2.length());

  }
  */  
    //Serial.print("Boot Time: ");Serial.println(millis());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }




 Serial.print("TIME: ");Serial.print( millis());Serial.println("  Mili second for wakeup");
 Serial.println("Going to sleep now");  
  esp_deep_sleep_start();
  } // End of loop
