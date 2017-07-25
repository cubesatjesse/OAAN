
#ifndef NYCubeSat_h
#define NYCubeSat_h

#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino.h>

class NYCubeSat

{
  public:
   
    int getTempDegrees(int TempPin);

    void configureSensor();

    int getImuTempData(Adafruit_LSM9DS0 imu, int wT);

    void initalizePinOut();

    void buildBuffer(String com);

    boolean isInputValid(String input);

    void popCommand();

    void readSerialAdd2Buffer();

    void sendSCommand(char data[]);

    String requestFromSlave();

};

#endif
