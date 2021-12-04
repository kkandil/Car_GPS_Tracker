
#include "OBDIIHandler.h"
#include <mcp_can.h>
#include <SPI.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MCP2515 Variables

#define standard 1
// 7E0/8 = Engine ECM
// 7E1/9 = Transmission ECM
#if standard == 1
  #define LISTEN_ID 0x7EA
  #define REPLY_ID 0x7E0
  #define FUNCTIONAL_ID 0x7DF
#else
  #define LISTEN_ID 0x98DAF101
  #define REPLY_ID 0x98DA01F1
  #define FUNCTIONAL_ID 0x98DB33F1
#endif 

char msgString[128];                        // Array to store serial string

// CAN Interrupt and Chip Select Pins
int CAN0_INT = PB4;                              /* Set INT to pin 2 (This rarely changes)   */
//MCP_CAN CAN0(PA4);                                /* Set CS to pin 9 (Old shields use pin 10) */
MCP_CAN* CAN0 ;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  LOCAL Variables


#define OBDII_DEBUG

enOBD_STATE current_state = IDLE_STATE;
 

strOBD_Data OBD_Data;

unsigned long obd_responce_timeout = 100;
unsigned long obd_responce_timer = 0;

bool data_ready = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Globale Functions

OBDIIHandler::OBDIIHandler(int CS_pin, int INT_pin) { 
    CAN0 = new MCP_CAN(CS_pin);
    CAN0_INT = INT_pin;
  }

OBDIIHandler::~OBDIIHandler(void) { }


bool OBDIIHandler::Begin()
{
  bool Status = false;
 
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0->begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Status = true;
  } else{ 
    return false;
  }


  #if standard == 1
  // Standard ID Filters
  CAN0->init_Mask(0,0x7F00000);                // Init first mask...
  CAN0->init_Filt(0,0x7DF0000);                // Init first filter...
  CAN0->init_Filt(1,0x7E10000);                // Init second filter...

  CAN0->init_Mask(1,0x7F00000);                // Init second mask...
  CAN0->init_Filt(2,0x7DF0000);                // Init third filter...
  CAN0->init_Filt(3,0x7E10000);                // Init fouth filter...
  CAN0->init_Filt(4,0x7DF0000);                // Init fifth filter...
  CAN0->init_Filt(5,0x7E10000);                // Init sixth filter...

#else
  // Extended ID Filters
  CAN0->init_Mask(0,0x90FF0000);                // Init first mask...
  CAN0->init_Filt(0,0x90DA0000);                // Init first filter...
  CAN0->init_Filt(1,0x90DB0000);                // Init second filter...

  CAN0->init_Mask(1,0x90FF0000);                // Init second mask...
  CAN0->init_Filt(2,0x90DA0000);                // Init third filter...
  CAN0->init_Filt(3,0x90DB0000);                // Init fouth filter...
  CAN0->init_Filt(4,0x90DA0000);                // Init fifth filter...
  CAN0->init_Filt(5,0x90DB0000);                // Init sixth filter...
#endif

  CAN0->setMode(MCP_NORMAL);                      // Set operation mode to normal so the MCP2515 sends acks to received data.

  // Having problems?  ======================================================
  // If you are not receiving any messages, uncomment the setMode line below
  // to test the wiring between the Ardunio and the protocol controller.
  // The message that this sketch sends should be instantly received.
  // ========================================================================
  //CAN0.setMode(MCP_LOOPBACK);

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input

 
  return Status;
} 





