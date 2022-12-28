/*
    @Ref: File > Examples > ESP32 BLE Arduino > BLE_server
    @Ref: https://medium.com/@jalltechlab/%E5%88%A9%E7%94%A8esp32-%E7%AF%84%E4%BE%8B%E9%80%B2%E8%A1%8Cbluetooth-ble-%E5%BB%A3%E6%92%AD-%E7%AC%AC%E4%BA%8C%E7%AF%80-ab0e1a4d5481
    @Ref: File > Examples > M5ATOM > Basics > LEDDisplay
    @Ref: https://docs.m5stack.com/en/api/atom/system
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "M5Atom.h"

#define SERVICE_UUID_BEATS            "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_BEATS     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SERVICE_UUID_OXYGEN           "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define CHARACTERISTIC_UUID_OXYGEN    "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define SERVICE_UUID_EMERGENCY        "4fafc201-1fb5-459e-8fcc-c5c9c331914d"
#define CHARACTERISTIC_UUID_EMERGENCY "beb5483e-36e1-4688-b7f5-ea07361b26aa"

// Global vriable
BLEServer *pServer;

BLEService *pService_beats;
BLEAdvertising *pAdvertising_beats;
BLECharacteristic *pCharacteristic_beats;

BLEService *pService_oxygen;
BLEAdvertising *pAdvertising_oxygen;
BLECharacteristic *pCharacteristic_oxygen;

BLEService *pService_emergency;
BLEAdvertising *pAdvertising_emergency;
BLECharacteristic *pCharacteristic_emergency;

String data;
int beats = 120;
int oxygen = 99;
int emergency = 0;
unsigned long long now_time = -1;
unsigned long long pre_time_2000 = 0;
unsigned long long pre_time_150 = 0;
int count = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

extern const unsigned char
    AtomImageData[375 + 2];  // External reference stores the array of images shown.  外部引用存储所示图像的数组

bool IMU6886Flag = false;  // Close IMU6886.  关闭IMU6886

void setup() {
  M5.begin(true, false,true);  // Init Atom-Matrix(Initialize serial port, LED matrix).
  delay(50);

  //Serial.begin(115200);
  Serial.println("Start...");

  BLEDevice::init("worker-a wrist");
  pServer = BLEDevice::createServer();
  //-----beats-----
  pService_beats = pServer->createService(SERVICE_UUID_BEATS);
  pCharacteristic_beats = pService_beats->createCharacteristic(CHARACTERISTIC_UUID_BEATS,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_beats->setValue("0");
  pService_beats->start();

  pAdvertising_beats = BLEDevice::getAdvertising();
  pAdvertising_beats->addServiceUUID(SERVICE_UUID_BEATS);
  pAdvertising_beats->setScanResponse(true);
  pAdvertising_beats->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising_beats->setMinPreferred(0x12);

  //-----oxygen-----
  pService_oxygen = pServer->createService(SERVICE_UUID_OXYGEN);
  pCharacteristic_oxygen = pService_oxygen->createCharacteristic(CHARACTERISTIC_UUID_OXYGEN,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_oxygen->setValue("0");
  pService_oxygen->start();

  pAdvertising_oxygen = BLEDevice::getAdvertising();
  pAdvertising_oxygen->addServiceUUID(SERVICE_UUID_OXYGEN);
  pAdvertising_oxygen->setScanResponse(true);
  pAdvertising_oxygen->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising_oxygen->setMinPreferred(0x12);

  //-----emergency-----
  pService_emergency = pServer->createService(SERVICE_UUID_EMERGENCY);
  pCharacteristic_emergency = pService_emergency->createCharacteristic(CHARACTERISTIC_UUID_EMERGENCY,
                        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic_emergency->setValue("0");
  pService_emergency->start();

  pAdvertising_emergency = BLEDevice::getAdvertising();
  pAdvertising_emergency->addServiceUUID(SERVICE_UUID_EMERGENCY);
  pAdvertising_emergency->setScanResponse(true);
  pAdvertising_emergency->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising_emergency->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("Ready!");
  M5.dis.setWidthHeight(25, 5);  // Set the width and height of the display pattern.  设置显示图案的宽高
}

void loop() {
  now_time = millis();
  if(now_time - pre_time_150 > 150){
    emergency = M5.Btn.read();
    pCharacteristic_emergency->setValue(emergency);
    if(emergency){
      M5.dis.fillpix(0xff0000);
    }else{
      M5.dis.displaybuff((uint8_t *)AtomImageData, -1*count, 0);
      count++;
      if(count >= 25) count = 0;
    }
    pre_time_150 = now_time;
  }

  if(now_time - pre_time_2000 > 2000){
    beats = random(110, 130);
    oxygen = random(95, 99);
    pCharacteristic_beats->setValue(beats);
    pCharacteristic_oxygen->setValue(oxygen);
    Serial.println(millis());
    pre_time_2000 = now_time;
  }
}