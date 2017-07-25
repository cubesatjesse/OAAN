/*
   TODO calibrate while detumbling may be a problem?
   V2 Added noise cancling. data is fairly clean now but approach is quite manual. I suspect the random values
   the IMU jumps to vary between model. these false readings can come 2 or 3 in a row.
   how can we prepare the spacecraft for an abscence of data that long

   two (or more) noise values per data stream. 9 streams means 18 if statments and 9 switches per read...
   the other option is buy many imu's and select the best ones. (may entirely solve noise problem)
   it's a legitimate practice in tranzistor manufacturing. these definitely arent transistors though.


   log every crazy value. if too many per thousand updates restart?
*/



#include "quaternionFilters.h"
#include "MPU9250.h"

MPU9250 myIMU;

long int i = 0; //number of imu reads
String lr = ""; //last sensor read

void setup()
{
  Wire.begin();
  // TWBR = 12;  // 400 kbit/sec I2C speed
  Serial.begin(38400); // Max speed TBD

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
 // delay(50);
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
  myIMU.ax = (float)myIMU.accelCount[0] * myIMU.aRes; // - accelBias[0];
  myIMU.ay = (float)myIMU.accelCount[1] * myIMU.aRes; // - accelBias[1];
  myIMU.az = (float)myIMU.accelCount[2] * myIMU.aRes; // - accelBias[2];

  myIMU.readGyroData(myIMU.gyroCount);  // Read the x/y/z adc values
  myIMU.getGres();

  // Calculate the gyro value into actual degrees per second
  // This depends on scale being set
  myIMU.gx = (float)myIMU.gyroCount[0] * myIMU.gRes;
  myIMU.gy = (float)myIMU.gyroCount[1] * myIMU.gRes;
  myIMU.gz = (float)myIMU.gyroCount[2] * myIMU.gRes;

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
  myIMU.mx = (float)myIMU.magCount[0] * myIMU.mRes * myIMU.magCalibration[0] -
             myIMU.magbias[0];
  myIMU.my = (float)myIMU.magCount[1] * myIMU.mRes * myIMU.magCalibration[1] -
             myIMU.magbias[1];
  myIMU.mz = (float)myIMU.magCount[2] * myIMU.mRes * myIMU.magCalibration[2] -
             myIMU.magbias[2];

  // Must be called before updating quaternions!
  myIMU.updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  //  MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  my,  mx, mz);
  MahonyQuaternionUpdate(myIMU.ax, myIMU.ay, myIMU.az, myIMU.gx * DEG_TO_RAD,
                         myIMU.gy * DEG_TO_RAD, myIMU.gz * DEG_TO_RAD, myIMU.my,
                         myIMU.mx, myIMU.mz, myIMU.deltat);

  String output = "";

  //output += String(1000 * myIMU.ax);
  String cr = String(1000 * myIMU.ax);
  bool noise = false;
  if (cr == "1765.63") {
    noise = true;
  } else if (cr == "-987.55") {
    noise = true;
  }
  

  switch (noise) {
    case (1):
      output += lr;
      break;
    case (0):
      output += cr;
      break;
  }

  lr = cr;
  //  output += "ax:" + String(1000 * myIMU.ax) + ",";
  //  output += "ay:" + String(1000 * myIMU.ay) + ","; // milligs
  //  output += "az:" + String(1000 * myIMU.az) + ",";
  //  output += "gx:" + String(myIMU.gx, 3) + ",";
  //  output += "gy:" + String(myIMU.gy, 3) + ","; // deg/sec
  //  output += "gz:" + String(myIMU.gz, 3) + ",";
  //  output += "mx:" + String(myIMU.mx) + ",";
  //  output += "my:" + String(myIMU.my) + ","; // milligauss
  //  output += "mz:" + String(myIMU.mz) + ",";
  //
  //  myIMU.tempCount = myIMU.readTempData();  // Read the adc values
  //  // Temperature in degrees Centigrade
  //  myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
  //  // Print temperature in degrees Centigrade
  //
  //  output += "tm:" + String(myIMU.temperature) + ","; // degrees celcius





  //  Serial.print("OUTPUT STRING:");
    Serial.println(output);
  return output;


  // Print acceleration values in milligs!

  //
  //        myIMU.tempCount = myIMU.readTempData();  // Read the adc values
  //        // Temperature in degrees Centigrade
  //        myIMU.temperature = ((float) myIMU.tempCount) / 333.87 + 21.0;
  //        // Print temperature in degrees Centigrade
  //        Serial.print("Temperature is ");  Serial.print(myIMU.temperature, 1);
  //        Serial.println(" degrees C");



  //        Serial.print("q0 = "); Serial.print(*getQ());
  //        Serial.print(" qx = "); Serial.print(*(getQ() + 1));
  //        Serial.print(" qy = "); Serial.print(*(getQ() + 2));
  //        Serial.print(" qz = "); Serial.println(*(getQ() + 3));


  // Define output variables from updated quaternion---these are Tait-Bryan
  // angles, commonly used in aircraft orientation. In this coordinate system,
  // the positive z-axis is down toward Earth. Yaw is the angle between Sensor
  // x-axis and Earth magnetic North (or true North if corrected for local
  // declination, looking down on the sensor positive yaw is counterclockwise.
  // Pitch is angle between sensor x-axis and Earth ground plane, toward the
  // Earth is positive, up toward the sky is negative. Roll is angle between
  // sensor y-axis and Earth ground plane, y-axis up is positive roll. These
  // arise from the definition of the homogeneous rotation matrix constructed
  // from quaternions. Tait-Bryan angles as well as Euler angles are
  // non-commutative; that is, the get the correct orientation the rotations
  // must be applied in the correct order which for this configuration is yaw,
  // pitch, and then roll.
  // For more see
  // http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
  // which has additional links.
}
