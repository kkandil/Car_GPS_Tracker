#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 5, TXPin = 6;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

void setup(){
  ss.begin(115200);
  Serial.begin(GPSBaud);
  
}

void loop(){
  // This sketch displays information every time a new sentence is correctly encoded.
 
  smartDelay(0);
  if (gps.location.isUpdated()){
      ss.print("Latitude= "); 
      ss.print(gps.location.lat(), 6);
      ss.print(" Longitude= "); 
      ss.print(gps.location.lng(), 6);
      
      //get number of satellites 
      ss.print(" satellites= "); 
      ss.println(gps.satellites.value());
      
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
    while (Serial.available())
      gps.encode(Serial.read());
  } while (millis() - start < ms);
}
