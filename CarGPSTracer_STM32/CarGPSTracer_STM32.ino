
#include "GPSHandler.h"
#include "OBDIIHandler.h"




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

// OBDII module
OBDIIHandler OBDII(PA4, PB0);
bool update_obd_data = false;
int obd_auto_update_timer_counter = 0;
bool is_obd_auto_update = false; 
strOBD_Data obdData;


//GSM Module Settings 
TinyGsm modem(Serial2);


GPSHandler GPS;


 
bool update_gps_data = false;
bool is_auto_update = false;
int auto_update_duration = 2;
int auto_update_timer_counter = 0;
strGPS_DATA gpsData;
#define MODE_NORMAL   0
#define MODE_STOLLEN  1
int operation_mode = MODE_NORMAL;
int stollen_mode_update_duration = 60;
int stollen_mode_update_timer = 0; 
bool prevAtHomeState = false;


/* Comment this out to disable prints and save space */
#include <WidgetRTC.h>
WidgetMap myMap(V0);
BlynkTimer timer;
WidgetLED data_valid_led(V6);
WidgetLED prev_data_valid_led(V7);
WidgetTerminal terminal(V10);
WidgetRTC rtc;


bool terminal_debug_enable = false;
bool geofencing_check_enable = true;
bool print_geofencing_data_enable = false;

unsigned long timer_start = 0;
int sleepcounter = 0;
unsigned long current_time = 0;

void gpsUpdateTimer();

void setup() {
	//Set Serial monitor baud rate
	Serial1.begin(115200);

	// Init GPS module
	GPS.Begin();

	//Set GSM module baud rate
	Serial2.begin(9600);

	// Init OBDII driver
	OBDII.Begin( );

	delay(3000);


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

	if( operation_mode > MODE_STOLLEN || operation_mode < MODE_NORMAL )
	{
		operation_mode = MODE_STOLLEN;
	}

	timer.setInterval(100L, gpsUpdateTimer);
}

 
void loop() {
	Blynk.run();
	timer.run();
	GPS.smartDelay(0);
	OBDII.HandleOBD();
} //main loop ends

