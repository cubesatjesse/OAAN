


unsigned long manualTimeout = 10 * 1000;
int endT = 0;

boolean newData = false;
String receivedChars = "4321,1234!1,32!6,-11!";
// makeshift command interpreter. read command parse note for better
// explaination. popcommands is up and running.


////////////////////////////////////////////////////////////
/////////////////Database of parameters/////////////////////
////////////////////////////////////////////////////////////

class masterStatus {
    //Class to hold entire State of Spacecraft Operation except timers
  public:
    // To others looking at this code:
    //it is important that as few of these variables as possible get arbitrarily assigned a value.
    // these should only be given real data

    // change final string to binary. get bytes. find upper and lower limits. round floats and set value for maxes (like MAX)

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

    // todo accumulated or raw data?
    //  get all the min max vals/percision
    float Mag[3];
    float Gyro[3];
    float Accel[3];
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
      output += "Rs:" + String(Faults) + ",";
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

};
masterStatus MSH; //Declare MSH






////////////////////////////////////////////////////////////
/////////////////Command functions//////////////////////////
////////////////////////////////////////////////////////////



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
  // to the best if my understanding, theres no (properly formatted) command
  // stream that will break the code. it appears that something very
  // low level is broken/breaking... idk what there is to be done about that
  // i may need to put it on a shelf untill a clearer solution appears :(
  // serial print statements for debugging

  Serial.print ("receivedChars:");
  Serial.println (receivedChars);

  int commandData;
  int commandType;
  bool l = true;
  String i = "";
  i = receivedChars;

  while (l) {
    Serial.print ("i:");
    Serial.println (i);
    Serial.println (receivedChars);
    commandType = (receivedChars.substring(0, receivedChars.indexOf(","))).toInt();
    Serial.println (receivedChars);
    Serial.print ("i:");
    Serial.println (i);
    commandData = (receivedChars.substring(receivedChars.indexOf(",") + 1, receivedChars.indexOf("!"))).toInt();
    Serial.println (receivedChars);
    Serial.print("commandType:");
    Serial.println(commandType);
    Serial.print("commandData:");
    Serial.println(commandData);
    Serial.println("********* ind !");
    Serial.println(receivedChars.indexOf("!"));
    Serial.println("********* lenght");
    Serial.println(receivedChars.length() - 1);
    Serial.println("*********");
    if (receivedChars.indexOf("!") == receivedChars.length() - 1) {
      l = false;
      Serial.println(F("Finished Adding Commands"));
    } else {
      Serial.print ("rc bef:");
      Serial.println (receivedChars);
      receivedChars = receivedChars.substring(receivedChars.indexOf("!") + 1);
      i = receivedChars;
      Serial.print ("rc aft:");
      Serial.println (receivedChars);

    }
  }
}



void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (rc != endMarker) {
      receivedChars += (char(rc));
    }
    else {
      newData = true;
    }
  }
}



void showNewData() {
  if (newData == true) {
    Serial.print("This just in ... ");
    Serial.println(receivedChars);
    receivedChars = "";
    newData = false;
  }
}



boolean isInputValid() {
  // todo make timeout
  Serial.println("starting parse");
  //Check if incoming command string <input> is valid
  int lastPunc = 0; //1 if ",", 2 if "!", 0 Otherwise
  bool valid = true;
  int q = 0;
  int l = receivedChars.length();
  while (q < l) {
    char currentChar = receivedChars[q];
    q++;

    if (l < 4) {
      Serial.println("command too short");
      valid = false;
    }
    if (isPunct(currentChar)) {
      if (currentChar == (',')) {
        //Check if last was a period
        Serial.println("Comma Found");
        if (receivedChars[q - 2] == '!') {
          Serial.println("No First Command Number");
          valid = false;
          break;
        }
        if (lastPunc == 0 || lastPunc == 2) {
          Serial.println("Comma OK");
          lastPunc = 1;
        } else {
          Serial.println("2 Commas");
          valid = false;
          break;
        }
      } else if (currentChar == ('!')) {
        if (receivedChars[q - 2] == ',') {
          Serial.println("No Second Command Number");
          valid = false;
          break;
        }
        Serial.println("Excl Found");
        if (lastPunc == 1) {
          Serial.println("Period ok");
          lastPunc = 2;
        } else {
          Serial.println("2 Excl or No prior comma");
          valid = false;
          break;
        }
      } else if (currentChar == ('-')) {
        Serial.println("Hypen Found");
        if (receivedChars[q - 2] == ',') { //q incremented after value capture
          Serial.println("Negative Sign ok");
        } else {
          Serial.println("Hyphen in wrong place");
          valid = false;
          break;
        }
      } else {
        Serial.println("Invalid Punc");
        valid = false;
        break;
      }
    } else if (isAlpha(currentChar)) {
      Serial.println("Alpha");
      valid = false;
      break;
    } else if (isSpace(currentChar)) {
      Serial.println("Space");
      valid = false;
      break;
    }

    //Detect no ending exclamation point
    if (q == receivedChars.length() - 1) {
      if (receivedChars[q] != '!') {
        Serial.println("No Ending");
        valid = false;
        break;
      }
    }
    //Null Character in the middle
    if (currentChar == '\0' && q != receivedChars.length() - 1) {
      Serial.println("null character");
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
          Serial.println((currentCommand[1]));
          break;
        case (2):
          Serial.println("working!");
          break;
      }
    } else {
      Serial.println("No Command");
    }
  }
}


////////////////////////////////////////////////////////////
//////////////////////Setup & loop//////////////////////////
////////////////////////////////////////////////////////////


void setup() {
  //initialize
  Serial.begin (9600);
  cBuf = commandBuffer();
}

void loop() {
  //  wait 1 "tick" unit of time between sensor updates + new commands
  MSH.OutputString();

  if (Serial.read() != -1 ) {
    // VERY temporary
    //set whatever commandType/Data you need
    int commandType = 2;
    int commandData = 456;
    cBuf.commandStack[cBuf.openSpot][0] = commandType;
    cBuf.commandStack[cBuf.openSpot][1] = commandData;
    cBuf.openSpot++;
  }
  popCommands();

  ///////////////////Under construction/////////////////////////
  //  recvWithEndMarker();
  //  if (newData) {
  //    if ((isInputValid()) && (receivedChars != "")) {
  //      Serial.print("receivedChars:<");
  //      Serial.print(receivedChars);
  //      Serial.println(">");
  //      commandParse();
  //    }
  //    newData = false;
  //    Serial.println("wiping serial buffer");
  //    receivedChars = "";
  //  }
  ////////////////////////////////////////////////////////////

}





////////////////////////////////////////////////////////
////////////////Mode Control////////////////////////////
////////////////////////////////////////////////////////

void ModeCon() {
  if (checkSpin() == true) {

  }

  //MDetumble();

  //MNormal();


}

void MDetumble() {

}

void MNormal() {

}


////////////////////////////////////////////////////////
////////////////Sensor Functions////////////////////////
////////////////////////////////////////////////////////


bool checkSpin() {


}


//return true if spin rate exceeds threshold

