//worked on aug 8th

//teensy to teensy comm still needs work, get IMU data transfer nice and clean todo
//////////////////////////////////////////////////
// Flight_Code_v8:
//
// partially finished changing dowlinking format from a string to binary int array
//
//////////////////////////////////////////////////

//todo i2c interface w/ acs comp, test rockblock functionality, checktime, turn imu floats to ints
//run IMU, Binary parser, update MSH, Check in w/ ben, safely reset millis

long unsigned int deleteme = 0;

long day = 86400000; // 86400000 milliseconds in a day
long hour = 3600000; // 3600000 milliseconds in an hour
long minute = 60000; // 60000 milliseconds in a minute
long second =  1000; // 1000 milliseconds in a second

unsigned long manualTimeout = 10 * 1000;
int endT = 0;
unsigned long int z;//number of millis in current cycle
int nextMode = 0; // mode of the spacecraft next cycle

const int SenseNumb = 10; //change number of data samples in averaging array
const int faultSize = 10;
bool SensUpdate = false; //is there fresh IMU data
bool IMUBufferFull = false; //must fill w/ data before averages can be taken
int SenseCounter = 0;

bool WireConnected = true;
boolean newData = false;
String receivedChars = "";
// makeshift command interpreter. read command parse note for better
// explaination. popcommands is up and running.


