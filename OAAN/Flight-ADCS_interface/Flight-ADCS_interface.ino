void setup() {
Serial1.begin(38400);
Serial.begin(9600);
}

void loop() {

  String msg = "";
  while (Serial1.available() > 0) {
  char inchar = char(Serial1.read());
  msg += inchar;
  }
  Serial.print (msg);

}
