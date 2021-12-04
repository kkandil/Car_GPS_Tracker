#include "BLYNK.h"
#include "CarGPSTracer_STM32.h"
#include "GPSHandler.h"
#include "OBDIIHandler.h"

GPSHandler GPS_BLYNK;
OBDIIHandler OBDII_BLYNK;

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



BLYNK::BLYNK(){}

BLYNK::~BLYNK(){}

bool BLYNK::Begin()
{
	// Restart takes quite some time
	// To skip it, call init() instead of restart()
	modem.restart();
	//modem.init();

	Blynk.begin(auth, modem, apn, user, pass, "blynk-cloud.com", 8080);
#ifdef ENABLE_DEBUG
	Serial1.println("Blynk done...");
#endif

	setSyncInterval(10 * 60);

	//EEPROM.begin(512);
	//EEPROM.get(0, operation_mode);
/*
	if( operation_mode > MODE_STOLLEN || operation_mode < MODE_NORMAL )
	{
		operation_mode = MODE_STOLLEN;
	}
*/
	timer.setInterval(100L, gpsUpdateTimer);
}

void BLYNK::Run()
{
	Blynk.run();
	timer.run();
}

void BLYNK::SendEmail(char *subject, char *body)
{
	Blynk.email("khaledmagdy50@gmail.com", subject, body);
}

void BLYNK::SendNotification(char *msg)
{
	Blynk.notify(msg);
}

void BLYNK::UpdateLocation(float latitude, float longitude, float speed, float satellites, bool reset)
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
void BLYNK::UpdateMapLocation(int pointIndex, float latitude, float longitude)
{
	myMap.location(pointIndex, latitude, longitude, "GPS_Location");
}

void BLYNK::UpdateDataValidLed(bool state)
{
	if( state == true)
		data_valid_led.on();
	else
		data_valid_led.off();
}
void BLYNK::UpdatePrevDataValidLed(bool state)
{
	if( state == true)
		prev_data_valid_led.on();
	else
		prev_data_valid_led.off();
}

void BLYNK::UpdatePIDData(int engine_col_temp, double engine_speed, int vehicle_speed, int throttle_pos, double runtime_since_engine_start)
{
	Blynk.virtualWrite(V11, engine_col_temp);
	Blynk.virtualWrite(V12, engine_speed);
	Blynk.virtualWrite(V13, vehicle_speed);
	Blynk.virtualWrite(V14, throttle_pos);
	Blynk.virtualWrite(V15, runtime_since_engine_start);
}

void BLYNK::TerminalWriteLine(char *msg)
{
	if( terminal_debug_enable== true )
	{
		terminal.println(msg);
	}
}
void BLYNK::TerminalWrite(char *msg)
{
	if( terminal_debug_enable== true )
	{
		terminal.print(msg);
	}
}
void BLYNK::TerminalFlush()
{
	if( terminal_debug_enable== true )
	{
		terminal.flush();
	}
}


int BLYNK::GetDay()
{
	return day();
}
int BLYNK::GetMonth()
{
	return month();
}
int BLYNK::GetYear()
{
	return year();
}
int BLYNK::GetHour()
{
	return hour();
}
int BLYNK::GetMinute()
{
	return minute();
}
int BLYNK::GetSecond()
{
	return second();
}


BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
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
	  GPS_BLYNK.SetUpdateGpsData(true);
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
	 GPS_BLYNK.ResetAutoUpdateTimer();
     GPS_BLYNK.SetIsAutoUpdate(true);
  }
  else
  {
	  GPS_BLYNK.SetIsAutoUpdate(false);
	  GPS_BLYNK.ResetAutoUpdateTimer();
  }
}
// Set GPS auto update timer duration
BLYNK_WRITE(V9)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue > 0 && pinValue <= 60)
  {
	  GPS_BLYNK.SetAutoUpdateTimerDuration(pinValue);
  }
}

// Set GPS operation mode
BLYNK_WRITE(V20)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
	  GPS_BLYNK.ResetStollenModeUpdateTimer();
      GPS_BLYNK.SetOperationMode(MODE_STOLLEN);
  }
  else
  {
	  GPS_BLYNK.SetOperationMode(MODE_NORMAL);
  }
}
// Set stollen_mode_update_duration
BLYNK_WRITE(V26)
{
	int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
	GPS_BLYNK.SetStollenModeUpdateDuration(pinValue);
}

// Set GPS park_location_check_timer_1_Duration
BLYNK_WRITE(V21)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_BLYNK.SetParkTimer1Duration(pinValue);
}
// Set GPS park_location_check_timer_2_Duration
BLYNK_WRITE(V22)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_BLYNK.SetParkTimer2Duration(pinValue);
}
// Set GPS geofencing_check_timer_1_Duration
BLYNK_WRITE(V23)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_BLYNK.SetGeofencingTimer1Duration(pinValue);
}
// Set GPS geofencing_check_timer_2_Duration
BLYNK_WRITE(V27)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS_BLYNK.SetGeofencingTimer2Duration(pinValue);
}
// Set GPS StandStillThreshold Duration
BLYNK_WRITE(V24)
{
  float pinValue = param.asFloat(); // assigning incoming value from pin V1 to a variable
  GPS_BLYNK.SetGeofencingRadius(pinValue);
}
// Set geofencing_check_enable
BLYNK_WRITE(V25)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
	  GPS_BLYNK.SetGeofencingCheckEnable(true);
  }
  else
  {
	  GPS_BLYNK.SetGeofencingCheckEnable(false);
  }
}



// Update OBD data request
BLYNK_WRITE(V16)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 && OBDII_BLYNK.GetUpdateObdData() == false)
  {
#ifdef ENABLE_DEBUG
      Serial1.println("Update OBD Requested");
#endif
    if( terminal_debug_enable== true )
    {
      terminal.println("Update OBD Requested");
      terminal.flush();
    }

    bool Status = OBDII_BLYNK.SendPIDRequest(WAIT_RESPONCE_ENGINE_COL_TEMP_05);
    if( Status == true)
    {
    	OBDII_BLYNK.SetDataReady(false);
    	OBDII_BLYNK.SetUpdateObdData(true);
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
  if( pinValue == 1 && OBDII_BLYNK.GetUpdateObdData() == false )//&& is_obd_auto_update == false)
  {
	  OBDII_BLYNK.ResetObdAutoUpdateTimerCounter();
    OBDII_BLYNK.SetIsObdAutoUpdate(true);

  }
  else
  {
    OBDII_BLYNK.SetIsObdAutoUpdate(false);
    OBDII_BLYNK.ResetObdAutoUpdateTimerCounter();
  }
}