bool OBDIIHandler::ReadOBDData(unsigned long* rxID, byte* dlc, byte* rxBuf)
{
  if(!digitalRead(CAN0_INT)){                         // If CAN0_INT pin is low, read receive buffer
    CAN0->readMsgBuf(rxID, dlc, rxBuf);             // Get CAN data   
    return true;
  }

  return false;
}
bool OBDIIHandler::SendPIDRequest(enOBD_STATE PID)
{
    byte txData[] = {0x02,0x01,0x00,0x55,0x55,0x55,0x55,0x55};
    txData[2] = (byte) PID;
    
    if(CAN0->sendMsgBuf(FUNCTIONAL_ID, 8, txData) == CAN_OK)
    {
      current_state = PID;
      obd_responce_timer = millis(); 
      
      #ifdef OBDII_DEBUG
      sprintf(msgString, "PID 0x%.2X Request Sent Successfully", PID); 
      Serial1.println(msgString);
      #endif
    } 
    else 
    { 
      #ifdef OBDII_DEBUG
      sprintf(msgString, "PID 0x%.2X Error Sending Request", PID); 
      Serial1.println(msgString); 
      #endif
      
      return false;
    } 
    return true;
}

void OBDIIHandler::HandleOBD()
{
  unsigned long rxID;
  byte dlc;
  byte rxBuf[8];
  bool Status = false;
  
  switch(current_state)
  {
    case IDLE_STATE:
    break;

    // PID: 05 
    case WAIT_RESPONCE_ENGINE_COL_TEMP_05:
        Status = ReadOBDData(&rxID, &dlc, rxBuf);
        if( Status == true )
        {
           if( rxBuf[0] == 0x03 && rxBuf[1] == 0x41 && rxBuf[2] == 0x05 )
           {
              OBD_Data.engine_col_temp = ((int)rxBuf[3]) - 40;
              OBD_Data.engine_col_temp_val = 0;
           }
           else
           {
              OBD_Data.engine_col_temp_val = -1;
           }
           #ifdef OBDII_DEBUG
           //sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxID, dlc);
           //Serial1.println(msgString);
           sprintf(msgString, "engine_col_temp: %d, Val: %d", OBD_Data.engine_col_temp, OBD_Data.engine_col_temp_val);
           Serial1.println(msgString);
           #endif
           
           SendPIDRequest(WAIT_RESPONCE_ENGINE_SPEED_0C);
           current_state = WAIT_RESPONCE_ENGINE_SPEED_0C; 
        }
        else if( (millis() - obd_responce_timer) > obd_responce_timeout )
        {
          OBD_Data.engine_col_temp_val = -2; 

          #ifdef OBDII_DEBUG
          Serial1.println("TimeOut.....");
          #endif
          
          SendPIDRequest(WAIT_RESPONCE_ENGINE_SPEED_0C);
          current_state = WAIT_RESPONCE_ENGINE_SPEED_0C; 
        }
    break;

    // PID: 0C
    case WAIT_RESPONCE_ENGINE_SPEED_0C:
        Status = ReadOBDData(&rxID, &dlc, rxBuf);
        if( Status == true )
        {
           if( rxBuf[0] == 0x04 && rxBuf[1] == 0x41 && rxBuf[2] == 0x0C )
           {
              OBD_Data.engine_speed = (256.0*((double)rxBuf[3]) + ((double)rxBuf[4]))/4.0 ;
              OBD_Data.engine_speed_val = 0;
           }
           else
           {
              OBD_Data.engine_speed_val = -1;
           } 
           #ifdef OBDII_DEBUG
           sprintf(msgString, "engine_speed: %.2lf, Val: %d", OBD_Data.engine_speed, OBD_Data.engine_speed_val);
           Serial1.println(msgString);
           #endif
           
           SendPIDRequest(WAIT_RESPONCE_VEHICLE_SPEED_0D);
           current_state = WAIT_RESPONCE_VEHICLE_SPEED_0D; 
        }
        else if( (millis() - obd_responce_timer) > obd_responce_timeout )
        {
          OBD_Data.engine_speed_val = -2; 
          
          #ifdef OBDII_DEBUG
          Serial1.println("TimeOur.....");
          #endif
          
          SendPIDRequest(WAIT_RESPONCE_VEHICLE_SPEED_0D);
          current_state = WAIT_RESPONCE_VEHICLE_SPEED_0D; 
        }
    break;

    // PID: 0D
    case WAIT_RESPONCE_VEHICLE_SPEED_0D:
        Status = ReadOBDData(&rxID, &dlc, rxBuf);
        if( Status == true )
        {
           if( rxBuf[0] == 0x03 && rxBuf[1] == 0x41 && rxBuf[2] == 0x0D )
           {
              OBD_Data.vehicle_speed = ((int)rxBuf[3]);
              OBD_Data.vehicle_speed_val = 0;
           }
           else
           {
              OBD_Data.engine_col_temp_val = -1;
           } 
           #ifdef OBDII_DEBUG
           sprintf(msgString, "engine_col_temp: %d, Val: %d", OBD_Data.vehicle_speed, OBD_Data.vehicle_speed_val);
           Serial1.println(msgString);
           #endif
           
           SendPIDRequest(WAIT_RESPONCE_THROTTLE_POS_11);
           current_state = WAIT_RESPONCE_THROTTLE_POS_11; 
        }
        else if( (millis() - obd_responce_timer) > obd_responce_timeout )
        {
          OBD_Data.vehicle_speed_val = -2; 
          #ifdef OBDII_DEBUG
          Serial1.println("TimeOur.....");
          #endif
          
          SendPIDRequest(WAIT_RESPONCE_THROTTLE_POS_11);
          current_state = WAIT_RESPONCE_THROTTLE_POS_11; 
        }
    break;

    // PID: 11
    case WAIT_RESPONCE_THROTTLE_POS_11:
        Status = ReadOBDData(&rxID, &dlc, rxBuf);
        if( Status == true )
        {
           if( rxBuf[0] == 0x03 && rxBuf[1] == 0x41 && rxBuf[2] == 0x11 )
           {
              OBD_Data.throttle_pos = (int)((100.0/255.0)*((float)rxBuf[3]));
              OBD_Data.throttle_pos_val = 0;
           }
           else
           {
              OBD_Data.engine_col_temp_val = -1;
           }  
           #ifdef OBDII_DEBUG
           sprintf(msgString, "throttle_pos: %d, Val: %d", OBD_Data.throttle_pos, OBD_Data.throttle_pos_val);
           Serial1.println(msgString); 
           #endif
           
           SendPIDRequest(WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F);
           current_state = WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F; 
        }
        else if( (millis() - obd_responce_timer) > obd_responce_timeout )
        {
          OBD_Data.throttle_pos_val = -2; 
          #ifdef OBDII_DEBUG
          Serial1.println("TimeOur.....");
          #endif
          
          SendPIDRequest(WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F);
          current_state = WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F; 
        }
    break;

    // PID: 1F
    case WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F:
        Status = ReadOBDData(&rxID, &dlc, rxBuf);
        if( Status == true )
        {
           if( rxBuf[0] == 0x04 && rxBuf[1] == 0x41 && rxBuf[2] == 0x1F )
           {
              OBD_Data.runtime_since_engine_start = 256*((int)rxBuf[3]) + ((int)rxBuf[4]) ;
              OBD_Data.runtime_since_engine_start_val = 0;
           }
           else
           {
              OBD_Data.engine_speed_val = -1;
           } 
           #ifdef OBDII_DEBUG
           sprintf(msgString, "runtime_since_engine_start: %d, Val: %d", OBD_Data.runtime_since_engine_start, OBD_Data.runtime_since_engine_start_val);
           Serial1.println(msgString);
           #endif
           
           current_state = IDLE_STATE; 

           data_ready = true;
        }
        else if( (millis() - obd_responce_timer) > obd_responce_timeout )
        {
          OBD_Data.runtime_since_engine_start_val = -2; 
          #ifdef OBDII_DEBUG
          Serial1.println("TimeOur.....");
          #endif
          
          current_state = IDLE_STATE; 
          data_ready = true;
        }
    break;
    case WAIT_SEGMENTED:
    break;
  }
}

bool OBDIIHandler::GetDataReady()
{
   return data_ready;
}

bool OBDIIHandler::SetDataReady(bool data_ready_state)
{
   data_ready = data_ready_state;
}


strOBD_Data OBDIIHandler::GetPIDData()
{
  return OBD_Data; 
}
