///////// need Serial code to be decimal friendy
///////// clean up IMU stuff


//!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/
//!/!/!/!/!/!/!/Dont use still WIP!/!/!/!/!/!/!/!/
//!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/!/

unsigned long manualTimeout = 10 * 1000;
int endT = 0;

const int SenseNumb = 5; //change number of data samples in averaging array
bool SensUpdate = false; //is there fresh IMU data
bool IMUBufferFull = false; //must fill w/ data before averages can be taken
int SenseCounter = 0;

bool WireConnected = true;
boolean newData = false;
String receivedChars = "4321,1234!1,32!6,-11!";
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

    // change final string to binary. get bytes.
    // find upper and lower limits. round floats and set value for maxes (like MAX)

    int State;
    int NextState;
    int Faults; //number of faults detected over satellite lifetime
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


    // todo accumulated or raw data?
    //  get all the min max vals/percision
    float Mag[3];
    float Gyro[3];
    float Accel[3];

    float MagAve[3];
    float GyroAve[3];
    float AccelAve[3];

    float MagLog[3][SenseNumb];
    float GyroLog[3][SenseNumb];
    float AccelLog[3][SenseNumb];

    int TempAcc;
    int ImuTemp;
    // may need to store the zeros aswell.

    //power board variables
    //todo is there any more variables available that i may want?
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

    String toString() {
      //Produces JSON Output in ASCII for Printing and Downlink
      String output = "";
      output += "{";
      output += "S:" + String(State) + ",";
      output += "GX:" + String(Gyro[0]) + ",GY:" + String(Gyro[1]) + ",GZ:" + String(Gyro[2]) + ",";
      output += "MX:" + String(Mag[0]) + ",MY:" + String(Mag[1]) + ",MZ:" + String(Mag[2]) + ",";
      output += "AX:" + String(Accel[0]) + ",AY:" + String(Accel[1]) + ",AZ:" + String(Accel[2]) + ",";
      output += "IT:" + String(ImuTemp) + ",";
      output += "B:" + String(Battery) + ",";
      output += "Fl:" + String(Faults) + ",";
      output += "TXD:" + String(TorqueXDir) + ",";
      output += "TXP:" + String(TorqueXPWM) + ",";
      output += "TYD:" + String(TorqueYDir) + ",";
      output += "TYP:" + String(TorqueYPWM) + ",";
      output += "TZD:" + String(TorqueZDir) + ",";
      output += "TZP:" + String(TorqueZPWM) + ",";
      //todo should motor "intensity" be RPM?
      output += "MXD:" + String(MotorXDir) + ",";
      output += "MXR:" + String(MotorXRPM) + ",";
      output += "MYD:" + String(MotorYDir) + ",";
      output += "MYR:" + String(MotorYRPM) + ",";
      output += "MZD:" + String(MotorZDir) + ",";
      output += "MZR:" + String(MotorZRPM) + "}";
      return output;
    }

    float roundDecimal(float num, int places) {
      int roundedNum = round(pow(10, places) * num);
      return roundedNum / ((float)(pow(10, places)));
    }

    String chop(float num, int p) {
      String s = String(num);
      if (p == 0) {
        return s.substring(0, s.indexOf('.'));
      }
      return s.substring(0, s.indexOf('.') + p + 1);
    }

    String OutputString() {
      //round floats first
      //(round(),2)
      //constrain
      //getbytes for loop
      String OutputString = "";
      // todo update constrain values
      OutputString += chop(constrain(roundDecimal(Gyro[0], 1), -245, 245), 1) + ",";
      OutputString += chop (constrain(roundDecimal(Gyro[1], 1), -245, 245), 1) + ",";
      OutputString += chop (constrain(roundDecimal(Gyro[2], 1), -245, 245), 1) + ",";
      OutputString += chop (constrain(roundDecimal(Mag[0], 2), -2, 2), 2) + ",";
      OutputString += chop (constrain(roundDecimal(Mag[1], 2), -2, 2), 2) + ",";
      OutputString += chop (constrain(roundDecimal(Mag[2], 2), -2, 2), 2) + ",";
      OutputString += chop (constrain(roundDecimal(ImuTemp, 0), -60, 90), 0) + ",";
      OutputString += chop (constrain(roundDecimal(Battery, 2), 2.75, 4.2), 2) + ",";

      OutputString += chop (constrain(roundDecimal(TorqueXDir, 0), -1, 1), 0) + ",";
      OutputString += chop (constrain(roundDecimal(TorqueXPWM, 2), 0, 4), 2) + ",";
      OutputString += chop (constrain(roundDecimal(TorqueYDir, 0), -1, 1), 0) + ",";
      OutputString += chop (constrain(roundDecimal(TorqueYPWM, 2), 0, 4), 2) + ",";
      OutputString += chop (constrain(roundDecimal(TorqueZDir, 0), -1, 1), 0) + ",";
      OutputString += chop (constrain(roundDecimal(TorqueZPWM, 2), 0, 4), 2) + ",";
      OutputString += chop (constrain(roundDecimal(MotorXDir, 0), -1, 1), 0) + ",";
      OutputString += chop (constrain(roundDecimal(MotorXRPM, 2), 0, 4), 2) + ",";
      OutputString += chop (constrain(roundDecimal(MotorYDir, 0), -1, 1), 0) + ",";
      OutputString += chop (constrain(roundDecimal(MotorYRPM, 2), 0, 4), 2) + ",";
      OutputString += chop (constrain(roundDecimal(MotorZDir, 0), -1, 1), 0) + ",";
      OutputString += chop (constrain(roundDecimal(MotorZRPM, 2), 0, 4), 2) + ",";
      //      if string length less thatn max number add random symbols until it is max length

      //      for (int i = 109 - OutputString.length(); i <= (109 - OutputString.length() + 1); i++) {
      //        OutputString[i] = "#";
      //      }
      //
      //      byte DLBIN[OutputString.length()];
      //      OutputString.getBytes(DLBIN, OutputString.length());
      //      Serial.print(OutputString);
      //      for (int i = 0; i < OutputString.length() - 1; i++) {
      //        Serial.print("00");
      //        Serial.print(DLBIN[i], BIN);
      //        Serial.print(" ");
      //      }
      Serial.println(OutputString);
      return OutputString;
    }


    String SpecialReport() {
      // all fixed values
      String output = "";
      output += "{";
      output += "Fl:" + String(Faults) + ",";
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
  int commandType;
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
  // for serial0
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
  ////////////////////////////////////////////
  Serial.print("Serial available:");
  Serial.println(Serial1.available());
  if (newData == true) {
    Serial.println("Newdata = true");
  } else {
    Serial.println("Newdata = false");
  }


  ////////////////////////////////////////////

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

boolean isInputValid() {
  // todo make timeout
  //Serial.println("starting parse");
  //Check if incoming command string <input> is valid
  int lastPunc = 0; //1 if ",", 2 if "!", 0 Otherwise
  bool valid = true;
  int q = 0;
  int l = receivedChars.length();
  while (q < l) {
    char currentChar = receivedChars[q];
    q++;

    if (l < 4) {
      //      Serial.println("command too short");
      valid = false;
    }
    if (isPunct(currentChar)) {
      if (currentChar == (',')) {
        //Check if last was a period
        //        Serial.println("Comma Found");
        if (receivedChars[q - 2] == '!') {
          //          Serial.println("No First Command Number");
          valid = false;
          break;
        }
        if (lastPunc == 0 || lastPunc == 2) {
          //          Serial.println("Comma OK");
          lastPunc = 1;
        } else {
          //          Serial.println("2 Commas");
          valid = false;
          break;
        }
      } else if (currentChar == ('!')) {
        if (receivedChars[q - 2] == ',') {
          //          Serial.println("No Second Command Number");
          valid = false;
          break;
        }
        //        Serial.println("Excl Found");
        if (lastPunc == 1) {
          //          Serial.println("Period ok");
          lastPunc = 2;
        } else {
          //          Serial.println("2 Excl or No prior comma");
          valid = false;
          break;
        }
      } else if (currentChar == ('-')) {
        //        Serial.println("Hypen Found");
        if (receivedChars[q - 2] == ',') { //q incremented after value capture
          //          Serial.println("Negative Sign ok");
        } else {
          //          Serial.println("Hyphen in wrong place");
          valid = false;
          break;
        }
      } else if (currentChar == ('.')) {
        //        Serial.println("period ok");
      } else {
        Serial.println("Invalid Punc");
        valid = false;
        break;
      }
    } else if (isAlpha(currentChar)) {
      //      Serial.println("Alpha");
      valid = false;
      break;
    } else if (isSpace(currentChar)) {
      //      Serial.println("Space");
      valid = false;
      break;
    }

    //Detect no ending exclamation point
    if (q == receivedChars.length() - 1) {
      if (receivedChars[q] != '!') {
        //        Serial.println("No Ending");
        valid = false;
        break;
      }
    }
    //Null Character in the middle
    if (currentChar == '\0' && q != receivedChars.length() - 1) {
      //      Serial.println("null character");
      valid = false;
      break;
    }
  }
  if (valid) {
    Serial.println("valid");
  }
  if (!valid) {
    Serial.println("invalid");
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
          Serial.print(MSH.toString());
          break;

        case (2):
          Serial1.print(F("2,3!\n")); // <---found the secret to Serial1 :)
          break;

        case (3):
          Serial1.println("5,3!");
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

        case (100): // slave commands (this terminology is really due for an update)
          MSH.Gyro[0] = currentCommand[1];
          Serial.println(currentCommand[1]);
          break;

        case (101):
          MSH.Gyro[1] = currentCommand[1];
          Serial.println(MSH.Gyro[1]);
          break;

        case (102):
          MSH.Gyro[2] = currentCommand[1];
          Serial.println(MSH.Gyro[2]);
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
  Serial.begin (19200);
  Serial1.begin (74880);
  Serial2.begin (19200);
  //Wire.begin();
  cBuf = commandBuffer();
}

void loop() {
  //  wait 1 "tick" unit of time between sensor updates + new commands
  popCommands();
  recvWithEndMarker();
  //recvWithEndMarker1();
  //tenatively working
  //initializeRB();


  if (newData) {
    Serial.print("receivedChars:");
    Serial.println(receivedChars);
    if (isInputValid() && (receivedChars != "")) {
      commandParse();
    }
    newData = false;
    Serial.println("wiping serial buffer");
    receivedChars = "";
  }

  //updateSensors();


}





////////////////////////////////////////////////////////////////////////////
////////////////Mode Control////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void ModeCon() {


}
  //MDetumble();

void  MNormal() {
    if (MSH.MOStatus == 0 || MSH.currentSegment == 0) {
      //Attempt Segment Downlink
      if (MSH.currentSegment != 0) {
        Serial.println("\nSegment Downlink Successful");
        MSH.SBDIXFails = 0;
      }
      MSH.currentSegment++;

    } else {
      if (downlinkJustStaged) {
        //SBDIX Call after SBDWB
        Serial.println("\nSBDIX Sent");
        sendSBDIX(true); //Attempting Link is true after this
        linkTime = millis();
        downlinkJustStaged = false;
      }

      if (millis() - lastRBCheck > RBCheckTime) {
        Serial.print("<R2>");
        Serial.print("<F:" + String(MSH.SBDIXFails) + ">");
        RBData(); //Can Reset MOStatus to 0
        lastRBCheck = millis();
        //Check if SBDIX failed
        if ((!MSH.AttemptingLink) && (MSH.MOStatus != 0)) { //SBDIX failed -> Retry
          downlinkJustStaged = true;
          Serial.print(F("\nSegment Downlink Failed. Retrying: "));
          MSH.SBDIXFails++;
          Serial.println(MSH.SBDIXFails);
        }
      }
      if (MSH.SBDIXFails > 8) { //TODO set num of fails = RBCheckTime*100 ~20min
        MSH.currentSegment--;
        Serial.println(F("\nImage Downlink Failed due to no SBDIX Link"));
        MSH.NextState = NORMAL_OPS;
      }
    }
  } else {
    MSH.NextState = NORMAL_OPS;
  }
  //TODO Timeout
  //if(timeout){
  //    MSH.currentSegment--
}




