#include "Nokia_5110.h"
#define RST PA0
#define CE PA4
#define DC PA1
#define DIN PA7
#define CLK PA5
Nokia_5110 lcd = Nokia_5110(RST, CE, DC, DIN, CLK);


/* Comment this out to disable prints and save space */
//#define BLYNK_PRINT Serial1
#define BLYNK_PRINT lcd

#define ENABLE_DEBUG

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
TinyGsm modem(Serial2);

//GPS Module Settings
static const int RXPin = 5, TXPin = 6;
static const uint32_t GPSBaud = 9600; 
TinyGPSPlus gps; 


float latitude = 0.0, prevlatitude = 0.0;;
float longitude = 0.0, prevlongitude = 0.0;
float speed = 0.0, prevspeed;
float satellites = 0.0, prevsatellites;
bool is_gps_data_valid = false, is_prev_gps_data_valid = false;
bool UpdateData = false;

WidgetMap myMap(V0);
BlynkTimer timer;
WidgetLED data_valid_led(V6);
WidgetLED prev_data_valid_led(V7);
WidgetTerminal terminal(V10);

void setup() {
  
  //Set Serial monitor baud rate
  Serial1.begin(115200);

 
#ifdef ENABLE_DEBUG
  Serial1.println("esp32 serial initialize");
  lcd.println("esp32 serial initialize");
#endif
  delay(10);
  
  //Set GPS module baud rate
  //ss.begin(GPSBaud);
  Serial3.begin(9600);
  //neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
#ifdef ENABLE_DEBUG
  Serial1.println("neogps serial initialize");
  lcd.println("neogps serial initialize");
#endif  
  delay(10);

  //Set GSM module baud rate
  
  Serial2.begin(9600);
#ifdef ENABLE_DEBUG
  Serial1.println("SIM800L serial initialize");
  lcd.println("SIM800L serial initialize");
#endif
  delay(3000); 
 
   
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
#ifdef ENABLE_DEBUG
  Serial1.println("Initializing modem...");
#endif
  modem.restart();
  //modem.init();
  
  lcd.clear();
  Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
#ifdef ENABLE_DEBUG
  Serial1.println("Blynk done...");
#endif

  lcd.clear();
  lcd.print("Blynk done...");

  timer.setInterval(100L, gpsUpdateTimer);
}
 
void loop() {

 
  Blynk.run();
  timer.run();
 smartDelay(0);
} //main loop ends


int update_request_index = 0;
BLYNK_WRITE(V5)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 && UpdateData == false)
  {
     UpdateData = true;
#ifdef ENABLE_DEBUG
      Serial1.println("Update Requested"); 
#endif
    String ms = "Update Request: " + String(update_request_index);
    lcd.clear();
    lcd.print(ms);
    terminal.println(ms);
    terminal.flush();
    update_request_index++;
  }
  
  // process received value
}

bool is_auto_update = false;
int auto_update_duration = 1;
int auto_update_timer_counter = 0;
BLYNK_WRITE(V8)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
     auto_update_timer_counter = 0;
     is_auto_update = true; 
  }
  else
  {
    is_auto_update = false;
    auto_update_timer_counter = 0;
  }
}
BLYNK_WRITE(V9)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue > 0 && pinValue <= 60 && auto_update_duration!=pinValue )
  {
    auto_update_duration = pinValue;
  }
}

void gpsUpdateTimer()
{
  int pointIndex = 1; 

  
  
  is_gps_data_valid = gps.location.isUpdated();
  //Serial1.print("Data Valid = ");
  //Serial1.println(is_gps_data_valid);  
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
  if( UpdateData == true || (is_auto_update == true && auto_update_timer_counter >= (auto_update_duration*10)))
  { 
    auto_update_timer_counter = 0;
    
    if ( is_gps_data_valid )
    { 
      Blynk.virtualWrite(V1, String(latitude, 6));
      Blynk.virtualWrite(V2, String(longitude, 6));
      Blynk.virtualWrite(V3, speed);
      Blynk.virtualWrite(V4, satellites);
      myMap.location(pointIndex, latitude, longitude, "GPS_Location");  
      data_valid_led.on();
      prev_data_valid_led.off();     
  #ifdef ENABLE_DEBUG
      Serial1.println("GPS Data Valid");  
      Serial1.print("Latitude:  ");
      Serial1.print(latitude, 6);
      Serial1.print(" ,Longitude: ");
      Serial1.print(longitude, 6);
      Serial1.print(" ,Speed: ");
      Serial1.println(speed, 6);  
     
  #endif     
      lcd.clear();
      lcd.print("Valid");
      terminal.println("Valid");
    }
    else if( is_prev_gps_data_valid == true )
    {
      Blynk.virtualWrite(V1, String(prevlatitude, 6));
      Blynk.virtualWrite(V2, String(prevlongitude, 6));
      Blynk.virtualWrite(V3, prevspeed);
      Blynk.virtualWrite(V4, prevsatellites);
      myMap.location(pointIndex, prevlatitude, prevlongitude, "GPS_Location");
      data_valid_led.off();
      prev_data_valid_led.on();   
  #ifdef ENABLE_DEBUG
      Serial1.println("Prev GPS Data Valid");  
      Serial1.print("Latitude:  ");
      Serial1.print(prevlatitude, 6);
      Serial1.print(" ,Longitude: ");
      Serial1.print(prevlongitude, 6);
      Serial1.print(" ,Speed: ");
      Serial1.println(prevspeed, 6);   
  #endif      
      lcd.clear();
      lcd.print("Prev Valid");
      terminal.println("Prev Valid");
    }
    else
    {
#ifdef ENABLE_DEBUG
      Serial1.println("No Data Valid");  
#endif   
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V2, 0);
      Blynk.virtualWrite(V3, 0);
      Blynk.virtualWrite(V4, 0);   
      data_valid_led.off();
      prev_data_valid_led.off();
      lcd.clear();
      lcd.print("No Data");
      terminal.println("No Data");
    }
#ifdef ENABLE_DEBUG    
    Serial1.println("data updated");
#endif    
    UpdateData = false;
  } 
  else if( is_auto_update == true )
  {
    auto_update_timer_counter++;
  }
  terminal.flush();
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
