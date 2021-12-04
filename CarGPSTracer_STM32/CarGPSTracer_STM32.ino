
#include "BLYNK.h"
#include "CarGPSTracer_STM32.h"
#include "GPSHandler.h"
#include "OBDIIHandler.h"



// OBDII module
OBDIIHandler OBDII(PA4, PB0);
strOBD_Data obdData;


GPSHandler GPS;

//BLYNK BLYNK;
BLYNK BLYNK;
 
void setup() {
	//Set Serial monitor baud rate
	Serial1.begin(115200);

	// Init GPS module
	GPS.Begin();

	//Set GSM module baud rate
	Serial2.begin(9600);

	// Init OBDII driver
	OBDII.Begin();

	delay(3000);

	BLYNK.Begin();

}

 
void loop() {
	BLYNK.Run();
	GPS.smartDelay(0);
	OBDII.HandleOBD();
} //main loop ends

void gpsUpdateTimer()
{
	char msgString[128];

	GPS.UpdateGPSData();

	GPS.HandleLocationUpdate();

	GPS.HandleStollenMode();

	GPS.HandleGeofencingMode();

	OBDII.UpdatePIDData();

}

