
#include "BLYNK.h"
#include "CarGPSTracer_STM32.h"
#include "GPSHandler.h"
#include "OBDIIHandler.h"




void setup() {
 

	//Set Serial monitor baud rate
	Serial1.begin(115200);

	// Init GPS module
	GPS_Begin();

	//Set GSM module baud rate
	Serial2.begin(9600);

	// Init OBDII driver
	OBDIIBegin(PA4, PB0);

	delay(3000);

	BLYNKBegin(); 
 
}

 
void loop() {  
	BLYNKRun();
	GPS_SmartDelay(0);
	HandleOBD();  
} //main loop ends

void gpsUpdateTimer(void)
{
	char msgString[128];

	GPS_UpdateGPSData();

	GPS_HandleLocationUpdate();

	GPS_HandleStollenMode();

	GPS_HandleGeofencingMode();

	UpdatePIDData();

}