void MDetumble() {

}

void MNormal() {

}





////////////////////////////////////////////////////////////////////////////
////////////Communication fucntions/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void stageDownLink() {
  long int i = micros();
  String msg = (MSH.toString()); // do i want to constrain values? maybe not
  MSH.lastSR++;

  if (MSH.lastSR > MSH.SRFreq) { // int can change based on how often you'd like special data.
    MSH.StageReport = true;
  }
  if (MSH.StageReport) {
    String u = "";
    u += MSH.SpecialReport();
    msg += u;
    MSH.StageReport = false;
    MSH.lastSR = 0;
  }
  //  downLink(msg);

}


//*********************************************
//****delete below if not using rockblock******
//*********************************************

// RockBlock Uplink/Downlink Functions

int curComL = 0;

String rocResponseRead() {
  long start = millis();
  //Serial.print(Serial2.available());
  while (!Serial2.available() && (millis() - start > 6000));
  delay(10);
  String responseString = "";

  while (Serial2.available() > 0) {
    //NEED TO DETECT \r or Command End
    responseString += (char)Serial2.read();

    if (millis() - start > 4000) {
      Serial.println(F("Com Timeout"));
      break;
    }
  }
  Serial.print(responseString);
  return responseString;
}

bool rockOKParse() {
  delay(10);
  String input = rocResponseRead();
  Serial.println("#" + input + "#");
  bool valid = false;
  if (input[2] == 'O' && input[3] == 'K') {
    valid = true;
  }
  return valid;
}

