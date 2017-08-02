//Things to put in MSH
bool AttemptingLink;
bool MessageStaged;
int RBCheckType = 0;
int LastMsgType;
int MOStatus;
int MTStatus;
unsigned long int lastRBchar; //not me
String receivedChars; //or me

void setup() {
  Serial.begin(19200);
  Serial2.begin(19200);

  delay(1000);
  //    Serial2.print(F("AT&F0\r")); //Reset to Factory Config
  //    Serial2.print(F("AT&K0\r")); //Disable Flow Control
  Serial2.print(F("ATE0\r")); //Disable Echo
  Serial2.print(F("AT+SBDD2\r")); //Clear Buffers
  Serial2.print(F("AT+SBDMTA=0\r")); //Disable RING alerts
  //initializeRB();

  Serial2.print(F("AT&W0\r")); //Set This as default configuration
  Serial2.print(F("AT&Y0\r")); //Set This as Power Up configuration
  // put your setup code here, to run once:

}

void loop() {
  Serial.println("loop");
  // put your main code here, to run repeatedly:
  delay(1000);
    if (millis() >= (lastRBchar + 3000)) {
  RBData();
    }
  if (receivedChars != "") {
    //Serial.println(receivedChars);
  }
  if (Serial.available() > 0) {
    routineDownlink();
    while (Serial.available() != 0) {
      Serial.read();
    }
  }
  switch (RBCheckType) { //msh
    case (0): //Ping RockBlock to ensure Unit Functionality
      //        if (!AttemptingLink) {
      //          //Serial.print("<R");
      //          Serial2.println("AT\r");
      //          rockOKParse();
      //        } else {
      //          Serial.print("<R2>"); //Waiting for SBDIX to return
      //        }
      break;
    case (1): //Ping Iridium and Check for Messages, Send if any are outgoing
      Serial.println("\nSBDIX Sent");
      sendSBDIX(true);
      RBCheckType = 0;
      break;
    case (2): //Fetch Incomming Command
      Serial2.print("AT+SBDRT\r");
      RBCheckType = 0;
  }
}

void downLink() {

}

bool responsePing() {
  Serial.println("responseping");
  bool ping = false;
  Serial2.print(F("AT\r"));
  if (rockOKParse()) {
    ping = true;
  }
  return ping;
}


void sendSBDIX(bool AL) {
  Serial.println("sendSBDIX");
  Serial2.print(F("AT+SBDIX\r")); // \r?
  if (AL) {
    AttemptingLink = true;
  }
}

String rocResponseRead() {
  Serial.println("rocRespRead");
  long start = millis();
  //Serial.print(Serial2.available());
  while (!Serial2.available() && (millis() - start > 6000));
  delay(10);
  String responseString = "";

  while (Serial2.available() > 0) {
    //NEED TO DETECT \r or Command End
    responseString += (char)Serial2.read();
    lastRBchar = millis();

    if (millis() - start > 4000) {
      Serial.println(F("Com Timeout"));
      break;
    }
  }
  //Serial.print(responseString);
  return responseString;
}

bool rockOKParse() {
  Serial.println("OKParse");
  delay(10);
  String input = rocResponseRead();
  Serial.println("#" + input + "#");
  bool valid = false;
  if (input[2] == 'O' && input[3] == 'K') {
    valid = true;
    Serial.println("valid");
  }
  return valid;
}




void RBData() {
  Serial.println("RBDATA");
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
    Serial.print("Swnumb:");
    Serial.println(swnumber);
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

        Serial.println(firstnumber.toInt());
        Serial.println(secondnumber.toInt());
        Serial.println(thirdnumber.toInt());
        Serial.println(fourthnumber.toInt());
        Serial.println(fifthnumber.toInt());
        Serial.println(sixthnumber.toInt());

        //Valid Command, Invalid -> false
        //      MSH.MOStatus = firstnumber.toInt();
        //      MSH.MOMSN = secondnumber.toInt();
        //      MSH.MTStatus = thirdnumber.toInt();
        //      MSH.MTMSN = fourthnumber.toInt();
        //      MSH.MTLength = fifthnumber.toInt();
        //      MSH.MTQueued = sixthnumber.toInt();

        //Safe to do here????
        AttemptingLink = false; //msh
        if (MTStatus == 1) { //Message Recieved by Iridium from RB
          MessageStaged = false; //msh
        } else {
          //Retry?
        }
        RBCheckType = 0; //Back to Idle //msh

        switch (MOStatus) {
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
      //      if (isInputValid()) {
      //        //buildBuffer(SBDRTDATA);
      //        popCommands();
      //      } else {
      //        //Invalid Uplink
      //        LastMsgType = 0; //MSH
      //        receivedChars = "";
      //      }
      //      break;
      case 3://SBDRING (Shouldn't be Possible)
        //Message is waiting the Buffer
        LastMsgType = 2; //MSH
        LastMsgType = 2; //MSH
        //return "";
        break;
      case 4: //OK
        LastMsgType = 1; //MSH
        LastMsgType = 1; //MSH
        //return "";
        break;
      case 5: // Error
        LastMsgType = 3; //MSH
        LastMsgType = 3; //MSH
        //return "";
        break;
      case 6: // blank msg //TODO
        LastMsgType = 0; //MSH
        //return "";
        break;
      case 7: // ready
        LastMsgType = 4; //MSH
        LastMsgType = 4; //MSH
        break;
      case 0: // invalid
        LastMsgType = 0; //MSH
        break;
        //0 = inval
        //1 = ok
        //2 = ring
        //3 = error
        //4 = ready
    }
  }

int routineDownlink() {
  Serial.println("RoutineDownlink");
  //Follow with SBDIX
  //0 is success, 1 is RB error, 2 is message is too long
  String DLS = "Hello World!";
  if (DLS.length() < 120) {
    Serial2.print(("AT+SBDWT=" + DLS + "\r"));
    delay(400);
    if (rockOKParse()) {
      MessageStaged = true;
      MOStatus = 5; //Reset so it can go to 0 when success (5 means nothing)
      Serial.print(F("\nRDL Staged for Downlink: "));
      RBCheckType = 1; //Send Staged Message
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
  Serial.println("initRB");
  bool initRB = true;
  Serial2.print(F("AT&F0\r")); //Reset to Factory Config
  Serial2.print(F("AT&K0\r")); //Disable Flow Control
  Serial2.print(F("ATE0\r")); //Disable Echo
  Serial2.print(F("AT+SBDD2\r")); //Clear Buffers
  Serial2.print(F("AT+SBDMTA=0\r")); //Disable RING alerts TODO this may be worth using. will reduce power consumption?
  if (initRB) {
    Serial2.print(F("AT&W0\r")); //Set This as default configuration
    Serial2.print(F("AT&Y0\r")); //Set This as Power Up configuration
  }
  delay(1500);
  int n = 18;
  if (!initRB) {
    n = 2;
  }
  Serial.print(Serial2.available());
  while (n > 0) {
    char c = (char)Serial2.read();
    Serial.print(c);
    n--;
  }
  return true;//responsePing(); //Ping to Check thats its working
}