////////////////////////////////////////////////////////////////////////////////
/////////////////Database of parameters/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class masterStatus {
    //Class to hold entire State of Spacecraft Operation except timers
  public:
    // To others looking at this code:
    //it is important that as few of these variables as possible get arbitrarily assigned a value.
    // these should only be given real data

    int State;
    int NextState;
    int Faults; //number of faults detected over satellite lifetime
    int activeFaults[faultSize];
    String FaultString;
    bool DockingLeader; //docking requires leader and follower this can be toggled through commands

    //Fixed variables
    int XGyroThresh;
    int YGyroThresh;
    int ZGyroThresh;
    int XAccelThresh;
    int YAccelThresh;
    int ZAccelThresh;

    //ROCKBLOCK Message Variables
    bool AttemptingLink; //True if waiting for SBDIX to return
    bool MessageStaged; //True if message waiting in Mobile Originated Buffer
    int RBCheckType; //State of Outgoing Communication with RockBlock. 0=ping, 1=Send SBDIX, 2=Fetch Incomming Command
    int MOStatus; //0 if No Outgoing message, 1 if outgoing message success, 2 if error
    int MOMSN; //Outgoing Message #
    int MTStatus; //0 if No Incoming message, 1 if Incoming message success, 2 if error
    int MTMSN; //Incoming Message #
    int MTLength; //Incoming Message Length in bytes
    int MTQueued; //# of messages waiting in iridium
    String SBDRT;
    int LastMsgType; //0 = inval, 1 = ok, 2 = ring, 3 = error, 4 = ready //TODO
    int LastSMsgType; //Only Update on NON EMPTY reads from RB: 0 = inval, 1 = ok, 2 = ring, 3 = error, 4 = ready //TODO
    int SBDIXFails;
    int currentSegment;

    //Binary array variables
    int binArr[1000];
    int binOpenSpot = 0;


    // todo accumulated or raw data?
    //  get all the min max vals/percision
    long int Mag[3];
    long int Gyro[3];
    long int Accel[3];

    long int MagAve[3];
    long int GyroAve[3];
    long int AccelAve[3];

    long int MagLog[3][SenseNumb];
    long int GyroLog[3][SenseNumb];
    long int AccelLog[3][SenseNumb];

    int TempAcc;
    int ImuTemp;
    // may need to store the zeros aswell.

    //power board variables
    float Battery; // battery charge

    //ADCS State Variables: torque rods
    int TorqueXDir; //-1 or 1 for Coil Current Direction
    int TorqueXPWM; // 0 to 255 for Coil Current Level
    int TorqueYDir; //-1 or 1 for Coil Current Direction
    int TorqueYPWM; // 0 to 255 for Coil Current Level
    int TorqueZDir; //-1 or 1 for Coil Current Direction
    int TorqueZPWM; // 0 to 255 for Coil Current Level

    //ADCS State Variables: motors
    //todo update motor min/max rpm once new motors have been selected
    int MotorXDir; //-1 or 1 for Direction
    int MotorXRPM; // ~ 200 to 6000 RPM
    int MotorYDir; //-1 or 1 for Direction
    int MotorYRPM; // ~ 200 to 6000 RPM
    int MotorZDir; //-1 or 1 for Direction
    int MotorZRPM; // ~ 200 to 6000 RPM

    //Radio State Variables:
    //todo flesh out once we get our hands on a radio
    bool DownlinkStaged;
    bool AttemptingDownLink;
    bool StageReport;
    int lastSR; //last special report
    int SRFreq;

    masterStatus() {

      //fixed variables
      XGyroThresh = 500; //todo chose real values
      YGyroThresh = 500;
      ZGyroThresh = 500;
      XAccelThresh = 100;
      YAccelThresh = 100;
      ZAccelThresh = 100;

      //Constructor
      State = 1; //normal ops
      DockingLeader = true;

      Gyro[3] = {0};
      Mag[3] = {0};
      Accel[3] = {0};
      ImuTemp = 0;
      Battery = 3.7; //todo i'd rather not have the default value trip fault detection 50%?

      Faults = 0;

      DownlinkStaged = false; // have data to transfer
      AttemptingDownLink = false; // radio transmission in progress
      StageReport = false;
      lastSR = 0;
      SRFreq = 9; //how many normal reports before special extra data report


      TorqueXDir = 0;
      TorqueXPWM = 0;
      TorqueYDir = 0;
      TorqueYPWM = 0;
      TorqueZDir = 0;
      TorqueZPWM = 0;

      MotorXDir = 0;
      MotorXRPM = 0;
      MotorYDir = 0;
      MotorYRPM = 0;
      MotorZDir = 0;
      MotorZRPM = 0;
    }

    float roundDecimal(float num, int places) {
      int roundedNum = round(pow(10, places) * num);
      return roundedNum / ((float)(pow(10, places)));
    }

    void PrintStatus() { // all data should be ints (still works w/ floats though..)
      build1Byte(State);
      build3Byte(Gyro[0]); build3Byte(Gyro[1]); build3Byte(Gyro[2]); // change to ints
      build3Byte(Mag[0]); build3Byte(Mag[1]); build3Byte(Mag[2]);
      build3Byte(Accel[0]); build3Byte(Accel[1]); build3Byte(Accel[2]);
      build1Byte(ImuTemp);
      build1Byte(Battery);
      build1Byte(Faults);
      build1Byte(TorqueXDir); build2Byte(TorqueXPWM);
      build1Byte(TorqueYDir); build2Byte(TorqueYPWM);
      build1Byte(TorqueZDir); build2Byte(TorqueZPWM);
      build1Byte(MotorXDir); build2Byte(MotorXRPM);
      build1Byte(MotorYDir); build2Byte(MotorYRPM);
      build1Byte(MotorZDir); build2Byte(MotorZRPM);
    }

    String SpecialReport() { //todo make this work w/ binary
      // all fixed values
      String output = "";
      output += "{";
      output += "Fl:" + String(Faults) + ",";
      output += "FlSt:" + String(FaultString);
      output += "SRF:" + String(SRFreq) + ",";
      output += "XGT:" + String(XGyroThresh) + ",";
      output += "YGT:" + String(YGyroThresh) + ",";
      output += "ZGT:" + String(ZGyroThresh) + ",";
      output += "XAT:" + String(XAccelThresh) + ",";
      output += "YAT:" + String(YAccelThresh) + ",";
      output += "ZAT:" + String(ZAccelThresh) + "}";
      return (output);
      // all misc information
    }

    /////////////////binary functions//////////////////

    void build3Byte(long int numb) {
      //range +- 8,388,607

      //if we have any values above 8 million we probably have a problem :)

      if (numb >= 0) { //if positive first bit = 1
        binArr[binOpenSpot] = 1;
        binOpenSpot ++;
      } else {
        binArr[binOpenSpot] = 0;
        binOpenSpot ++; //if negative first bit = 0
        numb = abs(numb);
      }

      if (numb > -8388607 && numb < 8388607) {


        if (numb < 2) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 4) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 8) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 16) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 32) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 64) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 128) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 256) {
          binArr[binOpenSpot] = 0; binOpenSpot ++; // 1 byte
        }
        if (numb < 512) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 1024) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 2048) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 4096) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 8192) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 16384) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 32768) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 65536) {
          binArr[binOpenSpot] = 0; binOpenSpot ++; //2byte
        }
        if (numb < 131072) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 262144) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 524288) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 1048576) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 2097152) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 4194304) {
          binArr[binOpenSpot] = 0; binOpenSpot ++; //23 bits of data
        }

      } else {
        numb = 8388607;
      }
      String h = "";
      int a = 0;
      h += String(numb, BIN);
      int msgLen = h.length();

      while (msgLen != 0) {

        String x = h.substring(a, a + 1);
        char d[10];
        x.toCharArray(d, 10);

        if (int(d[0]) == 49) {
          binArr[binOpenSpot] = 1;
        } else {
          binArr[binOpenSpot] = 0;
        }

        binOpenSpot++;
        a++;
        msgLen--;
      }
    }


    void build2Byte(long int numb) {
      //range +- 32,767

      if (numb >= 0) { //if positive first bit = 1
        binArr[binOpenSpot] = 1;
        binOpenSpot ++;
      } else {
        binArr[binOpenSpot] = 0;
        binOpenSpot ++; //if negative first bit = 0
        numb = abs(numb);
      }

      if (numb > -32767 && numb < 32767) {


        if (numb < 2) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 4) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 8) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 16) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 32) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 64) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 128) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 256) {
          binArr[binOpenSpot] = 0; binOpenSpot ++; // 1 byte
        }
        if (numb < 512) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 1024) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 2048) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 4096) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 8192) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 16384) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }

      } else {
        numb = 32767;
      }
      String h = "";
      int a = 0;
      h += String(numb, BIN);
      int msgLen = h.length();

      while (msgLen != 0) {

        String x = h.substring(a, a + 1);
        char d[10];
        x.toCharArray(d, 10);

        if (int(d[0]) == 49) {
          binArr[binOpenSpot] = 1;
        } else {
          binArr[binOpenSpot] = 0;
        }

        binOpenSpot++;
        a++;
        msgLen--;
      }
    }


    void build1Byte(long int numb) {
      //range +- 127

      if (numb >= 0) { //if positive first bit = 1
        binArr[binOpenSpot] = 1;
        binOpenSpot ++;
      } else {
        binArr[binOpenSpot] = 0;
        binOpenSpot ++; //if negative first bit = 0
        numb = abs(numb);
      }

      if (numb > -127 && numb < 127) {


        if (numb < 2) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 4) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 8) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 16) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 32) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }
        if (numb < 64) {
          binArr[binOpenSpot] = 0; binOpenSpot ++;
        }

      } else {
        numb = 127;
      }
      String h = "";
      int a = 0;
      h += String(numb, BIN);
      int msgLen = h.length();

      while (msgLen != 0) {

        String x = h.substring(a, a + 1);
        char d[10];
        x.toCharArray(d, 10);

        if (int(d[0]) == 49) {
          binArr[binOpenSpot] = 1;
        } else {
          binArr[binOpenSpot] = 0;
        }

        binOpenSpot++;
        a++;
        msgLen--;
      }
    }

    /////////////////////////////////////////////////////////////





};
masterStatus MSH; //Declare MSH