void RBData() {
  int swnumber = 0;
  String ReceivedMessage = rocResponseRead(); //determines case

  //TODO Need to Slice if two commands recieved?

  Serial.print("<M:" + ReceivedMessage + ">");
  Serial.print("ReceivedMessage:");
  Serial.println(ReceivedMessage);
  int plus = ReceivedMessage.indexOf('+');
  int colon = ReceivedMessage.indexOf(':');
  int S_SBDRING = ReceivedMessage.indexOf('S');
  int E_ERROR = ReceivedMessage.indexOf('E');
  int R_ERROR = ReceivedMessage.lastIndexOf('R');
  int O_OK = ReceivedMessage.indexOf('O');
  int K_OK = ReceivedMessage.indexOf('K');
  int space = ReceivedMessage.indexOf(' ');
  int carReturn = ReceivedMessage.lastIndexOf('\r');
  int R_READY = ReceivedMessage.indexOf('R');
  int Y_READY = ReceivedMessage.lastIndexOf('Y');

  String Ring;
  String OK;
  String error;
  String nomessage;
  String invalid;
  int LengthOfMessage = ReceivedMessage.length();
  Serial.print("Length of Message:");
  Serial.println(ReceivedMessage.length());
  Serial.print("Substring:");
  Serial.println(ReceivedMessage.substring(plus, colon));

  if (ReceivedMessage.substring(plus, colon).equals(F("+SBDIX"))) {
    swnumber = 1;
  }
  else if (ReceivedMessage.substring(plus, colon).equals(F("+SBDRT"))) {
    swnumber = 2;
  }
  else if (ReceivedMessage.substring(S_SBDRING).equals(F("SBDRING"))) {
    swnumber = 3;
  }
  else if (ReceivedMessage.substring(E_ERROR, R_ERROR).equals(F("ERRO"))) {
    swnumber = 5;
  }
  else if (ReceivedMessage.substring(R_READY, Y_READY).equals(F("READ"))) {
    swnumber = 7;
  }
  else if (ReceivedMessage.length() == 0) {
    swnumber = 6;
  }
  else if (ReceivedMessage.substring(O_OK, K_OK + 1).equals(F("OK"))) {
    if (swnumber == 0) {
      swnumber = 4;
    }
  } else {
    swnumber = 0;
  }

  String DATA = ReceivedMessage.substring(colon + 1);
  int DATALength = DATA.length();
  String SBDRTDATA = DATA.substring(2, (DATALength - 6));
  int SBDRTDATALength = SBDRTDATA.length();

  int firstcomma, secondcomma, thirdcomma, fourthcomma, fifthcomma;
  String firstnumber, secondnumber, thirdnumber, fourthnumber, fifthnumber, sixthnumber;

  switch (swnumber) {
    case 1: //SBDIX command
      Serial.println("case 1");
      firstcomma  = DATA.indexOf(',');
      secondcomma = DATA.indexOf(',', firstcomma + 1);
      thirdcomma = DATA.indexOf(',', secondcomma + 1);
      fourthcomma = DATA.indexOf(',', thirdcomma + 1);
      fifthcomma = DATA.indexOf(',', fourthcomma + 1);
      firstnumber = DATA.substring(1, firstcomma);
      Serial.println(firstnumber);
      secondnumber = DATA.substring(firstcomma + 2, secondcomma);
      Serial.println(secondnumber);
      thirdnumber = DATA.substring(secondcomma + 2, thirdcomma);
      Serial.println(thirdnumber);
      fourthnumber = DATA.substring(thirdcomma + 2, fourthcomma);
      Serial.println(fourthnumber);
      fifthnumber = DATA.substring(fourthcomma + 2, fifthcomma);
      Serial.println(fifthnumber);
      sixthnumber = DATA.substring(fifthcomma + 2, LengthOfMessage - 17);
      Serial.print("sixthnumber:");
      Serial.println(sixthnumber);
      //Valid Command, Invalid -> false
      MSH.MOStatus = firstnumber.toInt();
      MSH.MOMSN = secondnumber.toInt();
      MSH.MTStatus = thirdnumber.toInt();
      MSH.MTMSN = fourthnumber.toInt();
      MSH.MTLength = fifthnumber.toInt();
      MSH.MTQueued = sixthnumber.toInt();

      //Safe to do here????
      MSH.AttemptingLink = false;
      if (MSH.MTStatus == 1) { //Message Recieved by Iridium from RB
        MSH.MessageStaged = false;
      } else {
        //Retry?
      }
      MSH.RBCheckType = 0; //Back to Idle

      switch (MSH.MOStatus) {
        case (32):
          Serial.println("No network service, unable to initiate call");
          break;
        case (33):
          Serial.println("Antenna fault, unable to initiate call");
          break;
        case (0):
          Serial.println("Message Sent Successfully");
          break;
      }

      break;

    case 2: //SBDRT command
      //Valid Command From Ground
      receivedChars += SBDRTDATA;
      if (isInputValid()) {
        //buildBuffer(SBDRTDATA);
        popCommands();
      } else {
        //Invalid Uplink
        MSH.LastMsgType = 0;
        receivedChars = "";
      }
      break;
    case 3://SBDRING (Shouldn't be Possible)
      //Message is waiting the Buffer
      MSH.LastMsgType = 2;
      MSH.LastSMsgType = 2;
      //return "";
      break;
    case 4: //OK
      MSH.LastMsgType = 1;
      MSH.LastSMsgType = 1;
      //return "";
      break;
    case 5: // Error
      MSH.LastMsgType = 3;
      MSH.LastSMsgType = 3;
      //return "";
      break;
    case 6: // blank msg //TODO
      MSH.LastMsgType = 0;
      //return "";
      break;
    case 7: // ready
      MSH.LastMsgType = 4;
      MSH.LastSMsgType = 4;
      break;
    case 0: // invalid
      MSH.LastMsgType = 0;
      break;
      //0 = inval
      //1 = ok
      //2 = ring
      //3 = error
      //4 = ready
  }
}

