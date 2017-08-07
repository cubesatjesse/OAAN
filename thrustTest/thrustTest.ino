//segun trust test
 int contPin = 13;
 int i;

void setup() {
pinMode(contPin,OUTPUT);

}

void loop() {

while (i > 3) { //change int to suit needs
  
digitalWrite(contPin, HIGH);
delay(250);
digitalWrite(contPin, LOW);
delay(1000); 

i++;
}

}
