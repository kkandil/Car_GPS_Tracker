#include <Arduino.h>

#include "BLYNK.h"
#include "CarGPSTracer_STM32.h"
#include "GPSHandler.h"
#include "OBDIIHandler.h"
#include <ArduinoJson.h>

//GPSHandler GPS_BLYNK;
//OBDIIHandler OBDII_BLYNK;

#define BLYNK_PRINT Serial1
//#define ENABLE_DEBUG

// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
//#define BLYNK_HEARTBEAT 30

#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "pbwUBfcm1FXwo4xVZa78VVncV7IDt3if";

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "internet.vodafone.net";
char user[] = "internet";
char pass[] = "internet";


//GSM Module Settings
TinyGsm modem(Serial2);

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

 
#define LIBMAPPLE_CORE //comment it for HAL based core

#define IWDG_PR_DIV_4 0x0
#define IWDG_PR_DIV_8 0x1
#define IWDG_PR_DIV_16 0x2
#define IWDG_PR_DIV_32 0x3
#define IWDG_PR_DIV_64 0x4
#define IWDG_PR_DIV_128 0x5
#define IWDG_PR_DIV_256 0x6

typedef enum iwdg_prescaler {
  IWDG_PRE_4 = IWDG_PR_DIV_4,     //< Divide by 4  
  IWDG_PRE_8 = IWDG_PR_DIV_8,     //< Divide by 8  
  IWDG_PRE_16 = IWDG_PR_DIV_16,   //< Divide by 16  
  IWDG_PRE_32 = IWDG_PR_DIV_32,   //< Divide by 32  
  IWDG_PRE_64 = IWDG_PR_DIV_64,   //< Divide by 64  
  IWDG_PRE_128 = IWDG_PR_DIV_128, //< Divide by 128  
  IWDG_PRE_256 = IWDG_PR_DIV_256  //< Divide by 256 
} iwdg_prescaler;

#if defined(LIBMAPPLE_CORE)
typedef struct iwdg_reg_map {
  volatile uint32_t KR;  //< Key register.  
  volatile uint32_t PR;  //< Prescaler register.  
  volatile uint32_t RLR; //< Reload register.  
  volatile uint32_t SR;  //< Status register  
} iwdg_reg_map;

#define IWDG ((struct iwdg_reg_map *)0x40003000)
#endif

void iwdg_feed(void) { IWDG->KR = 0xAAAA; }

//Time calculation (approximate): Tout=((4*2^prescaler)*reload)/40 (ms)
void iwdg_init(iwdg_prescaler prescaler, uint16_t reload) {
  IWDG->KR = 0x5555;
  IWDG->PR = prescaler;
  IWDG->RLR = reload;
  IWDG->KR = 0xCCCC;
  IWDG->KR = 0xAAAA;
}
 

bool BLYNKBegin()
{
  pinMode(PA12, OUTPUT);
  digitalWrite(PA12, HIGH);
  
	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	modem.restart();
	//modem.init();

  modem.gprsConnect(apn, user, pass);
  if (modem.isNetworkConnected())
  {
    Serial1.println("Network Connected");
    Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
    setSyncInterval(10 * 60);
  }
  else
  {
    Serial1.println("Network Not Connected");
  }
 

	//Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
#ifdef ENABLE_DEBUG
	Serial1.println("Blynk done...");
#endif

	//setSyncInterval(10 * 60);

 
/*
	if( operation_mode > MODE_STOLLEN || operation_mode < MODE_NORMAL )
	{
		operation_mode = MODE_STOLLEN;
	}
*/
	timer.setInterval(100L, gpsUpdateTimer);

	iwdg_init(IWDG_PRE_256, 4000);
 iwdg_feed();
}

long execution_time = 0;
long start_time = 0;
void BLYNKRun()
{
  //Serial1.println(millis()-execution_time);
  start_time = millis();
	if (modem.isNetworkConnected())
	{
		//DBG("Network connected");
		Blynk.run();
	}
	else
	{
  //Serial1.println("Disconnect");
		if( (start_time - reconnect_timer >= reconnect_timer_duration*1000) && is_reset_required == false )
		{
			Serial1.print("Trying to connect ");
      Serial1.println(reset_counter);
			//modem.init();

     if( is_reset_required == false)
     {
        iwdg_feed();
      }
			modem.restart();
      

      if( is_reset_required == false)
       {
          iwdg_feed();
        }
			modem.gprsConnect(apn, user, pass);

     if( is_reset_required == false)
     {
        iwdg_feed();
      }
			//Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
      if (modem.isNetworkConnected())
      {
        if( is_reset_required == false)
         {
            iwdg_feed();
          }
			  //Blynk.run();
        Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
      }
			reconnect_timer = millis();
			reset_counter++;
		}
		/*
		else
		{
			reconnect_timer++;
		}
		*/
    
		if( reset_counter > 45)
		{
			is_reset_required = true;
			reset_counter = 0;
      digitalWrite(PA12, LOW);
      delay(200);
      digitalWrite(PA12, HIGH);
			Serial1.println("Sw Reset");
		}
  
	}

	if( is_reset_required == false)
	{
		iwdg_feed();
	}

	//Blynk.run();
	timer.run();
}


