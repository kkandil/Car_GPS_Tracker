#include "BLYNK.h"
#include "CarGPSTracer_Arduino.h"
#include "GPSHandler.h"  

//GPSHandler GPS_BLYNK;
//OBDIIHandler OBDII_BLYNK;

#define BLYNK_PRINT Serial
//#define ENABLE_DEBUG

// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
//#define BLYNK_HEARTBEAT 30
 
#include <BlynkSimpleTinyGSM.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "pbwUBfcm1FXwo4xVZa78VVncV7IDt3if";

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "internet.vodafone.net";
char user[] = "internet";
char pass[] = "internet";

#include <SoftwareSerial.h> 
SoftwareSerial Blynk_Serial(2, 3); // RX, TX


//GSM Module Settings
TinyGsm modem(Blynk_Serial);

#include <WidgetRTC.h>
WidgetMap myMap(V0);
BlynkTimer timer;
WidgetLED data_valid_led(V6);
WidgetLED prev_data_valid_led(V7);
WidgetTerminal terminal(V10);
WidgetRTC rtc;



bool terminal_debug_enable = false;
long reconnect_timer = 0;
long reconnect_timer_duration = 20;
int reset_counter = 0;
bool is_reset_required = false;


bool BLYNKBegin()
{
  Blynk_Serial.begin(9600);
  
	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	modem.restart();
	//modem.init();

  modem.gprsConnect(apn, user, pass);
  if (modem.isNetworkConnected())
  {
    Serial.println("Network Connected");
    Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
    setSyncInterval(10 * 60);
  }
  else
  {
    Serial.println("Network Not Connected");
  }
 

	//Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
#ifdef ENABLE_DEBUG
	Serial.println("Blynk done...");
#endif

	//setSyncInterval(10 * 60);

 
/*
	if( operation_mode > MODE_STOLLEN || operation_mode < MODE_NORMAL )
	{
		operation_mode = MODE_STOLLEN;
	}
*/
	timer.setInterval(100L, gpsUpdateTimer);
 
}

long execution_time = 0;
long start_time = 0;
void BLYNKRun()
{
  //Serial.println(millis()-execution_time);
  start_time = millis();
	if (modem.isNetworkConnected())
	{
		//DBG("Network connected");
		Blynk.run();
	}
	else
	{
  //Serial.println("Disconnect");
		if( (start_time - reconnect_timer >= reconnect_timer_duration*1000) && is_reset_required == false )
		{
			Serial.print("Trying to connect ");
      Serial.println(reset_counter);
			//modem.init();
 
			modem.restart(); 
			modem.gprsConnect(apn, user, pass); 
      
			//Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
      if (modem.isNetworkConnected())
      { 
			  //Blynk.run();
        Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
      }
			reconnect_timer = millis();
			reset_counter++;
		} 
    /*
		if( reset_counter > 45)
		{
			is_reset_required = true;
			reset_counter = 0;
			Serial.println("Sw Reset");
		}
    */
	}
 
	//Blynk.run();
	timer.run();
}


BLYNK_CONNECTED() {
  // Synchronize time on connection
  Serial.println("Blynk Connected");
  rtc.begin();

  Blynk.virtualWrite(V19, 0);   // Enable Terminal debuging
  Blynk.virtualWrite(V20, 0);   // operation_mode
  Blynk.virtualWrite(V26, 60);  // stollen_mode_update_duration
  Blynk.virtualWrite(V21, 60);  // park_location_check_timer_1_Duration
  Blynk.virtualWrite(V22, 30);   // park_location_check_timer_2_Duration
  Blynk.virtualWrite(V23, 20);  // geofencing_check_timer_1_Duration
  Blynk.virtualWrite(V27, 5);   // geofencing_check_timer_2_Duration
  Blynk.virtualWrite(V24, 150);   // geofencing_check_radius
  Blynk.virtualWrite(V25, 1);   // geofencing_check_enable
  Blynk.virtualWrite(V28, 0);   // print_geofencing_data_enable
  Blynk.virtualWrite(V29, 1);   // geofencing_send_email

  //Blynk.virtualWrite(V30, "?AllParm=1");
}
 /*
BLYNK_WRITE(V30)
{
  //Serial.println("WebHook data:");
  //Serial.println(param.asStr());

  DynamicJsonDocument doc(2024);
  DeserializationError error = deserializeJson(doc, param.asStr()); 
 if (error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.f_str());
  return;
 }
 
  //int arraySize =  doc["Parameters"].size();
  //Serial.println(arraySize);
  //const char* parm = doc[1]["Parm"];
  //Serial.println(parm); 
  
  bool terminal_debug_enable = (bool) doc[0]["Value"].as<int>();
  int operation_mode = doc[1]["Value"].as<int>();
  int stollen_mode_update_duration = doc[2]["Value"].as<int>();
  bool geofencing_check_enable = (bool)doc[3]["Value"].as<int>();
  bool print_geofencing_data_enable = (bool)doc[4]["Value"].as<int>();
  float geofencing_check_radius =  (float)doc[5]["Value"].as<int>();
  bool geofencing_send_email = (bool)doc[6]["Value"].as<int>();
  int park_location_check_timer_1_Duration = doc[7]["Value"].as<int>();
  int park_location_check_timer_2_Duration = doc[8]["Value"].as<int>();
  int geofencing_check_timer_1_Duration = doc[9]["Value"].as<int>();
  int geofencing_check_timer_2_Duration = doc[10]["Value"].as<int>();
  
  char msg[200];
   
  sprintf(msg,"terminal_debug_enable: %d", terminal_debug_enable);
  Serial.println(msg); 
  sprintf(msg,"operation_mode: %d", operation_mode);
  Serial.println(msg); 
  sprintf(msg,"stollen_mode_update_duration: %d", stollen_mode_update_duration);
  Serial.println(msg); 
  sprintf(msg,"geofencing_check_enable: %d", geofencing_check_enable);
  Serial.println(msg); 
  sprintf(msg,"print_geofencing_data_enable: %d", print_geofencing_data_enable);
  Serial.println(msg);  
  sprintf(msg,"geofencing_check_radius: %.02f", geofencing_check_radius);
  Serial.println(msg);  
  sprintf(msg,"geofencing_send_email: %d", geofencing_send_email);
  Serial.println(msg); 
  sprintf(msg,"park_location_check_timer_1_Duration: %d", park_location_check_timer_1_Duration);
  Serial.println(msg);  
  sprintf(msg,"park_location_check_timer_2_Duration: %d", park_location_check_timer_2_Duration);
  Serial.println(msg); 
  sprintf(msg,"geofencing_check_timer_1_Duration: %d", geofencing_check_timer_1_Duration);
  Serial.println(msg); 
  sprintf(msg,"geofencing_check_timer_2_Duration: %d", geofencing_check_timer_2_Duration);
  Serial.println(msg); 
 
}
 */
