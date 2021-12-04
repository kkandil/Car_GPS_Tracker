/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

//#define ENABLE_DEBUG

// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
//#define BLYNK_HEARTBEAT 30

#include <TinyGPS++.h> //https://github.com/mikalhart/TinyGPSPlus
#include <TinyGsmClient.h> //https://github.com/vshymanskyy/TinyGSM
#include <BlynkSimpleTinyGSM.h> //https://github.com/blynkkk/blynk-library

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "pbwUBfcm1FXwo4xVZa78VVncV7IDt3if";

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "internet.vodafone.net";
char user[] = "internet";
char pass[] = "internet";


//GSM Module Settings
//GSM Module RX pin to ESP32 2
//GSM Module TX pin to ESP32 4
#define rxPin 3
#define txPin 2
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(3, 2);  // RX, TX
TinyGsm modem(SerialAT);

//GPS Module Settings
static const int RXPin = 5, TXPin = 6;
static const uint32_t GPSBaud = 9600; 
TinyGPSPlus gps; 
SoftwareSerial ss(RXPin, TXPin);


float latitude = 0.0, prevlatitude = 0.0;;
float longitude = 0.0, prevlongitude = 0.0;
float speed = 0.0, prevspeed;
float satellites = 0.0, prevsatellites;
bool is_gps_data_valid = false, is_prev_gps_data_valid = false;
float index = 0.0;
bool UpdateData = false;

WidgetMap myMap(V0);
BlynkTimer timer;



void setup() {
  
  //Set Serial monitor baud rate
  Serial.begin(9600);
#ifdef ENABLE_DEBUG
  Serial.println("esp32 serial initialize");
#endif
  delay(10);
  
  //Set GPS module baud rate
  //ss.begin(GPSBaud);
  //neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
#ifdef ENABLE_DEBUG
  Serial.println("neogps serial initialize");
#endif  
  delay(10);

  //Set GSM module baud rate
  
  SerialAT.begin(9600);
#ifdef ENABLE_DEBUG
  Serial.println("SIM800L serial initialize");
#endif
  delay(3000); 
   
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
#ifdef ENABLE_DEBUG
  Serial.println("Initializing modem...");
#endif

  modem.restart();
  //modem.init();
  
 
   
  Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
#ifdef ENABLE_DEBUG
  Serial.println("Blynk done...");
#endif
  timer.setInterval(100L, gpsUpdateTimer);
}
 
void loop() {

 
  
 
  Blynk.run();
  timer.run();
  
  

  
 
} //main loop ends



BLYNK_WRITE(V5)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 )
  {
     UpdateData = true;
  }
  
  // process received value
}



void gpsUpdateTimer()
{
  int pointIndex = 1;
  
  smartDelay(0);
  is_gps_data_valid = gps.location.isUpdated();
  if ( is_gps_data_valid )
  {
    latitude = (gps.location.lat());
    longitude = (gps.location.lng());
    //get
    speed = gps.speed.kmph();
    //get number of satellites
    satellites = gps.satellites.value();

    prevlatitude = latitude;
    prevlongitude = longitude;
    prevspeed = speed;
    prevsatellites = satellites;
    is_prev_gps_data_valid = true;
  }
  if( UpdateData == true )
  { 
    if ( is_gps_data_valid )
    { 
      Blynk.virtualWrite(V1, String(latitude, 6));
      Blynk.virtualWrite(V2, String(longitude, 6));
      Blynk.virtualWrite(V3, speed);
      Blynk.virtualWrite(V4, satellites);
      myMap.location(pointIndex, latitude, longitude, "GPS_Location");  
       Blynk.virtualWrite(V6, 1);   
  #ifdef ENABLE_DEBUG
      Serial.print("Latitude:  ");
      Serial.print(latitude, 6);
      Serial.print(" ,Longitude: ");
      Serial.print(longitude, 6);
      Serial.print(" ,Speed: ");
      Serial.println(speed, 6);  
     
  #endif     
    }
    else if( is_prev_gps_data_valid == true )
    {
      Blynk.virtualWrite(V1, String(prevlatitude, 6));
      Blynk.virtualWrite(V2, String(prevlongitude, 6));
      Blynk.virtualWrite(V3, prevspeed);
      Blynk.virtualWrite(V4, prevsatellites);
      myMap.location(pointIndex, prevlatitude, prevlongitude, "GPS_Location");  
      Blynk.virtualWrite(V7, 1);  
  #ifdef ENABLE_DEBUG
      Serial.print("Latitude:  ");
      Serial.print(prevlatitude, 6);
      Serial.print(" ,Longitude: ");
      Serial.print(prevlongitude, 6);
      Serial.print(" ,Speed: ");
      Serial.println(prevspeed, 6);   
  #endif      
    }
    else
    {
      Blynk.virtualWrite(V6, 0);
      Blynk.virtualWrite(V7, 0);  
    }
#ifdef ENABLE_DEBUG    
    Serial.println("data updated");
#endif    
    UpdateData = false;
  }
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
