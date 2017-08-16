/*
  The sensor outputs provided by the library are the raw 16-bit values
  obtained by concatenating the 8-bit high and low magnetometer data registers.
  They can be converted to units of gauss using the
  conversion factors specified in the datasheet for your particular
  device and full scale setting (gain).

  Example: An LIS3MDL gives a magnetometer X axis reading of 1292 with its
  default full scale setting of +/- 4 gauss. The GN specification
  in the LIS3MDL datasheet (page 8) states a conversion factor of 6842
  LSB/gauss (where LSB means least significant bit) at this FS setting, so the raw
  reading of 1292 corresponds to 1292 / 6842 = 0.1888 gauss.
*/

#include <Wire.h>
#include <LIS3MDL.h>
#include <LSM6.h>

LSM6 imu;
LIS3MDL mag;

char report[80];
bool initFail = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  IMUInit();
}

void loop() {

  if (!initFail) {
    ReadImu();
  }
}
void IMUInit() {

  if (!mag.init() || !imu.init()) {
    Serial.println("Failed to detect and initialize magnetometer or IMU!");
    Serial1.println("110,1!");
    initFail = true;
  }
  else {
    initFail = false;
  }

  imu.enableDefault();
  mag.enableDefault();
}

void ReadImu() {

  mag.read();
  imu.read();

  snprintf(report, sizeof(report), "A: %6d %6d %6d    G: %6d %6d %6d    M: %6d %6d %6d",
           imu.a.x, imu.a.y, imu.a.z,
           imu.g.x, imu.g.y, imu.g.z,
           mag.m.x, mag.m.y, mag.m.z);

  Serial.println(report);


  //Serial.println(mag.m.x);
  //Serial.println(mag.m.y);
  //Serial.println(mag.m.z);

  delay(100);
}
