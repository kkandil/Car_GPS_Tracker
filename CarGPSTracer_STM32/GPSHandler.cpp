#include "GPSHandler.h"
#include "BLYNK.h"
#include <TinyGPS++.h>  

#define GPS_DEBUG_ENABLE


BLYNK BLYNK_GPS;
TinyGPSPlus gps; 			//GPS Module Handler


struct strGPS_DATA{
  float latitude;
  float longitude;
  float speed;
  float satellites;
  bool data_valid;
  bool prev_data_valid;

  float park_location_lat;
  float park_location_lon;
  bool is_vehicle_parked;
  float travelDistance;
};

strGPS_DATA gps_data;

float prev_lat = 0.0, prev_lon = 0.0;
int park_location_check_timer_1 = 0;
int park_location_check_timer_2 = 0;
int geofencing_check_timer_1 = 0;
int geofencing_check_timer_2 = 0;
int park_location_check_timer_1_Duration = 60; // 60 sec.
int park_location_check_timer_2_Duration = 30;  // 30 mins
int geofencing_check_timer_1_Duration = 20;  	// 20 sec
int geofencing_check_timer_2_Duration = 5;  	// 100 sec
float travelDistance = 0.0;
float geofencing_check_radius = 150.0;			// distance in m
float standstill_speed_threshold = 5; 			// speed in KPH


// StollenMode variables
int operation_mode = MODE_NORMAL;
int stollen_mode_update_duration = 60;
int stollen_mode_update_timer = 0;


// LocationUpdate variables
bool update_gps_data = false;
bool is_auto_update = false;
int auto_update_timer_duration = 2;
int auto_update_timer = 0;

// GeofencingUpdate variables
bool prevAtHomeState = false;
bool geofencing_check_enable = true;


void InitializeGPS();

GPSHandler::GPSHandler(){} 

GPSHandler::~GPSHandler(){}

bool GPSHandler::Begin()
{
	//Set GPS module baud rate
	Serial3.begin(9600);

	InitializeGPS();

#ifdef GPS_DEBUG_ENABLE
	Serial1.println("neogps serial initialize");
#endif

	delay(10);
}

void InitializeGPS()
{
	gps_data.latitude = 0.0;
	gps_data.longitude = 0.0;
	gps_data.speed = 0.0;
	gps_data.satellites = 0.0;
	gps_data.data_valid = false;
	gps_data.prev_data_valid = false;
	gps_data.park_location_lat = 0.0;
	gps_data.park_location_lon = 0.0;
	gps_data.is_vehicle_parked = false;
	gps_data.travelDistance = 0.0;
}

void GPSHandler::UpdateGPSData()
{ 
	gps_data.data_valid = gps.location.isUpdated();

	if ( gps_data.data_valid )
	{
		gps_data.latitude = (gps.location.lat());
		gps_data.longitude = (gps.location.lng());
		gps_data.speed = gps.speed.kmph();
		gps_data.satellites = gps.satellites.value();
		gps_data.prev_data_valid = true;
	}

}

