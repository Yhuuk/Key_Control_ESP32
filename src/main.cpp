#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define LED_PIN 26

// 设备名：小程序会根据这个名字搜索
static const char *DEVICE_NAME = "ESP32_LOCK_001";

// BLE UUID：小程序端必须和这里完全一致
static const char *SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";
static const char *WRITE_UUID   = "0000fff1-0000-1000-8000-00805f9b34fb";

volatile bool openRequested = false;

bool pulseActive = false;
unsigned long pulseStartMs = 0;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) override {
    Serial.println("[BLE] Client connected");
  }

  void onDisconnect(BLEServer *pServer) override {
    Serial.println("[BLE] Client disconnected");
    BLEDevice::startAdvertising();
    Serial.println("[BLE] Advertising restarted");
  }
};

class MyWriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    std::string value = pCharacteristic->getValue();

    Serial.print("[BLE] Received: ");
    for (size_t i = 0; i < value.length(); i++) {
      Serial.print(value[i]);
    }
    Serial.println();

    String cmd = String(value.c_str());
    cmd.trim();

    if (cmd == "OPEN") {
      openRequested = true;
      Serial.println("[GPIO] OPEN command accepted");
    } else {
      Serial.println("[GPIO] Unknown command");
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println();
  Serial.println("===== ESP32 BLE GPIO2 Demo =====");

  BLEDevice::init(DEVICE_NAME);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pWriteCharacteristic = pService->createCharacteristic(
    WRITE_UUID,
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
  );

  pWriteCharacteristic->setCallbacks(new MyWriteCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  Serial.println("[BLE] Device name: ESP32_LOCK_001");
  Serial.println("[BLE] Service UUID: 0000fff0-0000-1000-8000-00805f9b34fb");
  Serial.println("[BLE] Write UUID:   0000fff1-0000-1000-8000-00805f9b34fb");
  Serial.println("[BLE] Advertising...");
}

void loop() {
  if (openRequested) {
    openRequested = false;

    digitalWrite(LED_PIN, HIGH);
    pulseActive = true;
    pulseStartMs = millis();

    Serial.println("[GPIO] GPIO2 HIGH");
  }

  if (pulseActive && millis() - pulseStartMs >= 1000) {
    pulseActive = false;
    digitalWrite(LED_PIN, LOW);

    Serial.println("[GPIO] GPIO2 LOW");
  }

  delay(10);
}