#include <Arduino.h>
#include <NimBLEDevice.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID        "1818"  // Cycling Power Service (CPS) UUID
#define CHARACTERISTIC_UUID "2A63"  // Cycling Power Measurement UUID

class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device Connected!");
        // Ensure advertising stops once connected
        NimBLEDevice::getAdvertising()->stop();
    }
    
    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device Disconnected. Restarting advertisement...");
        // Restart advertising when disconnected
        NimBLEDevice::getAdvertising()->start();
    }
};

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("Initializing Power Meter...");

    NimBLEDevice::init("Cycling Power Meter");  
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);  

    // Create BLE Server
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    // Start the service
    pService->start();

    // Set up advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);

    // Set scan response data (advertise the device name)
    NimBLEAdvertisementData advertisementData;
    advertisementData.setName("Cycling Power Meter");
    pAdvertising->setScanResponseData(advertisementData);

    // Start advertising
    pAdvertising->start();
    Serial.println("Waiting for a client connection to notify...");
}

void loop() {
    // Check if device is connected or not
    if (!deviceConnected) {
        Serial.println("Device not connected. Restarting advertisement...");
        NimBLEDevice::getAdvertising()->start();
        delay(500);  // Wait for a client to connect
    } else {
        // When device is connected, send power data
        uint16_t power = random(100, 400);
        Serial.println("Device is connected. Sending power data...");
        Serial.print("Generated Power: ");
        Serial.println(power);

        // Flags (2 bytes)
        uint8_t flags[2] = { 0x00, 0x00 };

        // Power value (2 bytes) - little-endian format
        uint8_t powerData[2] = { lowByte(power), highByte(power) };

        // Combine into a single array
        uint8_t cpmData[4];
        memcpy(cpmData, flags, 2);
        memcpy(cpmData + 2, powerData, 2);

        // Send Cycling Power Measurement
        pCharacteristic->setValue(cpmData, sizeof(cpmData));
        pCharacteristic->notify();

        delay(500);  // Wait before next power update
    }
}
