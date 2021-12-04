#include <TinyGPS++.h>
 
static const int RXPin = 5, TXPin = 6;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

 

void setup(){
  Serial1.begin(115200);
  Serial3.begin(GPSBaud);
  
}

void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.


 
  smartDelay(0);
  if (gps.location.isUpdated()){
      Serial1.print("Latitude= "); 
      Serial1.print(gps.location.lat(), 6);
      Serial1.print(" Longitude= "); 
      Serial1.print(gps.location.lng(), 6);
      
      //get number of satellites 
      Serial1.print(" satellites= "); 
      Serial1.println(gps.satellites.value());
      
    }
 /*
  while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      Serial.print("Latitude= "); 
      Serial.print(gps.location.lat(), 6);
      Serial.print(" Longitude= "); 
      Serial.print(gps.location.lng(), 6);
      
      //get number of satellites 
      Serial.print(" satellites= "); 
      Serial.println(gps.satellites.value());
      
    }
  }
  */
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}
