#include<HardwareSerial.h>

String Arsp, Grsp;
//SoftwareSerial gsm(3, 2); // RX, TX

 
void setup() {
  // put your setup code here, to run once:

  Serial1.begin(115200);
  Serial1.println("Testing GSM SIM800L");
  Serial2.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
 
 
  if(Serial2.available())
  {
    Grsp = Serial2.readString();
    Serial1.println(Grsp);
  }

  if(Serial1.available())
  {
    Arsp = Serial1.readString();
    Serial2.println(Arsp);
  }
 
}