int lastRBcheck = 0;

bool responsePing() {
  bool ping = false;
  Serial2.print(F("AT\r"));
  if (rockOKParse()) {
    ping = true;
  }
  return ping;
}

void sendSBDIX(bool AL) {
  Serial2.print(F("AT+SBDIX\r")); // \r?
  if (AL) {
    MSH.AttemptingLink = true;
  }
}


void print_binary(int v, int num_places) {
  int mask = 0, n;
  for (n = 1; n <= num_places; n++) {
    mask = (mask << 1) | 0x0001;
  }
  v = v & mask;  // truncate v to specified number of places
  while (num_places) {
    if (v & (0x0001 << num_places - 1)) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }
    --num_places;
  }
}

int routineDownlink() {
  //Follow with SBDIX
  //0 is success, 1 is RB error, 2 is message is too long
  String DLS = MSH.OutputString();
  if (DLS.length() < 120) {
    Serial2.print(("AT+SBDWT=" + DLS + "\r"));
    delay(400);
    if (rockOKParse()) {
      MSH.MessageStaged = true;
      MSH.MOStatus = 5; //Reset so it can go to 0 when success (5 means nothing)
      Serial.print(F("\nRDL Staged for Downlink: "));
      MSH.RBCheckType = 1; //Send Staged Message
      return 0;
    } else {
      Serial.print(F("\nRDL Failed, RB Error: "));
      return 1;
    }
  } else {
    Serial.print(F("\nRDL Failed, Message Too Long: "));
    return 2;
  }
}

bool initializeRB() {
  bool init = true;
  //Serial2.print(F("AT&F0\r")); //Reset to Factory Config
  Serial2.print(F("AT&K0\r")); //Disable Flow Control
  Serial2.print(F("ATE0\r")); //Disable Echo
  Serial2.print(F("AT+SBDD2\r")); //Clear Buffers
  Serial2.print(F("AT+SBDMTA=0\r")); //Disable RING alerts
  if (init) {
    Serial2.print(F("AT&W0\r")); //Set This as default configuration
    Serial2.print(F("AT&Y0\r")); //Set This as Power Up configuration
  }
  delay(1500);
  int n = 18;
  if (!init) {
    n = 2;
  }
  Serial.print(Serial2.available());
  while (n > 0) {
    char c = (char)Serial2.read();
    Serial.print(c);
    n--;
  }
  return true; responsePing(); //Ping to Check thats its working
}

//*********************************************
//****delete above if not using rockblock******
//*********************************************