void GPSHandler::smartDelay(unsigned long ms)
{
  static unsigned long start = millis();
  do
  {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}

/*
void GPSHandler::GPSHandler_Cyclic100ms()
{

}
*/


void GPSHandler::HandleLocationUpdate()
{
	int pointIndex = 1;

	if( update_gps_data == true || (is_auto_update == true && auto_update_timer >= (auto_update_timer_duration*10)))
	{ 
		auto_update_timer = 0;
		if( gps_data.data_valid == true || gps_data.prev_data_valid == true )
		{
			BLYNK_GPS.UpdateLocation(gps_data.latitude, gps_data.longitude, gps_data.speed, gps_data.satellites, false);
			BLYNK_GPS.UpdateMapLocation(pointIndex, gps_data.latitude, gps_data.longitude);

			BLYNK_GPS.UpdateDataValidLed(gps_data.data_valid);
			BLYNK_GPS.UpdatePrevDataValidLed(gps_data.prev_data_valid);

			BLYNK_GPS.TerminalWriteLine("Data Valid");
			BLYNK_GPS.TerminalFlush();
		}
		else
		{
			// reset GPS data
			BLYNK_GPS.UpdateLocation(0.0, 0.0, 0.0, 0.0, true);

			BLYNK_GPS.UpdateDataValidLed(false);
			BLYNK_GPS.UpdatePrevDataValidLed(false);

			BLYNK_GPS.TerminalWriteLine("No Data");
			BLYNK_GPS.TerminalFlush();
		}
		update_gps_data = false;
	}
	else if( is_auto_update == true )
	{
		auto_update_timer++;
	}
}

void GPSHandler::HandleStollenMode()
{
	char msgString[128];

	if( operation_mode == MODE_STOLLEN && stollen_mode_update_timer >= stollen_mode_update_duration*10 )
	{
		if( gps_data.data_valid == true || gps_data.prev_data_valid == true )
		{
			// Send email with the vehicle location every stollen_mode_update_duration seconds
			sprintf(msgString, "Loc: %.06f, %.06f ,Speed: %.06f, Time: %d/%d/%d-%d:%d:%d",
			gps_data.latitude, gps_data.longitude, gps_data.speed, BLYNK_GPS.GetDay(), BLYNK_GPS.GetMonth(), BLYNK_GPS.GetYear(), BLYNK_GPS.GetHour(), BLYNK_GPS.GetMinute(), BLYNK_GPS.GetSecond());
			BLYNK_GPS.SendEmail("Accent GPS Update", msgString);
#ifdef GPS_DEBUG_ENABLE
			Serial.println("Email Sent");
#endif
		}
		stollen_mode_update_timer = 0;
	}
	else if( operation_mode == MODE_STOLLEN )
	{
		stollen_mode_update_timer++;
	}
}

void GPSHandler::HandleGeofencingMode()
{
  char msg[125];
  if( geofencing_check_enable == true )
  {
	  if( gps_data.is_vehicle_parked == false)
	  {
		if( park_location_check_timer_1 >= park_location_check_timer_1_Duration*10) // Check every 1 min.
		{
			// Calculate the distance between current vehicle location and the previous recorded location
			float travelDistance = (float)TinyGPSPlus::distanceBetween( gps_data.latitude, gps_data.longitude, prev_lat, prev_lon ) ;
			gps_data.travelDistance = travelDistance;

	#ifdef GPS_DEBUG_ENABLE
			sprintf(msg, "Dist = %.2f", travelDistance);
			Serial.println(msg);
	#endif

			// Check if the travel distance is within the geofencing check raduis and if the vehicle speed is less than
			// the standstill speed threshold
			// timer park_location_check_timer_2 is added to make sure that the vehicle was in standstill in the same location
			// for a sufficient time to role out the case if the driver was still in the vehicle and waiting for something
			// as there is no way of detecting that the driver is still in the car or if the engine is still turned on.
			if( travelDistance < geofencing_check_radius && gps_data.speed < standstill_speed_threshold )
			{
				park_location_check_timer_2++;
			}
			else
			{
				park_location_check_timer_2 = 0;
			}

			// Check if timer park_location_check_timer_2 elapsed while the vehicle was in standstill
			if( park_location_check_timer_2 >= park_location_check_timer_2_Duration )
			{
				// Mark the current location as the vehicle parked location
				gps_data.park_location_lat = gps_data.latitude;
				gps_data.park_location_lon = gps_data.longitude;
				gps_data.is_vehicle_parked = true;
				park_location_check_timer_2 = 0;

				sprintf(msg, "Accent parked at: %d/%d/%d-%d:%d:%d", BLYNK_GPS.GetDay(), BLYNK_GPS.GetMonth(), BLYNK_GPS.GetYear(), BLYNK_GPS.GetHour(), BLYNK_GPS.GetMinute(), BLYNK_GPS.GetSecond());
				BLYNK_GPS.TerminalWriteLine(msg);
				BLYNK_GPS.TerminalFlush();

	#ifdef GPS_DEBUG_ENABLE
				Serial.println("Vehicle Parked");
	#endif
			}
			prev_lat = gps_data.latitude;
			prev_lon = gps_data.longitude;
			park_location_check_timer_1 = 0;
		}
		else
		{
			park_location_check_timer_1++;
		}
	  }
	  else
	  {
		if( geofencing_check_timer_1 >= geofencing_check_timer_1_Duration*10 ) // Check every 30 sec.
		{
			// Calculate the distance between current location and the vehicle parked location
			float travelDistance = (float)TinyGPSPlus::distanceBetween( gps_data.latitude, gps_data.longitude, gps_data.park_location_lat, gps_data.park_location_lon ) ;
			gps_data.travelDistance = travelDistance;

	#ifdef GPS_DEBUG_ENABLE
			sprintf(msg, "Dist2 = %.2f", travelDistance);
			Serial.println(msg);
	#endif

			// Check if vehicle moved away from the home location or vehicle speed is more than standstill_speed_threshold
			// A filtering is done to avoid error spikes in the GPS data by using timer home_location_check_timer_4 so
			// distance or speed must be more than the specified threshold during this time in order to consider the vehicle moved
			if( travelDistance > geofencing_check_radius || gps_data.speed >= standstill_speed_threshold )
			{
				geofencing_check_timer_2++;
			}
			else
			{
				geofencing_check_timer_2 = 0;
			}

			// Check if timer geofencing_check_timer_2 elapsed while the vehicle was moving
			if( geofencing_check_timer_2 >= geofencing_check_timer_2_Duration )
			{
				gps_data.is_vehicle_parked = false;
				geofencing_check_timer_2 = 0;

				sprintf(msg, "Accent Moved at: %d/%d/%d-%d:%d:%d", BLYNK_GPS.GetDay(), BLYNK_GPS.GetMonth(), BLYNK_GPS.GetYear(), BLYNK_GPS.GetHour(), BLYNK_GPS.GetMinute(), BLYNK_GPS.GetSecond());
				BLYNK_GPS.TerminalWriteLine(msg);
				BLYNK_GPS.TerminalFlush();

				//String currentTime = String(BLYNK_GPS.GetDay()) + "/" + BLYNK_GPS.GetMonth() + "/" + BLYNK_GPS.GetYear() + "-" + String(BLYNK_GPS.GetHour()) + ":" + BLYNK_GPS.GetMinute() + ":" + BLYNK_GPS.GetSecond();
				//Stringmessage = "Accent Moved at: " + currentTime  ;
				BLYNK_GPS.SendNotification(msg) ;

	#ifdef GPS_DEBUG_ENABLE
			  Serial.println("vehicle moved");
	#endif
			}

			prev_lat = gps_data.latitude;
			prev_lon = gps_data.longitude;
			geofencing_check_timer_1 = 0;


		}
		else
		{
			geofencing_check_timer_1++;
		}
	  }
  }
}



/*
strGPS_DATA GPSHandler::GetGPSData()
{
  return gps_data;
}
*/
void GPSHandler::SetParkTimer1Duration(int duration)
{
	park_location_check_timer_1_Duration = duration;
}
void GPSHandler::SetParkTimer2Duration(int duration)
{
	park_location_check_timer_2_Duration = duration;
}
void GPSHandler::SetGeofencingTimer1Duration(int duration)
{
	geofencing_check_timer_1_Duration = duration;
}
void GPSHandler::SetGeofencingTimer2Duration(int duration)
{
	geofencing_check_timer_2_Duration = duration;
}

void GPSHandler::SetGeofencingRadius(float radius)
{
	geofencing_check_radius = radius;
} 

void GPSHandler::SetUpdateGpsData(bool state)
{
	update_gps_data = state;
}

void GPSHandler::SetIsAutoUpdate(bool state)
{
	is_auto_update = state;
}
void GPSHandler::ResetAutoUpdateTimer()
{
	auto_update_timer = 0;
}
void GPSHandler::SetAutoUpdateTimerDuration(int duration)
{
	auto_update_timer_duration = duration;
}

void GPSHandler::SetOperationMode(int mode)
{
	operation_mode = mode;
}
void GPSHandler::ResetStollenModeUpdateTimer()
{
	stollen_mode_update_timer = 0;
}
void GPSHandler::SetStollenModeUpdateDuration(int duration)
{
	stollen_mode_update_duration = duration;
}
void GPSHandler::SetGeofencingCheckEnable(bool state)
{
	geofencing_check_enable = state;
}
