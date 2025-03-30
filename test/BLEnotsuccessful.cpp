#include <NimBLEDevice.h>

#define SERVICE_UUID        "1818"  // Cycling Power Service (CPS) UUID
#define CHARACTERISTIC_UUID "2A63"  // Cycling Power Measurement UUID

NimBLEServer* pServer = nullptr;
NimBLECharacteristic* pPowerMeasurement = nullptr;
bool deviceConnected = false;
NimBLEAdvertising *pAdvertising = nullptr;

class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        deviceConnected = true;
        Serial.println("Device Connected!");  // This should print when connection is established
    }
    
    void onDisconnect(NimBLEServer* pServer) {
        deviceConnected = false;
        Serial.println("Device Disconnected.");
        pAdvertising->start();  // Restart advertising after disconnect
        Serial.println("Restarting advertisement...");
    }
};

void setup() {
    Serial.begin(115200);
    while (!Serial);  // Wait for Serial Monitor to open
    Serial.println("Initializing Power Meter...");

    // Initialize BLE device
    NimBLEDevice::init("ESP32 Power Meter");

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service
    NimBLEService *pService = pServer->createService(NimBLEUUID(SERVICE_UUID));

    // Create BLE Characteristic for Cycling Power Measurement
    pPowerMeasurement = pService->createCharacteristic(
        NimBLEUUID(CHARACTERISTIC_UUID),
        NIMBLE_PROPERTY::NOTIFY
    );

    // Start the service
    pService->start();

    // Create BLE Advertising
    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);

    // Set default advertising interval (leave it as it was)
    pAdvertising->setMinInterval(0x80);  // 1280ms minimum advertising interval
    pAdvertising->setMaxInterval(0x80);  // 1280ms maximum advertising interval

    // Start advertising
    Serial.println("Starting advertising...");
    bool started = pAdvertising->start();
    if (started) {
        Serial.println("Advertising started...");
    } else {
        Serial.println("Advertising failed to start!");
    }
}

void loop() {
    // Print message for debugging purposes
    if (!deviceConnected) {
        Serial.println("Device not connected. Restarting advertisement...");
        pAdvertising->start();  // Restart advertisement if not connected
        delay(5000);  // Restart advertisement every 5 seconds
    } else {
        // Ensure the device is connected before sending power data
        Serial.println("Device is connected. Sending power data...");

        // Generate random power value (100W - 400W)
        uint16_t power = random(100, 400);
        
        // Print the generated power value for debugging
        Serial.print("Generated Power: ");
        Serial.println(power);

        // Send instantaneous power in the correct format (16-bit power)
        uint8_t powerData[2] = { lowByte(power), highByte(power) };

        // Send notification with power data
        pPowerMeasurement->setValue(powerData, sizeof(powerData));
        pPowerMeasurement->notify();

        // Debugging: Log the power data being sent
        Serial.print("Power Data Sent: ");
        for (int i = 0; i < sizeof(powerData); i++) {
            Serial.print(powerData[i], HEX);  // Print in HEX format
            Serial.print(" ");
        }
        Serial.println();

        // Wait for a second before sending the next value
        delay(1000);
    }
}
