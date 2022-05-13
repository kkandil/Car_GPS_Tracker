#include <Arduino.h>
#include "GPSHandler.h"
#include "BLYNK.h"

#include <TinyGPS++.h>  

#define GPS_DEBUG_ENABLE


//BLYNK BLYNK_GPS;
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
int park_location_check_timer_2_Duration = 15;  // 30 mins
int geofencing_check_timer_1_Duration = 10;  	// 20 sec
int geofencing_check_timer_2_Duration = 5;  	// 100 sec
float travelDistance = 0.0;
float geofencing_check_radius = 100.0;			// distance in m
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
bool geofencing_check_enable = true;
bool print_geofencing_data_enable = false;
bool geofencing_send_email = true;

void InitializeGPS();

  
bool GPS_Begin()
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
	gps_data.is_vehicle_parked = true;
	gps_data.travelDistance = 0.0;
}

void GPS_UpdateGPSData()
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

void GPS_SmartDelay(unsigned long ms)
{
  static unsigned long start = millis();
  do
  {
    while (Serial3.available())
      gps.encode(Serial3.read());
  } while (millis() - start < ms);
}

/*
void GPSHandler_Cyclic100ms()
{

}
*/


void GPS_HandleLocationUpdate()
{
	int pointIndex = 1;
  //gps_data.latitude = 30.985491;
  //gps_data.longitude = 30.043659;

  if( gps_data.is_vehicle_parked == true && gps_data.park_location_lat == 0.0 || gps_data.park_location_lon == 0.0)
  {
    gps_data.park_location_lat = GetLastKnownLat();
    gps_data.park_location_lon = GetLastKnownLon();
  }
	if( update_gps_data == true || (is_auto_update == true && auto_update_timer >= (auto_update_timer_duration*10)))
	{ 
		auto_update_timer = 0; 
		if( gps_data.data_valid == true || gps_data.prev_data_valid == true )
		{
			UpdateLocation((float)gps_data.latitude, (float)gps_data.longitude, (float)gps_data.speed, (float)gps_data.satellites, false);
			UpdateMapLocation(pointIndex, (float)gps_data.latitude, (float)gps_data.longitude);
 
			UpdateDataValidLed(gps_data.data_valid);
			UpdatePrevDataValidLed(gps_data.prev_data_valid);

			TerminalWriteLine("Data Valid", false);
			TerminalFlush();
		}
		else
		{
			// reset GPS data
			UpdateLocation(0.0, 0.0, 0.0, 0.0, true);

			UpdateDataValidLed(false);
			UpdatePrevDataValidLed(false);

			TerminalWriteLine("No Data", false);
			TerminalFlush();
		}
		update_gps_data = false;
	}
	else if( is_auto_update == true )
	{
		auto_update_timer++;
	}
}

