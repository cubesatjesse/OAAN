void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Serial3.begin (115200);
}

void loop() {
delay(10);
//if (Serial3.available()) {

  Serial.println(Serial3.read());
 // Serial3.read();
//}


}
