


unsigned long manualTimeout = 10 * 1000;
int endT = 0;

boolean newData = false;
String receivedChars = "4321,1234!1,32!6,-11!";
// makeshift command interpreter. read command parse note for better
// explaination. popcommands is up and running.


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

/////////////////////////////////////////////////////////

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
        case (2):
          Serial.println("working!");
          break;
      }
    } else {
      Serial.println("No Command");
    }
  }
}
///////////////////////////////////
///////////////////////////////////

void setup() {
  //initialize
  Serial.begin (9600);
  cBuf = commandBuffer();
}

void loop() {
  //  wait 1 "tick" unit of time between sensor updates + new commands

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