void GPS_HandleStollenMode()
{
	String msgString;

	if( operation_mode == MODE_STOLLEN && stollen_mode_update_timer >= stollen_mode_update_duration*10 )
	{
		if( gps_data.data_valid == true || gps_data.prev_data_valid == true )
		{
			// Send email with the vehicle location every stollen_mode_update_duration seconds
			//sprintf(msgString, "Loc: %.06f, %.06f ,Speed: %.06f, Time: %d/%d/%d-%d:%d:%d",
			//gps_data.latitude, gps_data.longitude, gps_data.speed, GetDay(), GetMonth(), GetYear(), GetHour(), GetMinute(), GetSecond());

      UpdateLocation((float)gps_data.latitude, (float)gps_data.longitude, (float)gps_data.speed, (float)gps_data.satellites, false);
      UpdateMapLocation(1, (float)gps_data.latitude, (float)gps_data.longitude);
      
     msgString = "Loc: " + String(gps_data.latitude,6) + ", " + String(gps_data.longitude,6) + " ,Speed: " + String( gps_data.speed,2)
                 +  ", Time: " +  String(GetDay())+"/"+String(GetMonth())+"/"+String(GetYear())+"-"+String(GetHour())+":"+String(GetMinute())+":"+String(GetSecond());
			SendEmail("Accent GPS Update", msgString);
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


void GPS_HandleGeofencingMode()
{
  String msg; 
  if( geofencing_check_enable == true )
  {
	  if( gps_data.is_vehicle_parked == false)
	  {
    		if( park_location_check_timer_1 >= park_location_check_timer_1_Duration*10) // Check every 1 min.
    		{
    			// Calculate the distance between current vehicle location and the previous recorded location
    			float travelDistance = (float)TinyGPSPlus::distanceBetween( gps_data.latitude, gps_data.longitude, prev_lat, prev_lon ) ;
    			gps_data.travelDistance = travelDistance;
    
          //sprintf(msg, "MoveDist = %.2f, Speed = %.2f", travelDistance, gps_data.speed );
          msg = "MovedDist = " + String(travelDistance,2) +  ", Speed = " + String(gps_data.speed,2);
    	#ifdef GPS_DEBUG_ENABLE 
    			Serial.println(msg);
    	#endif
          if( print_geofencing_data_enable == true )
          {
            TerminalWriteLine(msg, false);
            TerminalFlush();
          }
    
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
    
    				//sprintf(msg, "Accent parked at: %.06f, %.06f , Time: %d/%d/%d-%d:%d:%d", gps_data.park_location_lat, gps_data.park_location_lon, GetDay(), GetMonth(), GetYear(), GetHour(), GetMinute(), GetSecond());
    				msg = "Accent parked at: " + String(gps_data.park_location_lat,6) + ", " + String(gps_data.park_location_lon,6) 
    				      +  " , Time: " +  String(GetDay())+"/"+String(GetMonth())+"/"+String(GetYear())+"-"+String(GetHour())+":"+String(GetMinute())+":"+String(GetSecond());
    				
            TerminalWriteLine(msg, true);
    				TerminalFlush();
    
            if( geofencing_send_email == true )
            {
              SendEmail("Accent GPS Update", msg);
            }

            BLYNK_UpdateVehicleState(gps_data.is_vehicle_parked);

            UpdateLocation((float)gps_data.park_location_lat, (float)gps_data.park_location_lon, (float)gps_data.speed, (float)gps_data.satellites, false);
            UpdateMapLocation(1, (float)gps_data.park_location_lat, (float)gps_data.park_location_lon);
    
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
    
          //sprintf(msg, "ParkDist = %.2f, Speed = %.2f", travelDistance, gps_data.speed );
          msg = "ParkDist = " + String(travelDistance,2) + ", Speed = " + String(gps_data.speed,2); 
#ifdef GPS_DEBUG_ENABLE 
    			Serial.println(msg);
#endif
    
          if( print_geofencing_data_enable == true )
          {
            TerminalWriteLine(msg, false);
            TerminalFlush();
          }
    
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
    
    				//sprintf(msg, "Accent Moved at: %.06f, %.06f , Time: %d/%d/%d-%d:%d:%d", gps_data.latitude, gps_data.longitude, GetDay(), GetMonth(), GetYear(), GetHour(), GetMinute(), GetSecond());
    				
    				msg = "Accent Moved at: " + String(gps_data.latitude,6) + ", " + String(gps_data.longitude,6) 
                  +  " , Time: " +  String(GetDay())+"/"+String(GetMonth())+"/"+String(GetYear())+"-"+String(GetHour())+":"+String(GetMinute())+":"+String(GetSecond());
    				TerminalWriteLine(msg, true);
    				TerminalFlush();
    
    				//String currentTime = String(GetDay()) + "/" + GetMonth() + "/" + GetYear() + "-" + String(GetHour()) + ":" + GetMinute() + ":" + GetSecond();
    				//Stringmessage = "Accent Moved at: " + currentTime  ;
    				//SendNotification(msg) ;
    
            if( geofencing_send_email == true )
            {
              SendEmail("Accent GPS Update", msg);
            }

            BLYNK_UpdateVehicleState(gps_data.is_vehicle_parked);

            UpdateLocation((float)gps_data.latitude, (float)gps_data.longitude, (float)gps_data.speed, (float)gps_data.satellites, false);
            UpdateMapLocation(1, (float)gps_data.latitude, (float)gps_data.longitude);
    
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
strGPS_DATA GetGPSData()
{
  return gps_data;
}
*/
void GPS_SetParkTimer1Duration(int duration)
{
	park_location_check_timer_1_Duration = duration;
}
void GPS_SetParkTimer2Duration(int duration)
{
	park_location_check_timer_2_Duration = duration;
}
void GPS_SetGeofencingTimer1Duration(int duration)
{
	geofencing_check_timer_1_Duration = duration;
}
void GPS_SetGeofencingTimer2Duration(int duration)
{
	geofencing_check_timer_2_Duration = duration;
}

void GPS_SetGeofencingRadius(float radius)
{
	geofencing_check_radius = radius;
} 

void GPS_SetUpdateGpsData(bool state)
{
	update_gps_data = state;
}

void GPS_SetIsAutoUpdate(bool state)
{
	is_auto_update = state;
}
void GPS_ResetAutoUpdateTimer()
{
	auto_update_timer = 0;
}
void GPS_SetAutoUpdateTimerDuration(int duration)
{
	auto_update_timer_duration = duration;
}

void GPS_SetOperationMode(int mode)
{
	operation_mode = mode;
}
void GPS_ResetStollenModeUpdateTimer()
{
	stollen_mode_update_timer = 0;
}
void GPS_SetStollenModeUpdateDuration(int duration)
{
	stollen_mode_update_duration = duration;
}
void GPS_SetGeofencingCheckEnable(bool state)
{
	geofencing_check_enable = state;
}
void GPS_SetPrintGeofencingDataEnable(bool state)
{
  print_geofencing_data_enable = state;
}
void GPS_SetGeofencingSendEmail(bool state)
{
  geofencing_send_email = state;
}

void GPS_SetVehicleStatus(bool state)
{
  gps_data.is_vehicle_parked = state;
}
