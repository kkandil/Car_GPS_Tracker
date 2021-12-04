/* CAN OBD & UDS Simple PID Request
 *
 *  Currently requests PID 0x00 at a 1 second interval and
 *  displays all received CAN traffic to the terminal at 115200.
 *
 *  Written By: Cory J. Fowler  April 5th, 2017
 *
 *  (Disclaimer: Standard IDs are currently UNTESTED against a vehicle)
 *
 */
 #include "OBDIIHandler.h"

 OBDIIHandler OBDII(PA4, PB6);
strOBD_Data OBDData;
char msg[128]; 

void setup(){

  Serial1.begin(115200);
  while(!Serial1);
 
  OBDII.Begin( );
 
} 
int first_entery = 0;
 
void loop(){

  OBDII.HandleOBD();
 if( first_entery == 0)
 {
    OBDII.SetDataReady(false);
    OBDII.SendPIDRequest(WAIT_RESPONCE_ENGINE_COL_TEMP_05);
    first_entery = 1;
 } 

 if( first_entery == 1 && OBDII.GetDataReady() == true )
 {
    OBDData = OBDII.GetPIDData();
    sprintf(msg, "engine_col_temp: %d, Val: %d", OBDData.engine_col_temp, OBDData.engine_col_temp_val);
    Serial1.println(msg);

    sprintf(msg, "engine_speed: %.2lf, Val: %d", OBDData.engine_speed, OBDData.engine_speed_val);
    Serial1.println(msg);

    sprintf(msg, "vehicle_speed: %d, Val: %d", OBDData.vehicle_speed, OBDData.vehicle_speed_val);
    Serial1.println(msg);

    sprintf(msg, "throttle_pos: %d, Val: %d", OBDData.throttle_pos, OBDData.throttle_pos_val);
    Serial1.println(msg); 

    sprintf(msg, "runtime_since_engine_start: %d, Val: %d", OBDData.runtime_since_engine_start, OBDData.runtime_since_engine_start_val);
    Serial1.println(msg);

    first_entery = 2;
 }
}
