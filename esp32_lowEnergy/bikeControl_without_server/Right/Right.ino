#include "BLEDevice.h"

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex

String data1 = "a";
String data2 = "b";
String data3 = "c";
String data4 = "d";


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


static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");// The remote service we wish to connect to.
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

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}
  void onDisconnect(BLEClient* pclient){connected = false;Serial.println("onDisconnect");
  reconnect();}
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
void callback(){
  //placeholder callback function

touchDetected =HIGH;

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


// This is the Arduino main loop function.
void loop(){
if(breakFlag==HIGH || touchFlag==HIGH){sleepOff=HIGH;Serial.println("awak...........");
if(breakOn==HIGH){if(digitalRead(33)==LOW){Serial.println(">>>>>RIGHT Break Release:");breakRelease=HIGH;breakOn=LOW;breakOn=LOW;}} else
if(breakOn==LOW){if(digitalRead(33)==HIGH){Serial.println(">>>>>RIGHT Break Press:");breakPress=HIGH;breakOn=HIGH;}}
//////////////////////
if(touchOn==HIGH){if(touchDetected==HIGH){Serial.println(">>>>>RIGHT Indicator OFF:");touchOffSend=HIGH;touchDetected=LOW;touchOn=LOW;}}else 
if(touchOn==LOW){if(touchDetected==HIGH){Serial.println(">>>>>RIGHT Indicator ON :");touchOnSend=HIGH;touchDetected=LOW;}}
}


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



  
    //Serial.print("Boot Time: ");Serial.println(millis());
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }


if(sleepOff==LOW){
 Serial.print("TIME: ");Serial.print( millis());Serial.println("  Mili second for wakeup");
 Serial.println("Going to sleep now");  
 esp_deep_sleep_start();
}
if(breakOn==LOW && touchOn==LOW)
{
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

