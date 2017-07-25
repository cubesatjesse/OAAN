
#include <Wire.h>

unsigned long manualTimeout = 10 * 1000;
int endT = 0;

boolean newData = false;
String receivedChars = "";

void setup() {
  //initialize
  Wire.begin();
  Serial.begin (9600);
  ////////////////////////////////////////////////////////////
  /////////////////Command functions//////////////////////////
  ////////////////////////////////////////////////////////////


  // this is all broken for no reason... fix tomorrow :)

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

/////////////////////////////////////////////////////////

void buildBuffer(String com) {
  //Check if incoming String <com> is valid set of commands and add it to the CommandBuffer
  int commandData;
  int commandType;
  String comRemaining = com;
  bool l = true;
  while (l) {
    commandType = (com.substring(0, com.indexOf(","))).toInt();
    commandData = (com.substring(com.indexOf(",") + 1, com.indexOf("!"))).toInt();
    cBuf.commandStack[cBuf.openSpot][0] = commandType;
    cBuf.commandStack[cBuf.openSpot][1] = commandData;
    if (com.indexOf("!") == com.length() - 1) {
      l = false;
      //Serial.println(F("Finished Adding Commands"));
    } else {
      com = com.substring(com.indexOf("!") + 1);
    }
    cBuf.openSpot++;
  }
}

//////////////////////////////////////////

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


/////////////////////////////////////////
// todo make timeout
//"1,1!,1!" breaks code
boolean isInputValid() {
  Serial.println("starting parse");
  //Check if incoming command string <input> is valid
  int lastPunc = 0; //1 if ",", 2 if "!", 0 Otherwise
  bool valid = true;
  int q = 0;
  int l = receivedChars.length();
  while (q < l) {
    char currentChar = receivedChars[q];
    q++;

    if (isPunct(currentChar)) {
      if (currentChar == (',')) {
        //Check if last was a period
        Serial.println("Comma Found");
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
///////////////////////////////////

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
      }
    } else {
      //Serial.println("No Command");
    }
  }
}
///////////////////////////////////
///////////////////////////////////

void loop() {

  //  wait 1 "tick" unit of time between sensor updates + new commands
  recvWithEndMarker();
  //if (receivedChars != "") {
  //Serial.print("receivedChars:");
  //Serial.println(receivedChars);
  if (newData) {
    if ((isInputValid()) && (receivedChars != "")) {
     //add to command buffer.
    }
    newData = false;
    receivedChars = "";
  }
  //showNewData();
  //ModeCon();


}





////////////////////////////////////////////////////////
////////////////Mode Control////////////////////////////
///////////////////////////////////////////////////////

void ModeCon() {
  //  if (masterstatus rotation != true) {

  //MDetumble();

  //MNormal();


}

void MDetumble() {

}

void MNormal() {

}

////////////////////////////////////////////////////////
////////////////Mode Control////////////////////////////
///////////////////////////////////////////////////////








