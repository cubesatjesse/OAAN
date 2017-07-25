
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
      //Constructor
      State = 1; //normal ops
      DockingLeader = true;

      Gyro[3] = {0};
      Mag[3] = {0};
      Accel[3] = {0};
      ImuTemp = 0;
      Battery = 0; //todo i'd rather not have the default value trip fault detection 50%?

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


void loop() {
  
  }




void setup() {
  Serial.begin(9600);

}



