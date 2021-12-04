#include <SoftwareSerial.h>

String Arsp, Grsp;
SoftwareSerial gsm(3, 2); // RX, TX

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Testing GSM SIM800L");
  gsm.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:

  if(gsm.available())
  {
    Grsp = gsm.readString();
    Serial.println(Grsp);
  }

  if(Serial.available())
  {
    Arsp = Serial.readString();
    gsm.println(Arsp);
  }

}