void SendEmail(char *subject, char *body)
{
	Blynk.email("khaledmagdy50@gmail.com", subject, body);
}

void SendNotification(char *msg)
{
	Blynk.notify(msg);
}

void UpdateLocation(float latitude, float longitude, float speed, float satellites, bool reset)
{
	if ( reset == false)
	{ 
		Blynk.virtualWrite(V1, String(latitude, 6));
		Blynk.virtualWrite(V2, String(longitude, 6));
		Blynk.virtualWrite(V3, speed);
		Blynk.virtualWrite(V4, satellites);
	}
	else
	{
		Blynk.virtualWrite(V1, 0);		// Latitude
		Blynk.virtualWrite(V2, 0);		// Longitude
		Blynk.virtualWrite(V3, 0);		// Speed
		Blynk.virtualWrite(V4, 0);		// Number of Satellites
	}
}
void UpdateMapLocation(int pointIndex, float latitude, float longitude)
{
	myMap.location(pointIndex, latitude, longitude, "GPS_Location");
}

void UpdateDataValidLed(bool state)
{
	if( state == true)
		data_valid_led.on();
	else
		data_valid_led.off();
}
void UpdatePrevDataValidLed(bool state)
{
	if( state == true)
		prev_data_valid_led.on();
	else
		prev_data_valid_led.off();
}

 

void TerminalWriteLine(char *msg, bool forcePrint)
{
	if( terminal_debug_enable== true || forcePrint == true)
	{
		terminal.println(msg);
	}
}
void TerminalWrite(char *msg)
{
	if( terminal_debug_enable== true )
	{
		terminal.print(msg);
	}
}
void TerminalFlush()
{
	if( terminal_debug_enable== true )
	{
		terminal.flush();
	}
}


int GetDay()
{
	return day();
}
int GetMonth()
{
	return month();
}
int GetYear()
{
	return year();
}
int GetHour()
{
	return hour();
}
int GetMinute()
{
	return minute();
}
int GetSecond()
{
	return second();
}





// Clear terminal
BLYNK_WRITE(V18)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    terminal.clear(); 
  }
}
// Enable Terminal debuging
BLYNK_WRITE(V19)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    terminal_debug_enable= true;
  }
  else
  {
    terminal_debug_enable= false;
  }
}


int update_request_index = 0;
// Update GPS data request
BLYNK_WRITE(V5)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 )//&& update_gps_data == false)
  {
	  GPS_SetUpdateGpsData(true);
#ifdef ENABLE_DEBUG
      Serial.println("Update Requested");
#endif
    String msg = "Update Request: " + String(update_request_index);
    if( terminal_debug_enable== true )
    {
      terminal.println(msg);
      terminal.flush();
    }
    update_request_index++;
  }
}

// Enable GPS auto update data
BLYNK_WRITE(V8)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
	 GPS_ResetAutoUpdateTimer();
     GPS_SetIsAutoUpdate(true);
  }
  else
  {
	  GPS_SetIsAutoUpdate(false);
	  GPS_ResetAutoUpdateTimer();
  }
}
// Set GPS auto update timer duration
BLYNK_WRITE(V9)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue > 0 && pinValue <= 60)
  {
	  GPS_SetAutoUpdateTimerDuration(pinValue);
  }
}

// Set GPS operation mode
BLYNK_WRITE(V20)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
	  GPS_ResetStollenModeUpdateTimer();
      GPS_SetOperationMode(MODE_STOLLEN);
  }
  else
  {
	  GPS_SetOperationMode(MODE_NORMAL);
  }
}
// Set stollen_mode_update_duration
BLYNK_WRITE(V26)
{
	int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
	GPS_SetStollenModeUpdateDuration(pinValue);
}

 
// Set GPS StandStillThreshold Duration
BLYNK_WRITE(V24)
{
  float pinValue = param.asFloat(); // assigning incoming value from pin V1 to a variable
  GPS_SetGeofencingRadius(pinValue);
}
// Set geofencing_check_enable
BLYNK_WRITE(V25)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
	  GPS_SetGeofencingCheckEnable(true);
  }
  else
  {
	  GPS_SetGeofencingCheckEnable(false);
  }
}
// Set print_geofencing_data_enable
BLYNK_WRITE(V28)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    GPS_SetPrintGeofencingDataEnable(true);
  }
  else
  {
    GPS_SetPrintGeofencingDataEnable(false);
  }
}
// Set geofencing_send_email
BLYNK_WRITE(V29)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    GPS_SetGeofencingSendEmail(true);
  }
  else
  {
    GPS_SetGeofencingSendEmail(false);
  }
}
