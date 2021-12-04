#include "BLYNK.h"
#include "CarGPSTracer_STM32.h"
#include "GPSHandler.h"
#include "OBDIIHandler.h"

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
			Serial1.println("Trying to connect");
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
		else
		{
			reconnect_timer++;
		}
		*/
    
		if( reset_counter > 5)
		{
			is_reset_required = true;
			reset_counter = 0;
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

void UpdatePIDData(int engine_col_temp, double engine_speed, int vehicle_speed, int throttle_pos, double runtime_since_engine_start)
{
	Blynk.virtualWrite(V11, engine_col_temp);
	Blynk.virtualWrite(V12, engine_speed);
	Blynk.virtualWrite(V13, vehicle_speed);
	Blynk.virtualWrite(V14, throttle_pos);
	Blynk.virtualWrite(V15, runtime_since_engine_start);
}

void TerminalWriteLine(char *msg)
{
	if( terminal_debug_enable== true )
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


BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();

  Blynk.virtualWrite(V19, 0);   // Enable Terminal debuging
  Blynk.virtualWrite(V20, 0);   // operation_mode
  Blynk.virtualWrite(V26, 60);  // stollen_mode_update_duration
  Blynk.virtualWrite(V21, 60);  // park_location_check_timer_1_Duration
  Blynk.virtualWrite(V22, 30);   // park_location_check_timer_2_Duration
  Blynk.virtualWrite(V23, 20);  // geofencing_check_timer_1_Duration
  Blynk.virtualWrite(V27, 5);   // geofencing_check_timer_2_Duration
  Blynk.virtualWrite(V24, 100);   // geofencing_check_radius
  Blynk.virtualWrite(V25, 1);   // geofencing_check_enable
  Blynk.virtualWrite(V28, 0);   // print_geofencing_data_enable
  Blynk.virtualWrite(V29, 0);   // geofencing_send_email
  
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
