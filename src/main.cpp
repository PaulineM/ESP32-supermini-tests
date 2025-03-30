#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// ESP32-C3 - GPIOs 2, 8 and 9 are strapping pins.

#define I2C_SDA 2  // Your chosen I2C SDA pin
#define I2C_SCL 3  // Your chosen I2C SCL pin

TwoWire I2CMPU = TwoWire(0);  // Use TwoWire instance for custom I2C pins
Adafruit_MPU6050 mpu;

float x_acceleration, y_acceleration, z_acceleration;
float x_gyro, y_gyro, z_gyro;
float tempe;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// #define SERVICE_UUID        "1818"  // Cycling Power Service (CPS) UUID
// #define CHARACTERISTIC_UUID "2A63"  // Cycling Power Measurement UUID

#define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"  
#define CHARACTERISTIC_UUID "87654321-4321-6789-4321-abcdef987654"

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

        Serial.println("Initializing I2C...");
    I2CMPU.begin(I2C_SDA, I2C_SCL, 100000);

    if (!mpu.begin(0x68, &I2CMPU)) {  // Pass TwoWire instance
        Serial.println("MPU6050 not connected! Check wiring.");
        while (1);
    }

    Serial.println("MPU6050 connected successfully!");

    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
    // Check if device is connected or not
    if (!deviceConnected) {
        Serial.println("Device not connected. Restarting advertisement...");
        NimBLEDevice::getAdvertising()->start();
        delay(500);  // Wait for a client to connect
    } else {
        // When device is connected, send power data
        // uint16_t power = random(100, 400);
        Serial.println("Device is connected. Sending power data...");
        Serial.print("Generated Power: ");
        // Serial.println(power);

        // sensors_event_t a, g, temp;
        // mpu.getEvent(&a, &g, &temp);

        // z_gyro = g.gyro.z - 0.01;
        // tempe = temp.temperature;

        // uint16_t power = z_gyro;

        // Serial.println(z_gyro);

        // Start measuring for 0.5 seconds (500 ms)
        float z_gyro_sum = 0;
        int sample_count = 0;

        unsigned long start_time = millis();
        
        while (millis() - start_time < 500) {  // Collect data for 0.5 seconds
            sensors_event_t a, g, temp;
            mpu.getEvent(&a, &g, &temp);
            
            z_gyro_sum += g.gyro.z;  // Add the gyro z-axis reading to the sum
            sample_count++;  // Increment the sample count
            
            delay(5);  // Small delay to not overwhelm the loop, ~200Hz sampling
        }
        
        // Calculate the average value of the gyro z-axis over 0.5 seconds
        float z_gyro_avg = z_gyro_sum / sample_count - 0.0093;

        uint16_t power = z_gyro_avg;


        // // Flags (2 bytes)
        uint8_t flags[2] = { 0x00, 0x00 };

        // // Power value (2 bytes) - little-endian format
        // uint8_t powerData[2] = { lowByte(power), highByte(power) };

        // // Combine into a single array
        // uint8_t cpmData[4];
        // memcpy(cpmData, flags, 2);
        // memcpy(cpmData + 2, powerData, 2);

        // Convert float to byte array
        uint8_t gyroData[4];
        memcpy(gyroData, &z_gyro_avg, sizeof(float));

        // Combine into a single array (flags + float)
        uint8_t cpmData[6];
        memcpy(cpmData, flags, 2);
        memcpy(cpmData + 2, gyroData, 4);

        // Send Cycling Power Measurement
        pCharacteristic->setValue(cpmData, sizeof(cpmData));
        pCharacteristic->notify();

        delay(500);  // Wait before next power update
    }
}
