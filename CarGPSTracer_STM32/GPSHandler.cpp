#include "GPSHandler.h"
#include <TinyGPS++.h>  

#define GPS_DEBUG_ENABLE

//GPS Module Settings 
TinyGPSPlus gps; 


strGPS_DATA gps_data;

void CheckVehicleTravelStatus();
 

GPSHandler::GPSHandler(){} 

GPSHandler::~GPSHandler(){}

bool GPSHandler::Begin()
{
  //Set GPS module baud rate 
  Serial3.begin(9600); 
  
  #ifdef GPS_DEBUG_ENABLE
  Serial1.println("neogps serial initialize"); 
  #endif  
  
  delay(10); 
}

float prev_lat = 0.0, prev_lon = 0.0;
int home_location_check_timer_1 = 0;
int home_location_check_timer_2 = 0;
int home_location_check_timer_3 = 0;
int home_location_check_timer_1_Duration = 600; // 60 sec.
int home_location_check_timer_2_Duration = 30;  // 30 mins
int home_location_check_timer_3_Duration = 300;  // 30 sec
float travelDistance = 0.0;
float traveDistanceThreshold = 30.0;

void GPSHandler::UpdateGPSData()
{ 
  gps_data.data_valid = gps.location.isUpdated(); 
  
  if ( gps_data.data_valid )
  {
    gps_data.latitude = (gps.location.lat());
    gps_data.longitude = (gps.location.lng());
    //get
    gps_data.speed = gps.speed.kmph();
    //get number of satellites
    gps_data.satellites = gps.satellites.value(); 
    gps_data.prev_data_valid = true;    
  }

  CheckVehicleTravelStatus();
  
  

  
}

void CheckVehicleTravelStatus()
{
  char msg[125];
  if( gps_data.isAtHome == false)
  {
    if( home_location_check_timer_1 >= home_location_check_timer_1_Duration) // Check every 1 min.
    {
      float travelDistance = (float)TinyGPSPlus::distanceBetween( gps_data.latitude, gps_data.longitude, prev_lat, prev_lon ) ; 

      gps_data.travelDistance = travelDistance;
      
      
      if( travelDistance < traveDistanceThreshold )
      {
        home_location_check_timer_2++;
      }
      else
      {
        home_location_check_timer_2 = 0;
      }
  
      if( home_location_check_timer_2 >= home_location_check_timer_2_Duration ) 
      {
        gps_data.home_lat = gps_data.latitude;
        gps_data.home_lon = gps_data.longitude;
        gps_data.isAtHome = true;
        home_location_check_timer_2 = 0;
        
        #ifdef GPS_DEBUG_ENABLE
        Serial.println("vehcile At home");
        #endif
      }
      #ifdef GPS_DEBUG_ENABLE
      sprintf(msg, "Dist = %.2f", travelDistance);
      Serial.println(msg);
      #endif
      prev_lat = gps_data.latitude;
      prev_lon = gps_data.longitude;
      home_location_check_timer_1 = 0;
    }
    else
    {
      home_location_check_timer_1++;
    }
  }
  else
  {
    if( home_location_check_timer_3 >= home_location_check_timer_3_Duration ) // Check every 30 sec.
    {
      float travelDistance = (float)TinyGPSPlus::distanceBetween( gps_data.latitude, gps_data.longitude, prev_lat, prev_lon ) ;

      gps_data.travelDistance = travelDistance;
      if( travelDistance > traveDistanceThreshold )
      {
        gps_data.isAtHome = false;
        #ifdef GPS_DEBUG_ENABLE
        Serial.println("vehcile moved");
        #endif
      }

      prev_lat = gps_data.latitude;
      prev_lon = gps_data.longitude;
      home_location_check_timer_3 = 0;
      
      #ifdef GPS_DEBUG_ENABLE
      sprintf(msg, "Dist2 = %.2f", travelDistance);
      Serial.println(msg);
      #endif
      
       
    }
    else
    {
      home_location_check_timer_3++;
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

 
void GPSHandler::SetTimer1Duration(int duration)
{
  home_location_check_timer_1_Duration = duration;
}
void GPSHandler::SetTimer2Duration(int duration)
{
  home_location_check_timer_2_Duration = duration;
}
void GPSHandler::SetTimer3Duration(int
duration)
{
  home_location_check_timer_3_Duration = duration;
}

void GPSHandler::SetStandStillThreshold(float threshold)
{
  traveDistanceThreshold = threshold; 
} 
