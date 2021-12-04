#include "GPSHandler.h"
#include <TinyGPS++.h>  

#define GPS_DEBUG_ENABLE


TinyGPSPlus gps; 			//GPS Module Handler

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

void UpdateGeofencingStatus();
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

	UpdateGeofencingStatus();
}


void UpdateGeofencingStatus()
{
  char msg[125];
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
		if( travelDistance > geofencing_check_radius && gps_data.speed >= standstill_speed_threshold )
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

void GPSHandler::smartDelay(unsigned long ms)
{
  static unsigned long start = millis();
  do 
  {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}


strGPS_DATA GPSHandler::GetGPSData()
{
  return gps_data;
}
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
