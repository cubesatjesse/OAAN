#include <Adafruit_LSM9DS0.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Arduino.h>

NYCubeSat::NYCubeSat() {
}
class commandBuffer {
  public:
    int commandStack[200][2];
    int openSpot;
    commandBuffer() {
      commandStack[200][2] = { -1};
      openSpot = 0;
    }
    void print() {
      int i = 0;
      Serial.print("cBuf = [");
      int endT = millis() + manualTimeout;
      while (i < 200 && millis() < endT) {
        if (commandStack[i][0] == -1 && commandStack[i][1] == -1) {
          break;
        }
        Serial.print(commandStack[i][0]);
        Serial.print(":");
        Serial.print(commandStack[i][1]);
        Serial.print("|");
        i++;
      }
      Serial.println("]");
    }
};


void NYCubeSat::int getTempDegrees(int TempPin) {
  int temp = analogRead(TempPin);
  temp = map(temp, 0, 543, -50, 125);
  return temp;
}

void NYCubeSat::void configureSensor()
{
  imu.setupMag(imu.LSM9DS0_MAGGAIN_2GAUSS);
  imu.setupGyro(imu.LSM9DS0_GYROSCALE_245DPS);
}
//error: expected unqualified-id before 'int'
// void NYCubeSat::int getImuTempData(Adafruit_LSM9DS0 imu, int wT)
// HEY FIX THIS PLEASE THE THING IN PARENTESIS IS BROKEN\/
//void NYCubeSat::int getImuTempData(Adafruit_LSM9DS0 imu, int wT)
//{
//  int k = 0;
//  int sum = 0;
//  long endTime = millis() + long(wT);
//  while (millis() < endTime) {
//    imu.read();
//    sum = sum + (int)imu.temperature;
//    k++;
//  }
//  return sum * (1.0 / k / 8.0);
//}

void NYCubeSat::void initalizePinOut() {
  const int DoorSens = 13; pinMode(DoorSens, INPUT);
  const int DoorTrig = 5; pinMode(DoorTrig, OUTPUT);
  const int Battery = A0; pinMode(Battery, INPUT);
  const int RBRx = 0;
  const int RBTx = 1;
  const int RBSleep = 22; pinMode(RBSleep, OUTPUT);
  const int RB_RI = 23; pinMode(RB_RI, INPUT);
  const int RB_RTS = 24; pinMode(RB_RTS, INPUT);
  const int RB_CTS = 6; pinMode(RB_CTS, INPUT);
  const int SDApin = 20;
  const int SCLpin = 21;
  const int SolarXPlus = A1; pinMode(SolarXPlus, INPUT);
  const int SolarXMinus = A2; pinMode(SolarXMinus, INPUT);
  const int SolarYPlus = A3; pinMode(SolarYPlus, INPUT);
  const int SolarYMinus = A4; pinMode(SolarYMinus, INPUT);
  const int SolarZPlus = A5; pinMode(SolarZPlus, INPUT);
  const int SolarZMinus = 9; pinMode(SolarZMinus, INPUT);
  const int SlaveReset = 10; pinMode(SolarZMinus, INPUT);
  const int DoorMagEnable = 11; pinMode(DoorMagEnable, OUTPUT);
}

void NYCubeSat::int getCurrentAmp(int CurrentPin) {
  int current = analogRead(CurrentPin);
  current = map(current, 0, 1, 0, 1);
  return current;
}
//void NYCubeSat:: buildBuffer(String com)
void NYCubeSat:: buildBuffer(String com)
{
  int commandData;
  int commandType;
  String comRemaining = com;
  bool loop = true;
  while (loop) {
    commandType = (com.substring(0, com.indexOf(","))).toInt();
    commandData = (com.substring(com.indexOf(",") + 1, com.indexOf("."))).toInt();
    cBuf.commandStack[cBuf.openSpot][0] = commandType;
    cBuf.commandStack[cBuf.openSpot][1] = commandData;
    if (com.indexOf(".") == com.length() - 1) {
      loop = false;
      Serial.println("Finished Adding Commands");
    } else {
      com = com.substring(com.indexOf(".") + 1);
    }
    cBuf.openSpot++;
  }
}

void NYCubeSat::boolean isInputValid(String input) {

  int lastPunc = 0;
  bool valid = true;
  int q = 0;
  int l = input.length();
  int endT = manualTimeout + millis();
  while (q < l) {
    char currentChar = input[q];
    q++;

    if (millis() > endT) {
      valid = false;
      break;
    }

    if (isPunct(currentChar)) {
      if (currentChar == (',')) {
        if (lastPunc == 0 || lastPunc == 2) {
          lastPunc = 1;
        } else {
          valid = false;
          break;
        }
      } else if (currentChar == ('.')) {
        if (lastPunc == 1) {
          lastPunc = 2;
        } else {
          valid = false;
          break;
        }
      } else {
        valid = false;
        break;
      }
    } else if (isAlpha(currentChar)) {
      valid = false;
      break;
    } else if (isSpace(currentChar)) {
      valid = false;
      break;
    }

    if (q == input.length() - 1) {
      if (input[q] != '.') {
        valid = false;
        break;
      }
    }
    if (currentChar == '\0' && q != input.length() - 1) {
      valid = false;
      break;
    }
  }
  return valid;
}

void NYCubeSat::void popCommand() {
  Serial.println ("Executing Command:");
  if (cBuf.openSpot > 0) {
    Serial.println (cBuf.openSpot - 1);
    int currentCommand[2] = {cBuf.commandStack[cBuf.openSpot - 1][0], cBuf.commandStack[cBuf.openSpot - 1][1]};
    Serial.print(currentCommand[0]);
    Serial.print(":");
    Serial.println(currentCommand[1]);
    cBuf.commandStack[cBuf.openSpot - 1][0] = 0;
    cBuf.commandStack[cBuf.openSpot - 1][1] = 0;
    cBuf.openSpot --;
    switch (currentCommand[1]) {
      case (11):
        deployTimeOut = (currentCommand[2]);
        break;
      case (12):
        manualTimeout = (currentCommand[2]);
        break;
    }

  } else {
    Serial.println("No Command");
  }
}

void NYCubeSat::void readSerialAdd2Buffer() {

  if (Serial.available() > 0) {
    Serial.println("Recieving Command");
    String comString = "";
    while (Serial.available() > 0) {

      char inChar = (char)Serial.read();
      comString += inChar;
    }
    if (isInputValid(comString)) {
      Serial.println("Command is Valid");
      buildBuffer(comString);
      Serial.println("Built Command Buffer Successfully");
    } else {
      Serial.println("Invalid Command");
    }
  }
}

void NYCubeSat::void sendSCommand(char data[]) {
  Wire.beginTransmission(8);
  Serial.print("Command Sent to Slave: ");
  Serial.println(data);
  Wire.write(data);
  Wire.endTransmission();
}

void NYCubeSat::String requestFromSlave() {
  Wire.requestFrom(8, slaveDataSize);
  delay(500);
  String res = "";
  while (Wire.available()) {
    res += Wire.read();
  }
  return res;
}

void NYCubeSat::char SCommand[] = "1,1.";
if (TestReset && (millis() - lastSComTime > SComTime || commandedSC)) {
  sendSCommand(SCommand);
  SlaveResponse = requestFromSlave();
  if (!SlaveResponse.equals("")) {
    lastSComTime = millis();
  }
}
