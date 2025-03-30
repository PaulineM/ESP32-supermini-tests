#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Adafruit_MPU6050 mpu;

// #define I2C_SDA 0
// #define I2C_SCL 1

// TwoWire I2CMPU = TwoWire(0);

// float x_acceleration, y_acceleration, z_acceleration;
// float x_gyro, y_gyro, z_gyro;
// float tempe;


// void setup(){
//     Wire.begin();
//     I2CMPU.begin(I2C_SDA, I2C_SCL, 100000);
//     Serial.begin(115200);
//     while(!Serial) delay(10);

//     Serial.println("Serial ok");

//     if(!mpu.begin()){
//         Serial.println("MPU not connected");
//     }

//     Serial.println("MPU ok");

//     mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
//     mpu.setGyroRange(MPU6050_RANGE_500_DEG);
//     mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);


// }

// void loop(){
//     sensors_event_t a, g, temp;
//     mpu.getEvent(&a, &g, &temp);

//     x_acceleration = a.acceleration.x;
//     y_acceleration = a.acceleration.y;
//     z_acceleration = a.acceleration.z;
//     x_gyro = g.gyro.x;
//     y_gyro = g.gyro.y;
//     z_gyro = g.gyro.z;
//     tempe = temp.temperature;
    
//     /* Print out the values */
//     Serial.print("Acceleration X: ");
//     Serial.print(x_acceleration);
//     Serial.print(", Y: ");
//     Serial.print(y_acceleration);
//     Serial.print(", Z: ");
//     Serial.print(z_acceleration);
//     Serial.println(" m/s^2");

//     Serial.print("Rotation X: ");
//     Serial.print(x_gyro);
//     Serial.print(", Y: ");

//     Serial.print(y_gyro);
//     Serial.print(", Z: ");
//     Serial.print(z_gyro);
//     Serial.println(" rad/s");

//     Serial.print("Temperature: ");
//     Serial.print(tempe);
//     Serial.println(" degC");

//     Serial.println("");
//     delay(500);
// }

#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define I2C_SDA 2  // Your chosen I2C SDA pin
#define I2C_SCL 3  // Your chosen I2C SCL pin

TwoWire I2CMPU = TwoWire(0);  // Use TwoWire instance for custom I2C pins
Adafruit_MPU6050 mpu;

float x_acceleration, y_acceleration, z_acceleration;
float x_gyro, y_gyro, z_gyro;
float tempe;

void setup() {
    delay(3000);
    Serial.begin(115200);
    while (!Serial) delay(10);  // Wait for Serial Monitor to open

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
    delay(100);
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    x_acceleration = a.acceleration.x;
    y_acceleration = a.acceleration.y;
    z_acceleration = a.acceleration.z - 1.4;
    x_gyro = g.gyro.x + 0.01;
    y_gyro = g.gyro.y + 0.04;
    z_gyro = g.gyro.z - 0.01;
    tempe = temp.temperature;

    Serial.print("Acceleration (m/s²) -> X: ");
    Serial.print(x_acceleration);
    Serial.print(" | Y: ");
    Serial.print(y_acceleration);
    Serial.print(" | Z: ");
    Serial.println(z_acceleration);

    Serial.print("Gyroscope (rad/s) -> X: ");
    Serial.print(x_gyro);
    Serial.print(" | Y: ");
    Serial.print(y_gyro);
    Serial.print(" | Z: ");
    Serial.println(z_gyro);

    Serial.print("Temperature: ");
    Serial.print(tempe);
    Serial.println(" °C");

    Serial.println("--------------------");
    delay(500);
}