////////////////////////////////////////////////////////////////////////////////
/////////////////Command functions//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



class commandBuffer {
  public:
    int commandStack[200][2];
    int openSpot;
    commandBuffer() {
      commandStack[200][2] = { -1};
      openSpot = 0;
    }
    void print() {
      //Serial formatting and Serial output
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

  int commandData;
  int commandType; // changed from int to long int. so if it breaks this is probably why
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



void recvWithEndMarker() {
  // for serial
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while ( Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars += (char(rc));
    }
    else {
      newData = true;
    }
  }
}

void recvWithEndMarker1() {
  // for serial0
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while ( Serial3.available() > 0 && newData == false) {
    rc = Serial3.read();
    if (rc != endMarker) {
      receivedChars += (char(rc));
    }
    else {
      newData = true;
    }
  }
}

boolean isInputValid() {
  // todo make timeout
  //Serial.println("starting parse");
  //Check if incoming command string <input> is valid
  int lastPunc = 0; //1 if ",", 2 if "!", 0 Otherwise
  bool valid = true;
  int q = 0;
  int l = receivedChars.length();

  if (l < 4) {
    //Serial.println("command too short");
    valid = false;
  }

  while (q < l) {
    char currentChar = receivedChars[q];
    //Serial.println(currentChar,HEX);
    q++;

    if (isPunct(currentChar)) {
      if (currentChar == (',')) {
        //Check if last was a period
        //Serial.println("Comma Found");
        if (receivedChars[q - 2] == '!') {
          //Serial.println("No First Command Number");
          valid = false;
          break;
        }
        if (lastPunc == 0 || lastPunc == 2) {
          //Serial.println("Comma OK");
          lastPunc = 1;
        } else {
          //Serial.println("2 Commas");
          valid = false;
          break;
        }
      } else if (currentChar == ('!')) {
        if (receivedChars[q - 2] == ',') {
          //Serial.println("No Second Command Number");
          valid = false;
          break;
        }
        //Serial.println("Excl Found");
        if (lastPunc == 1) {
          //Serial.println("Period ok");
          lastPunc = 2;
        } else {
          //          //Serial.println("2 Excl or No prior comma");
          valid = false;
          break;
        }
      } else if (currentChar == ('-')) {
        //Serial.println("Hypen Found");
        if (receivedChars[q - 2] == ',') { //q incremented after value capture
          //          //Serial.println("Negative Sign ok");
        } else {
          //Serial.println("Hyphen in wrong place");
          valid = false;
          break;
        }
      } else if (currentChar == ('.')) {
        //Serial.println("period ok");
      } else {
        //Serial.println("Invalid Punc");
        valid = false;
        break;
      }
    } else if (isAlpha(currentChar)) {
      //Serial.println("Alpha");
      valid = false;
      break;
    } else if (isSpace(currentChar)) {
      //Serial.println("Space");
      valid = false;
      break;
    }

    //Detect no ending exclamation point
    if (q == receivedChars.length() - 1) {
      if (receivedChars[q] != '!') {
        //Serial.println("No Ending");
        valid = false;
        break;
      }
    }
    //Null Character in the middle
    if (currentChar == '\0' && q != receivedChars.length() - 1) {
      //Serial.println("null character");
      valid = false;
      break;
    }
  }
  if (valid) {
    //Serial.println("valid");
  }
  if (!valid) {
    //Serial.println("invalid");
  }
  return valid;
}



void popCommands() {
  //Process all the Incoming Commands
  long start = millis();

  while (cBuf.openSpot > 0 && millis() - start < manualTimeout) {
    if (cBuf.openSpot > 0) {

      //Serial.println (cBuf.openSpot - 1);
      int currentCommand[2] = {cBuf.commandStack[cBuf.openSpot - 1][0], cBuf.commandStack[cBuf.openSpot - 1][1]};
      cBuf.commandStack[cBuf.openSpot - 1][0] = -1;
      cBuf.commandStack[cBuf.openSpot - 1][1] = -1;
      cBuf.openSpot --;

      //Supported Commands
      switch (currentCommand[0]) {
        case (1):
          MSH.PrintStatus();
          int a;

          while (a < MSH.binOpenSpot) {

            Serial.print(MSH.binArr[a]);
            a++;
          }
          Serial.println("");
          Serial.print("binary array length: ");
          Serial.print(MSH.binOpenSpot / 8);
          Serial.println(" bytes");

          MSH.binOpenSpot = 0;


          break;

        case (2):
          Serial1.print(F("1,1!\n")); // <---found the secret to Serial1 :)
          break;

        case (3):
          Serial1.println(F("1,0!\n"));
          break;

        case (4):
          Serial.print("Magnetometer X:"); Serial.print(MSH.MagAve[0]);
          Serial.print(" Y:");  Serial.print(MSH.MagAve[1]);
          Serial.print(" Z:"); Serial.println(MSH.MagAve[2]);
          Serial.print("Gyroscope X:"); Serial.print(MSH.GyroAve[0]);
          Serial.print(" Y:");  Serial.print(MSH.GyroAve[1]);
          Serial.print(" Z:"); Serial.println(MSH.GyroAve[2]);
          Serial.print("Accelerometer X:"); Serial.print(MSH.AccelAve[0]);
          Serial.print(" Y:");  Serial.print(MSH.AccelAve[1]);
          Serial.print(" Z:"); Serial.println(MSH.AccelAve[2]);
          break;

        case (5):
          Serial.println(currentCommand[1]);
          break;

        case (20): // 20 downlink commands
          MSH.SRFreq = currentCommand[1];
          break;

        case (21):
          break;

        case (22):
          break;

        case (30): //toggle active fault //todo test this
          if (MSH.activeFaults[currentCommand[1]] = 0) {
            MSH.activeFaults[currentCommand[1]] = 1;
          } else {
            MSH.activeFaults[currentCommand[1]] = 0;
          }
          break;

        case (80): // 80 gyro commands
          MSH.XGyroThresh = currentCommand[1];
          MSH.StageReport = true;
          break;

        case (81):
          MSH.YGyroThresh = currentCommand[1];
          MSH.StageReport = true;
          break;

        case (82):
          MSH.ZGyroThresh = currentCommand[1];
          MSH.StageReport = true;
          break;

        case (90): // 90 accelerometer commands
          MSH.XAccelThresh = currentCommand[1];
          MSH.StageReport = true;
          break;

        case (91):
          MSH.YAccelThresh = currentCommand[1];
          MSH.StageReport = true;
          break;

        case (92):
          MSH.ZAccelThresh = currentCommand[1];
          MSH.StageReport = true;
          break;

        case (111): // slave commands (this terminology is really due for an update)
          MSH.Gyro[0] = currentCommand[1];
          break;

        case (101):
          MSH.Gyro[1] = currentCommand[1];
          break;

        case (102):
          MSH.Gyro[2] = currentCommand[1];
          break;

        case (103):
          MSH.Mag[0] = currentCommand[1];
          break;

        case (104):
          MSH.Mag[1] = currentCommand[1];
          break;

        case (105):
          MSH.Mag[2] = currentCommand[1];
          break;

        case (106):
          MSH.Accel[0] = currentCommand[1];
          break;

        case (107):
          MSH.Accel[1] = currentCommand[1];
          break;

        case (108):
          MSH.Accel[2] = currentCommand[1];
          break;

        case (109):
          SensUpdate = true;
          break;

        case (110):
          MSH.activeFaults[1] = 1; //trip fault protection for imu init failure
          break;
      }
    } else {
      Serial.println("No Command");
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////Setup & loop//////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


void setup() {
  //initialize
  Serial.begin (74880);
  Serial1.begin (74880);
  Serial3.begin (74880);
  //Wire.begin();
  cBuf = commandBuffer();
  buildFaults();
}

void loop() {
  ModeCon();
}





////////////////////////////////////////////////////////////////////////////
////////////////Mode Control////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void ModeCon() {
  z = millis();
//  faultCheck(); activate when fault check is ready

  switch (nextMode) {

    case 0:
      // normal ops
      MNormal();
      break;

    case 1:
      // detumble
      MDetumble();
      break;

    case 2:
      // safe hold
      MSafeHold();
      break;
  }
  // use to check cycle speed
  //  Serial.println("////////////////////////////////////////");
  //  Serial.print("cycle completed in ");
  //  Serial.print(millis() - z);
  //  Serial.println(" milliseconds");
  //  Serial.println("////////////////////////////////////////");


}

void  MNormal() {
  //  checkTime();
  popCommands();

  recvWithEndMarker();

  recvWithEndMarker1();

  //initializeRB();


  if (newData) {
    if (isInputValid() && (receivedChars != "")) {
      commandParse();
    }
    newData = false;
    //Serial.println("wiping serial buffer");
    receivedChars = "";
  }
  ////////////////////// for testing sensor averaging
  //  cBuf.commandStack[cBuf.openSpot][0] = 100;
  //  cBuf.commandStack[cBuf.openSpot][1] = random(0, 1000);
  //  cBuf.openSpot++;
  //
  //  cBuf.commandStack[cBuf.openSpot][0] = 101;
  //  cBuf.commandStack[cBuf.openSpot][1] =  random(0, 1000);
  //  cBuf.openSpot++;
  //
  //  cBuf.commandStack[cBuf.openSpot][0] = 102;
  //  cBuf.commandStack[cBuf.openSpot][1] =  random(0, 1000);
  //  cBuf.openSpot++;
  //
  //  cBuf.commandStack[cBuf.openSpot][0] = 109;
  //  cBuf.commandStack[cBuf.openSpot][1] = 2;
  //  cBuf.openSpot++;
  ///////////////////////

  // updateSensors();
  if (SensUpdate) {
    if (!IMUBufferFull) {
      fillIMUBuffer();
    }
    else {
      buildIMULog();
      takeAverage();
      cBuf.commandStack[cBuf.openSpot][0] = 4;
      cBuf.commandStack[cBuf.openSpot][1] = 1;
      cBuf.openSpot++;
      SensUpdate = false;
    }
  }
  //}

}


void MDetumble() {
  //Serial.println ("yay im detumbling");
  nextMode = 0;
}

void MSafeHold() {
  //Serial.println("im safe AF right now");
}

void faultCheck() {
  // reads fault string and checks that no threshold has been tripped

  int f = 0;
  bool fault = false;


  while (f < faultSize) {

    int CurInt = MSH.activeFaults[f];

    if (CurInt = 0) {
      //ignore
    } else {

      switch (f) {

        case (1):
          //Serial.println("case 1 tripped");
          if (MSH.Gyro[0] > MSH.XGyroThresh) {
            fault = true;
          }
          break;

        case (2):
          //Serial.println("case 2 tripped");
          break;
      }
    }
    f++;
  }
  if (fault) {
    nextMode = 2; // enter safe hold mode
  }
}

void buildFaults() {
  int f = 0;
  while (f < faultSize) {
    MSH.activeFaults[f] = 1;
    f++;
  }
}

void FaultString() { //use as binary int array todo
  int f = 0;
  String g = "";
  while (f < faultSize) {
    g += MSH.activeFaults[f];
    f++;
  }
  MSH.FaultString = g;
}

////////////////////////////////////////////////////////////////////////////
////////////Communication fucntions/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// rockblock code is basically finished do not want to integrate quite yet. maybe once we have imu up & running?
// do imu > teensy > Due > rockblock > webserivice transfer!

void stageDownLink() { //rework downlink todo
  long int i = micros();
  //  checkTime();
  String msg;
  MSH.lastSR++;

  if (MSH.lastSR > MSH.SRFreq) { // int can change based on how often you'd like special data.
    MSH.StageReport = true;
    FaultString(); // update currently active faults
  }
  if (MSH.StageReport) {
    String u = "";
    u += MSH.SpecialReport();
    msg += u;
    MSH.StageReport = false;
    MSH.lastSR = 0;
  }
  //  downLink(msg);

} void downLink() {
  int a; //binary array length counter
  int b; // checksum

  // at sbdwb short burst data: write binary
  // not finished
  while (a < MSH.binOpenSpot) {
    int c = MSH.binArr[a];
    if (c == 0) {
      Serial.print(0);
    } else if (c == 1) {
      Serial.print(1);
      b++;
    }
    a++;
  }
  Serial.print("asd");
  MSH.binOpenSpot = 0;

}

////////////////////////////////////////////////////////////////////////////
////////////////Sensor Functions////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


void checkTime() { //todo
  long timeNow = millis();
  String timeString = "";

  int days = timeNow / day ;//number of days
  int hours = (timeNow % day) / hour;
  int minutes = ((timeNow % day) % hour) / minute ;//and so on...
  int seconds = (((timeNow % day) % hour) % minute) / second;

  // digital clock display of current time
  //  Serial.print(days, DEC);
  //  printDigits(hours);
  //  printDigits(minutes);
  //  printDigits(seconds);
  //  Serial.println();

  timeString += String(days, DEC) + ":";
  timeString += String(hours, DEC) + ":";
  timeString += String(minutes, DEC) + ":";
  timeString += String(seconds, DEC);

  Serial.println(timeString);

}

void printDigits(byte digits) {
  // utility function for digital clock display: prints colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits, DEC);
}

void fillIMUBuffer() {
  if (SenseCounter <= SenseNumb) {

    for (int j = 0; j < 3; j++) {
      MSH.MagLog[j][SenseCounter] = MSH.Mag[j];
      MSH.GyroLog[j][SenseCounter] = MSH.Gyro[j];
      MSH.AccelLog[j][SenseCounter] = MSH.Accel[j];
    }
    SenseCounter++;
  }
  else {
    IMUBufferFull = true;
  }
  SensUpdate = false;
}

void takeAverage() {
  //take each value from the imu log and build average

  long int MAverage[3] = {0, 0, 0};
  long int GAverage[3] = {0, 0, 0};
  long int AAverage[3] = {0, 0, 0};

  for (int i = 0; i < SenseNumb; i++) {
    for (int j = 0; j < 3; j++) {

      MAverage[j] += MSH.MagLog[j][i];
      GAverage[j] += MSH.GyroLog[j][i];
      AAverage[j] += MSH.AccelLog[j][i];

    }//close for i

  }//close for j

  for (int j = 0; j < 3; j++) {
    MAverage[j] = MAverage[j] / SenseNumb;
    GAverage[j] = GAverage[j] / SenseNumb;
    AAverage[j] = AAverage[j] / SenseNumb;

    MSH.MagAve[j] = MAverage[j];
    MSH.GyroAve[j] = GAverage[j];
    MSH.AccelAve[j] = AAverage[j];

  }
}

void buildIMULog() {

  for (int i = (SenseNumb - 1); i >= 0; i--) {

    for (int j = 0; j < 3; j++) {
      if (i == 0) {
        MSH.MagLog[j][i] = MSH.Mag[j];
        MSH.GyroLog[j][i] = MSH.Gyro[j];
        MSH.AccelLog[j][i] = MSH.Accel[j];
      }
      if (i != 0) {
        MSH.MagLog[j][i] = MSH.MagLog[j][i - 1];
        MSH.GyroLog[j][i] = MSH.GyroLog[j][i - 1];
        MSH.AccelLog[j][i] = MSH.AccelLog[j][i - 1];
      }
    }//close for i

  }//close for j;

  // very helpful for debugging
  // Serial.print(MSH.GyroLog[0][0]); Serial.print(" "); Serial.print(MSH.GyroLog[1][0]); Serial.print(" "); Serial.println(MSH.GyroLog[2][0]) ;
  // Serial.print(MSH.GyroLog[0][1]); Serial.print(" "); Serial.print(MSH.GyroLog[1][1]); Serial.print(" "); Serial.println(MSH.GyroLog[2][1]) ;
  // Serial.print(MSH.GyroLog[0][2]); Serial.print(" "); Serial.print(MSH.GyroLog[1][2]); Serial.print(" "); Serial.println(MSH.GyroLog[2][2]) ;
  // Serial.print(MSH.GyroLog[0][3]); Serial.print(" "); Serial.print(MSH.GyroLog[1][3]); Serial.print(" "); Serial.println(MSH.GyroLog[2][3]) ;
  // Serial.print(MSH.GyroLog[0][4]); Serial.print(" "); Serial.print(MSH.GyroLog[1][4]); Serial.print(" "); Serial.println(MSH.GyroLog[2][4]) ;


}

// Feed into matt walch's code
//piksi data gps pos & rel pos. vel & rel vel. target spacecraft orbit. chec propellant



