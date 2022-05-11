
#include "BLYNK.h"
#include "CarGPSTracer_Arduino.h"
#include "GPSHandler.h" 




void setup() {
 

	//Set Serial monitor baud rate
	Serial.begin(115200);

	// Init GPS module
	GPS_Begin();

	//delay(3000);

	BLYNKBegin(); 
 
}

 
void loop() {  
	BLYNKRun();
	GPS_SmartDelay(0); 
} //main loop ends

void gpsUpdateTimer(void)
{
	char msgString[128];

	GPS_UpdateGPSData();

	GPS_HandleLocationUpdate();

	GPS_HandleStollenMode();

	GPS_HandleGeofencingMode();
 

}