void gpsUpdateTimer()
{

	char msgString[128];
	int pointIndex = 1;


	//=================================================================================================================
	//=================================================================================================================
	// GPS function handling section

	// Stollen mode handling
	GPS.UpdateGPSData();
	if( operation_mode == MODE_STOLLEN && stollen_mode_update_timer >= stollen_mode_update_duration*10 )
	{
		gpsData = GPS.GetGPSData();
		if( gpsData.data_valid == true || gpsData.prev_data_valid == true )
		{
			// Send email with the vehicle location every stollen_mode_update_duration seconds
			sprintf(msgString, "Loc: %.06f, %.06f ,Speed: %.06f, Time: %d/%d/%d-%d:%d:%d", gpsData.latitude, gpsData.longitude, gpsData.speed, day(), month(), year(), hour(), minute(), second());
			Blynk.email("khaledmagdy50@gmail.com", "Accent GPS Update", msgString);
			Serial.println("Email Sent");
		}
		stollen_mode_update_timer = 0;
	}
	else if( operation_mode == MODE_STOLLEN )
	{
		stollen_mode_update_timer++;
	}

	//----------------------------------------------------
	// GPS location data update
	if( update_gps_data == true || (is_auto_update == true && auto_update_timer_counter >= (auto_update_duration*10)))
	{
		gpsData = GPS.GetGPSData();
		auto_update_timer_counter = 0;
		if( gpsData.data_valid == true || gpsData.prev_data_valid == true )
		{
			Blynk.virtualWrite(V1, String(gpsData.latitude, 6));
			Blynk.virtualWrite(V2, String(gpsData.longitude, 6));
			Blynk.virtualWrite(V3, gpsData.speed);
			Blynk.virtualWrite(V4, gpsData.satellites);
			myMap.location(pointIndex, gpsData.latitude, gpsData.longitude, "GPS_Location");

			if( gpsData.data_valid == true )
				data_valid_led.on();
			else
				data_valid_led.off();
			if( gpsData.prev_data_valid == true )
				prev_data_valid_led.on();
			else
				prev_data_valid_led.off();
			if( terminal_debug_enable== true )
			{
				terminal.println("Data Valid");
				terminal.flush();
			}
		}
		else
		{
			// reset GPS data
			Blynk.virtualWrite(V1, 0);		// Latitude
			Blynk.virtualWrite(V2, 0);		// Longitude
			Blynk.virtualWrite(V3, 0);		// Speed
			Blynk.virtualWrite(V4, 0);		// Number of Satellites
			data_valid_led.off();			// Current data valid indicator
			prev_data_valid_led.off();		// Previous received data valid
			if( terminal_debug_enable== true )
			{
				terminal.println("No Data");
				terminal.flush();
			}
		}
		update_gps_data = false;
	}
	else if( is_auto_update == true )
	{
		auto_update_timer_counter++;
	}

	//----------------------------------------------------
	// Geofencing check section
	if( geofencing_check_enable == true )
	{
		gpsData = GPS.GetGPSData();
		if(prevAtHomeState == false && gpsData.is_vehicle_parked == true )
		{
			sprintf(msgString, "Accent parked at: %d/%d/%d-%d:%d:%d", day(), month(), year(), hour(), minute(), second());
			terminal.println(msgString);
			terminal.flush();
			prevAtHomeState = gpsData.is_vehicle_parked ;
		}
		else if( prevAtHomeState == true && gpsData.is_vehicle_parked == false )
		{
			sprintf(msgString, "Accent Moved at: %d/%d/%d-%d:%d:%d", day(), month(), year(), hour(), minute(), second());
			terminal.println(msgString);
			terminal.flush();

			String currentTime = String(day()) + "/" + month() + "/" + year() + "-" + String(hour()) + ":" + minute() + ":" + second();
			//Stringmessage = "Accent Moved at: " + currentTime  ;
			Blynk.notify("Accent Moved at: " + currentTime ) ;
			prevAtHomeState = gpsData.is_vehicle_parked ;
		}

		if( terminal_debug_enable == true )
		{
			sprintf(msgString, "travel Dist = %.02f", gpsData.travelDistance);
			terminal.println(msgString);
			terminal.flush();
		}
	}


	//=================================================================================================================
	//=================================================================================================================
	// OBD function handling section
	if( update_obd_data == true && OBDII.GetDataReady() == true )
	{
		obdData = OBDII.GetPIDData();
		Blynk.virtualWrite(V11, obdData.engine_col_temp);
		Blynk.virtualWrite(V12, obdData.engine_speed/10.0);
		Blynk.virtualWrite(V13, obdData.vehicle_speed);
		Blynk.virtualWrite(V14, obdData.throttle_pos);
		Blynk.virtualWrite(V15, obdData.runtime_since_engine_start/10.0);

		if( terminal_debug_enable== true )
		{
			sprintf(msgString, "engine_col_temp[05]: %d",obdData.engine_col_temp_val);
			terminal.println(msgString);
			for(byte i = 0; i<8; i++){
				sprintf(msgString, " 0x%.2X", obdData.data_engine_col_temp[i]);
				terminal.print(msgString);
			}
			terminal.println();

			sprintf(msgString, "engine_speed[0C]: %d",obdData.engine_speed_val);
			terminal.println(msgString);
			for(byte i = 0; i<8; i++){
				sprintf(msgString, " 0x%.2X", obdData.data_engine_speed[i]);
				terminal.print(msgString);
			}
			terminal.println();

			sprintf(msgString, "Vehucle_Speed[0D]: %d",obdData.vehicle_speed_val);
			terminal.println(msgString);
			for(byte i = 0; i<8; i++){
				sprintf(msgString, " 0x%.2X", obdData.data_vehicle_speed[i]);
				terminal.print(msgString);
			}
			terminal.println();

			sprintf(msgString, "throttle_pos[11]: %d",obdData.throttle_pos_val);
			terminal.println(msgString);
			for(byte i = 0; i<8; i++){
				sprintf(msgString, " 0x%.2X", obdData.data_throttle_pos[i]);
				terminal.print(msgString);
			}
			terminal.println();

			sprintf(msgString, "throttle_pos[1F]: %d",obdData.runtime_since_engine_start_val);
			terminal.println(msgString);
			for(byte i = 0; i<8; i++){
				sprintf(msgString, " 0x%.2X", obdData.data_runtime_since_engine_start[i]);
				terminal.print(msgString);
			}
			terminal.println();
			terminal.println("-----------------------");
			terminal.flush();
		}
		update_obd_data = false;
	}
	else if( (is_obd_auto_update == true && obd_auto_update_timer_counter >= (auto_update_duration*10)))
	{
		if( OBDII.GetDataReady() == true)
		{
			obdData = OBDII.GetPIDData();
			Blynk.virtualWrite(V11, obdData.engine_col_temp);
			Blynk.virtualWrite(V12, obdData.engine_speed/10.0);
			Blynk.virtualWrite(V13, obdData.vehicle_speed);
			Blynk.virtualWrite(V14, obdData.throttle_pos);
			Blynk.virtualWrite(V15, obdData.runtime_since_engine_start/10.0);

			OBDII.SetDataReady(false);
			OBDII.SendPIDRequest(WAIT_RESPONCE_ENGINE_COL_TEMP_05);
			obd_auto_update_timer_counter = 0;
		}
	}
	else if(is_obd_auto_update == true)
	{
		obd_auto_update_timer_counter++;
	}
}

BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin(); 
}

int update_request_index = 0;
// Update GPS data request
BLYNK_WRITE(V5)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 && update_gps_data == false)
  {
     update_gps_data = true;
#ifdef ENABLE_DEBUG
      Serial1.println("Update Requested"); 
#endif
    String ms = "Update Request: " + String(update_request_index); 
    if( terminal_debug_enable== true )
    {
      terminal.println(ms);
      terminal.flush();
    }
    update_request_index++;
  }
  
  // process received value
}


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


// Update OBD data request
BLYNK_WRITE(V16)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1 && update_obd_data == false)
  { 
#ifdef ENABLE_DEBUG
      Serial1.println("Update OBD Requested"); 
#endif 
    if( terminal_debug_enable== true )
    {
      terminal.println("Update OBD Requested");
      terminal.flush(); 
    }
    
    bool Status = OBDII.SendPIDRequest(WAIT_RESPONCE_ENGINE_COL_TEMP_05);
    if( Status == true)
    {
      OBDII.SetDataReady(false);
      update_obd_data = true;
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
  if( pinValue == 1 && update_obd_data == false && is_obd_auto_update == false)
  { 
    //Blynk.disconnect();
    //modem.poweroff();
    //modem.radioOff();
    obd_auto_update_timer_counter = 0;
    is_obd_auto_update = true; 
    //timer_start = millis();
    /*
     obd_auto_update_timer_counter = 0;
     is_obd_auto_update = true; 
     OBDII.SetDataReady(false);
     OBDII.SendPIDRequest(WAIT_RESPONCE_ENGINE_COL_TEMP_05);
     */
  }
  else
  {
    is_obd_auto_update = false;
    obd_auto_update_timer_counter = 0;
  }
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

// update operation mode 
BLYNK_WRITE(V20)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    stollen_mode_update_timer = 0;
    operation_mode = MODE_STOLLEN;
  }
  else
  {
    operation_mode = MODE_NORMAL;
  }
}

// Set GPS park_location_check_timer_1_Duration
BLYNK_WRITE(V21)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS.SetParkTimer1Duration(pinValue);
} 
// Set GPS park_location_check_timer_2_Duration
BLYNK_WRITE(V22)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS.SetParkTimer2Duration(pinValue);
} 
// Set GPS geofencing_check_timer_1_Duration
BLYNK_WRITE(V23)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS.SetGeofencingTimer1Duration(pinValue);
}
// Set GPS geofencing_check_timer_2_Duration
BLYNK_WRITE(V27)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  GPS.SetGeofencingTimer2Duration(pinValue);
} 
// Set GPS StandStillThreshold Duration
BLYNK_WRITE(V24)
{ 
  float pinValue = param.asFloat(); // assigning incoming value from pin V1 to a variable
  GPS.SetGeofencingRadius(pinValue);
}
// Set geofencing_check_enable
BLYNK_WRITE(V25)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
	  geofencing_check_enable = true;
  }
  else
  {
	  geofencing_check_enable = false;
  }
}
// Set print_geofencing_data_enable
BLYNK_WRITE(V28)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if( pinValue == 1)
  {
    print_geofencing_data_enable = true;
  }
  else
  {
    print_geofencing_data_enable = false;
  }
}

// Set stollen_mode_update_duration
BLYNK_WRITE(V26)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  stollen_mode_update_duration = pinValue;
}  
