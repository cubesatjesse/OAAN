
// Example 2 - Receive with an end-marker

const byte numChars = 32;
String receivedChars = "1426,11139!";   // an array to store the received data


boolean newData = false;

void setup() {
    Serial.begin(9600);
    Serial.println("<Arduino is ready>");
    delay(100);
    test();
}

void loop() {



}

String test() {
  
  Serial.print ("receivedChars;");
  Serial.println (receivedChars);

    int commandData;
  int commandType;
  String comRemaining = receivedChars;
  bool l = true;
//  Serial.println("//////////");
//  Serial.println(com);
//  com = receivedChars;
//  Serial.println("//////////");
//  Serial.println(com);
//  Serial.println("//////////");
//  Serial.println(receivedChars);
//  Serial.println("//////////");
  
  while (l) {
    commandType = (receivedChars.substring(0, receivedChars.indexOf(","))).toInt();
    commandData = (receivedChars.substring(receivedChars.indexOf(",") + 1, receivedChars.indexOf("!"))).toInt();
    Serial.println(commandType);
    Serial.println(commandData);
    if (receivedChars.indexOf("!") == receivedChars.length() - 1) {
      l = false;
      Serial.println(F("Finished Adding Commands"));
    } else {
      receivedChars = receivedChars.substring(receivedChars.indexOf("!") + 1);
    }
  }
}

