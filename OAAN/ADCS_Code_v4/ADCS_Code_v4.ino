
#include <Wire.h>
#include <LIS3MDL.h>
#include <LSM6.h>

LSM6 imu;
LIS3MDL mag;

unsigned long manualTimeout = 10 * 1000;
int endT = 0;
String receivedChars = "";
boolean newData = false;
char report[80];
bool initFail = false;

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
      endT = millis() + manualTimeout;
      while (i < 200 && millis() < endT) {
        if (commandStack[i][0] == -1 && commandStack[i][1] == -1) {
          break;
        }
        i++;
      }
    }
};
commandBuffer cBuf;


void commandParse() {

  long int commandData;
  long int commandType; // changed from int to long int. so if it breaks this is probably why
  bool l = true;
  String i = "";
  i = receivedChars;

  while (l) {

    commandType = (receivedChars.substring(0, receivedChars.indexOf(","))).toInt();

    commandData = (receivedChars.substring(receivedChars.indexOf(",") + 1, receivedChars.indexOf("!"))).toInt();

    cBuf.commandStack[cBuf.openSpot][0] = commandType;
    cBuf.commandStack[cBuf.openSpot][1] = commandData;
    if (receivedChars.indexOf("!") == receivedChars.length() - 1) {
      l = false;

    } else {

      receivedChars = receivedChars.substring(receivedChars.indexOf("!") + 1);
      i = receivedChars;

    }
    cBuf.openSpot++;
  }
}

void popCommands() {
  //Process all the Incoming Commands
  long start = millis();

  while (cBuf.openSpot > 0 && millis() - start < manualTimeout) {
    if (cBuf.openSpot > 0) {

      int currentCommand[2] = {cBuf.commandStack[cBuf.openSpot - 1][0], cBuf.commandStack[cBuf.openSpot - 1][1]};
      cBuf.commandStack[cBuf.openSpot - 1][0] = -1;
      cBuf.commandStack[cBuf.openSpot - 1][1] = -1;
      cBuf.openSpot --;

      //Supported Commands
      switch (currentCommand[0]) {

        case (1):
          if (currentCommand[1] = 1) {
            digitalWrite(13, HIGH);
          } else {
            digitalWrite(13, LOW);
          }
          break;

      }
    }
  }
}

boolean isInputValid() {
  // todo make timeout
  //Check if incoming command string <input> is valid
  int lastPunc = 0; //1 if ",", 2 if "!", 0 Otherwise
  bool valid = true;
  int q = 0;
  int l = receivedChars.length();

  if (l < 4) {
    valid = false;
  }

  while (q < l) {
    char currentChar = receivedChars[q];
    q++;

    if (isPunct(currentChar)) {
      if (currentChar == (',')) {
        //Check if last was a period
        if (receivedChars[q - 2] == '!') {
          valid = false;
          break;
        }
        if (lastPunc == 0 || lastPunc == 2) {
          lastPunc = 1;
        } else {
          valid = false;
          break;
        }
      } else if (currentChar == ('!')) {
        if (receivedChars[q - 2] == ',') {
          valid = false;
          break;
        }
        if (lastPunc == 1) {
          lastPunc = 2;
        } else {
          valid = false;
          break;
        }
      } else if (currentChar == ('-')) {
        if (receivedChars[q - 2] == ',') { //q incremented after value capture
        } else {
          valid = false;
          break;
        }
      } else if (currentChar == ('.')) {
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

    //Detect no ending exclamation point
    if (q == receivedChars.length() - 1) {
      if (receivedChars[q] != '!') {
        valid = false;
        break;
      }
    }
    //Null Character in the middle
    if (currentChar == '\0' && q != receivedChars.length() - 1) {
      valid = false;
      break;
    }
  }
  if (valid) {
  }
  if (!valid) {
  }
  return valid;
}


void recvWithEndMarker1() {
  // for serial0
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  if (newData == true) {

    while ( Serial1.available() > 0 && newData == false) {
      rc = Serial1.read();

      if (rc != endMarker) {
        receivedChars += (char(rc));
      }
      else {
        newData = true;
      }
    }
  }

  while ( Serial1.available() > 0 && newData == false) {
    rc = Serial1.read();
    if (rc != endMarker) {
      receivedChars += (char(rc));
    }
    else {
      newData = true;
    }
  }
}

void setup() {
  pinMode(13, OUTPUT); // for testing
  Serial1.begin(74880);
  Serial.begin(74880);
  Wire.begin();
  delay(100);
  IMUInit();
}

void loop() {

  popCommands();
  recvWithEndMarker1();

  if (!initFail) {
    ReadImu();
  } else {
    IMUInit();
  }

  if (newData) {
    if (isInputValid() && (receivedChars != "")) {
      commandParse();
    }
    newData = false;
    receivedChars = "";
  }

}


void IMUInit() {

  if (!mag.init() || !imu.init()) {
    Serial1.print(F("110,1!\n"));
    Serial.println("inti fail");
    initFail = true;
  }
  else {
    digitalWrite(13,HIGH);
    initFail = false;
  }

  imu.enableDefault();
  mag.enableDefault();

}

void ReadImu() {

  mag.read();
  imu.read();

//  snprintf(report, sizeof(report), "A: %6d %6d %6d    G: %6d %6d %6d    M: %6d %6d %6d",
//           imu.a.x, imu.a.y, imu.a.z,
//           imu.g.x, imu.g.y, imu.g.z,
//           mag.m.x, mag.m.y, mag.m.z);
// change the order of print statements
  Serial1.print("111,"); Serial1.print(imu.g.x); Serial1.print(F("!\n"));
  Serial1.print("101,"); Serial1.print(imu.g.y); Serial1.print(F("!\n"));
  Serial1.print("102,"); Serial1.print(imu.g.z); Serial1.print(F("!\n"));

  Serial1.print("103,"); Serial1.print(mag.m.x); Serial1.print(F("!\n"));
  Serial1.print("104,"); Serial1.print(mag.m.y); Serial1.print(F("!\n"));
  Serial1.print("105,"); Serial1.print(mag.m.z); Serial1.print(F("!\n"));

  Serial1.print("106,"); Serial1.print(imu.a.x); Serial1.print(F("!\n"));
  Serial1.print("107,"); Serial1.print(imu.a.y); Serial1.print(F("!\n"));
  Serial1.print("108,"); Serial1.print(imu.a.z); Serial1.print(F("!\n"));

  Serial1.print(F("109,1!\n"));
}

