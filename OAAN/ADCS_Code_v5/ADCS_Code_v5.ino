//problem is getting too specific. this isnt the IMU we're using long term so i should probably just leave it alone


#include "quaternionFilters.h"
#include "MPU9250.h"

MPU9250 myIMU;

long int i = 0; //number of imu reads
String lr = ""; //last sensor read
String WireRead = "";

void setup()
{
  Wire.begin();
  // TWBR = 12;  // 400 kbit/sec I2C speed
  Serial.begin(19200); // Max speed TBD
  Serial1.begin(74880); // Max speed TBD

  byte c = myIMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);

  if (c == 0x71) // WHO_AM_I should always be 0x68
  {

    // Calibrate gyro and accelerometers, load biases in bias registers
    myIMU.calibrateMPU9250(myIMU.gyroBias, myIMU.accelBias);


    myIMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature

    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = myIMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);


    // Get magnetometer calibration from AK8963 ROM
    myIMU.initAK8963(myIMU.magCalibration);
    myIMU.magCalibration[0];
    myIMU.magCalibration[1];
    myIMU.magCalibration[2];
    // Initialize device for active mode read of magnetometer

  } // if (c == 0x71)
  else
  {
    // IMU not detected fault / retry
  }
}

void loop() {



  //Serial.println (i);
  i++;
  delay(1000);
  myIMU.updateTime();
  myIMU.delt_t = millis() - myIMU.count;
  if (myIMU.delt_t > 500) {
    ToString();
  }
}

String ToString()
{
  //gather data and turn into string
  // todo do these calibration values change? or do i only need th do them once?

  myIMU.readAccelData(myIMU.accelCount);  // Read the x/y/z adc values
  myIMU.getAres();

  // Now we'll calculate the accleration value into actual g's
  // This depends on scale being set
  myIMU.ax = (int)myIMU.accelCount[0] * myIMU.aRes; // - accelBias[0];
  myIMU.ay = (int)myIMU.accelCount[1] * myIMU.aRes; // - accelBias[1];
  myIMU.az = (int)myIMU.accelCount[2] * myIMU.aRes; // - accelBias[2];

  myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values
  myIMU.getGres();

  // Calculate the gyro value into actual degrees per second
  // This depends on scale being set
  myIMU.gx = (int)myIMU.gyroCount[0] * myIMU.gRes;
  myIMU.gy = (int)myIMU.gyroCount[1] * myIMU.gRes;
  myIMU.gz = (int)myIMU.gyroCount[2] * myIMU.gRes;

  myIMU.readMagData(myIMU.magCount);  // Read the x/y/z adc values
  myIMU.getMres();
  // User environmental x-axis correction in milliGauss, should be
  // automatically calculated
  myIMU.magbias[0] = +470.;
  // User environmental x-axis correction in milliGauss TODO axis??
  myIMU.magbias[1] = +120.;
  // User environmental x-axis correction in milliGauss
  myIMU.magbias[2] = +125.;

  // Calculate the magnetometer values in milliGauss
  // Include factory calibration per data sheet and user environmental
  // corrections
  // Get actual magnetometer value, this depends on scale being set
  myIMU.mx = (int)myIMU.magCount[0] * myIMU.mRes * myIMU.magCalibration[0] -
             myIMU.magbias[0];
  myIMU.my = (int)myIMU.magCount[1] * myIMU.mRes * myIMU.magCalibration[1] -
             myIMU.magbias[1];
  myIMU.mz = (int)myIMU.magCount[2] * myIMU.mRes * myIMU.magCalibration[2] -
             myIMU.magbias[2];

  // Must be called before updating quaternions!
  myIMU.updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the outputString to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  //  MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  my,  mx, mz);
  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx * DEG_TO_RAD,
                         myIMU.gy * DEG_TO_RAD, myIMU.gz * DEG_TO_RAD, myIMU.my,
                         myIMU.mx, myIMU.mz, myIMU.deltat);

  String outputString = "";

  //  //outputString += String(1000 * myIMU.ax);
  //  String cr = String(1000 * myIMU.ax);
  //  bool noise = false;
  //  if (cr == "1765.63") {
  //    noise = true;
  //  } else if (cr == "-987.55") {
  //    noise = true;
  //  }
  //
  //
  //  switch (noise) {
  //    case (1):
  //      outputString += lr;
  //      break;
  //    case (0):
  //      outputString += cr;
  //      break;
  //  }
  //
  //  lr = cr;

  float temp = float(1000 * myIMU.ax);
  int orary = (int) (temp * 1000);
  temp = (float) orary / 1000;

//  Serial.print("correct value:");
//  Serial.println(String(1000 * myIMU.ax));
//  Serial.print("our value:");
//  Serial.println(orary);
  
      outputString += "106," + String(unFloat(float(1000 * myIMU.ax))) + "!";
      outputString += "107," + String(unFloat(float(1000 * myIMU.ay))) + "!"; // milligs
      outputString += "108," + String(unFloat(float(1000 * myIMU.az))) + "!";
Serial.println(float(myIMU.gx));
Serial.println(String(myIMU.gx, 3));
  //    outputString += "100," + String(myIMU.gx, 3) + "!";
  //    outputString += "101," + String(myIMU.gy, 3) + "!"; // deg/sec
  //    outputString += "102," + String(myIMU.gz, 3) + "!";
  //    outputString += "103," + String(myIMU.mx) + "!";
  //    outputString += "104," + String(myIMU.my) + "!"; // milligauss
  //    outputString += "105," + String(myIMU.mz) + "!";
  //    outputString += "109,1!\n";

  myIMU.tempCount = myIMU.readTempData();  // Read the adc values
  // Temperature in degrees Centigrade
  myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
  // Print temperature in degrees Centigrade

  //outputString += "tm:" + String(myIMU.temperature) + ","; // degrees celcius





  //  Serial.print("outputString STRING:");
  //Serial1.print(outputString);
  //Serial.println(outputString);


  // Print acceleration values in milligs!


  //          myIMU.tempCount = myIMU.readTempData();  // Read the adc values
  //          // Temperature in degrees Centigrade
  //          myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
  //          // Print temperature in degrees Centigrade
  //          Serial.print("Temperature is ");  Serial.print(myIMU.temperature, 1);
  //          Serial.println(" degrees C");

  return outputString;


  //        Serial.print("q0 = "); Serial.print(*getQ());
  //        Serial.print(" qx = "); Serial.print(*(getQ() + 1));
  //        Serial.print(" qy = "); Serial.print(*(getQ() + 2));
  //        Serial.print(" qz = "); Serial.println(*(getQ() + 3));
}


int unFloat (float i) {
  float infloat = float(1000 * i);
  int outint = (int) (infloat * 1000);
  return outint;
}






