BLYNK_CONNECTED() {
  // Synchronize time on connection
  Serial1.println("Blynk Connected");
  rtc.begin();
  Blynk.syncVirtual(V1,V2,V19,V20,V21,V22,V23,V24,V25,V26,V27,V28,V29, V30);
  
  /*
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
  */
  //Blynk.virtualWrite(V30, "?AllParm=1");
}
 /*
BLYNK_WRITE(V30)
{
  //Serial1.println("WebHook data:");
  //Serial.println(param.asStr());

  DynamicJsonDocument doc(2024);
  DeserializationError error = deserializeJson(doc, param.asStr()); 
 if (error) {
  Serial1.print(F("deserializeJson() failed: "));
  Serial1.println(error.f_str());
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
  Serial1.println(msg); 
  sprintf(msg,"operation_mode: %d", operation_mode);
  Serial1.println(msg); 
  sprintf(msg,"stollen_mode_update_duration: %d", stollen_mode_update_duration);
  Serial1.println(msg); 
  sprintf(msg,"geofencing_check_enable: %d", geofencing_check_enable);
  Serial1.println(msg); 
  sprintf(msg,"print_geofencing_data_enable: %d", print_geofencing_data_enable);
  Serial1.println(msg);  
  sprintf(msg,"geofencing_check_radius: %.02f", geofencing_check_radius);
  Serial1.println(msg);  
  sprintf(msg,"geofencing_send_email: %d", geofencing_send_email);
  Serial1.println(msg); 
  sprintf(msg,"park_location_check_timer_1_Duration: %d", park_location_check_timer_1_Duration);
  Serial1.println(msg);  
  sprintf(msg,"park_location_check_timer_2_Duration: %d", park_location_check_timer_2_Duration);
  Serial1.println(msg); 
  sprintf(msg,"geofencing_check_timer_1_Duration: %d", geofencing_check_timer_1_Duration);
  Serial1.println(msg); 
  sprintf(msg,"geofencing_check_timer_2_Duration: %d", geofencing_check_timer_2_Duration);
  Serial1.println(msg); 
 
}
 */
void SendEmail(String subject, String body)
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
		Blynk.virtualWrite(V3, String(speed));
    Blynk.virtualWrite(V4, String(satellites));
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
  myMap.location(pointIndex, String(latitude, 6), String(longitude, 6), "GPS_Location");
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

void UpdatePIDData(int engine_col_temp, double engine_speed, int vehicle_speed, int throttle_pos, double runtime_since_engine_start)
{
	Blynk.virtualWrite(V11, engine_col_temp);
	Blynk.virtualWrite(V12, engine_speed);
	Blynk.virtualWrite(V13, vehicle_speed);
	Blynk.virtualWrite(V14, throttle_pos);
	Blynk.virtualWrite(V15, runtime_since_engine_start);
}

void TerminalWriteLine(String msg, bool forcePrint)
{
	if( terminal_debug_enable== true || forcePrint == true)
	{
		terminal.println(msg);
	}
}
void TerminalWrite(String msg)
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



float last_known_lat = 0.0;
float last_known_lon = 0.0;

float GetLastKnownLat(void)
{
  return last_known_lat;
}
float GetLastKnownLon(void)
{
  return last_known_lon;
}
BLYNK_WRITE(V1)
{
  
  last_known_lat = param.asFloat();
}
BLYNK_WRITE(V2)
{
  
  last_known_lon = param.asFloat(); 
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
      Serial1.println("Update Requested");
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

// Set GPS park_location_check_timer_1_Duration
BLYNK_WRITE(V21)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_SetParkTimer1Duration(pinValue);
}
// Set GPS park_location_check_timer_2_Duration
BLYNK_WRITE(V22)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_SetParkTimer2Duration(pinValue);
}
// Set GPS geofencing_check_timer_1_Duration
BLYNK_WRITE(V23)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_SetGeofencingTimer1Duration(pinValue);
}
// Set GPS geofencing_check_timer_2_Duration
BLYNK_WRITE(V27)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_SetGeofencingTimer2Duration(pinValue);
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
// Set is_vehicle_parked
BLYNK_WRITE(V30)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    GPS_SetVehicleStatus(true);
  }
  else
  {
    GPS_SetVehicleStatus(false);
  }
}

void BLYNK_UpdateVehicleState(bool state)
{
  if( state == 0)
  {
    Blynk.virtualWrite(V30, 0); 
  }
  else
  {
    Blynk.virtualWrite(V30, 1); 
  }
}




// Update OBD data request
BLYNK_WRITE(V16)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 && GetUpdateObdData() == false)
  {
#ifdef ENABLE_DEBUG
      Serial1.println("Update OBD Requested");
#endif
    if( terminal_debug_enable== true )
    {
      terminal.println("Update OBD Requested");
      terminal.flush();
    }

    bool Status = SendPIDRequest(WAIT_RESPONCE_ENGINE_COL_TEMP_05);
    if( Status == true)
    {
    	SetDataReady(false);
    	SetUpdateObdData(true);
    }
    else
    {
      if( terminal_debug_enable== true )
      {
        terminal.println("error sending data");
        terminal.flush();
      }
    }

  }

  // process received value
}

BLYNK_WRITE(V17)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 && GetUpdateObdData() == false )//&& is_obd_auto_update == false)
  {
	  ResetObdAutoUpdateTimerCounter();
    SetIsObdAutoUpdate(true);

  }
  else
  {
    SetIsObdAutoUpdate(false);
    ResetObdAutoUpdateTimerCounter();
  }
}
