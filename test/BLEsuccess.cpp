#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteritic;
bool deviceConnected = false;
int txValue = 0;

#define SERVICE_UUID        "1818"  // Cycling Power Service (CPS) UUID
#define CHARACTERISTIC_UUID "2A63"  // Cycling Power Measurement UUID

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        if (Serial) {
          Serial.println("Device Connected!");  // This should print when connection is established
        }
    }
    
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        pServer->getAdvertising()->start();
        if (Serial){
        Serial.println("Device Disconnected. Restart advertizing");
        } 
    }
};

void setup() {
    Serial.begin(115200);
    // while (!Serial);  // Wait for Serial Monitor to open
    delay(500);
    if(Serial){
    Serial.println("Initializing Power Meter...");
    }

    BLEDevice::init("Cycling Power Meter");  // Set device name
    BLEDevice::setPower(ESP_PWR_LVL_P9);     // Max BLE power (+9 dBm)

    // // ✅ Enable BLE 5.0 PHY (Fast 2M Mode)
    // BLEDevice::setPreferredPHY(BLE_PHY_2M, BLE_PHY_2M, BLE_PHY_2M);


    // Create BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

    // Create BLE Characteristic for Cycling Power Measurement
    pCharacteritic = pService->createCharacteristic(
        BLEUUID(CHARACTERISTIC_UUID),
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteritic->addDescriptor(new BLE2902());
    // Start the service
    pService->start();

        // ✅ Update advertising to include Cycling Power Service UUID
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(BLEUUID(SERVICE_UUID));  // Make sure Coros sees the service
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Reduces reconnection delay
    pAdvertising->setMinPreferred(0x12);

    pAdvertising->start();
    if(Serial){
    Serial.println("Waiting for a client connection to notify...");
    }
}

void loop() {
    // Print message for debugging purposes
    if (!deviceConnected) {
      if(Serial){
        Serial.println("Device not connected. Restarting advertisement...");
        delay(500);
      }
    } else {

        // Generate random power value (100W - 400W)
        uint16_t power = random(100, 400);
        
        if(Serial){ 
        // Ensure the device is connected before sending power data
        Serial.println("Device is connected. Sending power data...");
        // Print the generated power value for debugging
        Serial.print("Generated Power: ");
        Serial.println(power);
        }

        // Flags (2 bytes): 0x00 means only power is included, no additional data
        uint8_t flags[2] = { 0x00, 0x00 };

        // Power value (2 bytes) - little-endian format
        uint8_t powerData[2] = { lowByte(power), highByte(power) };

        // Combine into a single array
        uint8_t cpmData[4];
        memcpy(cpmData, flags, 2);  // Copy flags (2 bytes)
        memcpy(cpmData + 2, powerData, 2);  // Copy power (2 bytes)

        // Send the properly formatted Cycling Power Measurement packet
        pCharacteritic->setValue(cpmData, sizeof(cpmData));
        pCharacteritic->notify();

        pCharacteritic->notify();
        delay(500);
    }
}
