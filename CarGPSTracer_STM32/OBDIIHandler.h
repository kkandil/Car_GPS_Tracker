
#include <Arduino.h>


enum enOBD_STATE {
  IDLE_STATE = 0,
  WAIT_RESPONCE_ENGINE_COL_TEMP_05 = 0x05,
  WAIT_RESPONCE_ENGINE_SPEED_0C = 0x0C,
  WAIT_RESPONCE_VEHICLE_SPEED_0D = 0x0D,
  WAIT_RESPONCE_THROTTLE_POS_11 = 0x11,
  WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F = 0x1F,
  WAIT_RESPONCE_ODOMETER_A6 = 0xA6,
  WAIT_RESPONCE_GENERIC = 0xFF,
  WAIT_SEGMENTED,
};

struct strOBD_Data{
  int engine_col_temp;
  int engine_col_temp_val;
  byte data_engine_col_temp[8];
  double engine_speed;
  int engine_speed_val;
  byte data_engine_speed[8];
  int vehicle_speed;
  int vehicle_speed_val;
  byte data_vehicle_speed[8];
  int throttle_pos;
  int throttle_pos_val;
  byte data_throttle_pos[8];
  int runtime_since_engine_start;
  int runtime_since_engine_start_val;
  byte data_runtime_since_engine_start[8];
  int odometer;
  int odometer_val;
  byte data_odometer[8];

  byte data[8];
  bool data_val;
};

bool OBDIIBegin(int CS_pin, int INT_pin);

void UpdatePIDData();

bool ReadOBDData(unsigned long* rxID, byte* dlc, byte* rxBuf);

bool SendPIDRequest(enOBD_STATE PID);

bool SendOBDRequest(int dlc, byte* data);

void HandleOBD();

//bool GetDataReady();

bool SetDataReady(bool data_ready_state);

strOBD_Data GetPIDData();

void SetUpdateObdData(bool state);
bool GetUpdateObdData();
void SetIsObdAutoUpdate(bool state);
void ResetObdAutoUpdateTimerCounter();

//class OBDIIHandler;

/*
class OBDIIHandler {
public:
  OBDIIHandler(int CS_pin, int INT_pin);
  OBDIIHandler();
  ~OBDIIHandler();

  bool Begin();

  void UpdatePIDData();

  bool ReadOBDData(unsigned long* rxID, byte* dlc, byte* rxBuf);

  bool SendPIDRequest(enOBD_STATE PID);

  bool SendOBDRequest(int dlc, byte* data);

  void HandleOBD();

  //bool GetDataReady();

  bool SetDataReady(bool data_ready_state);

  strOBD_Data GetPIDData();

  void SetUpdateObdData(bool state);
  bool GetUpdateObdData();
  void SetIsObdAutoUpdate(bool state);
  void ResetObdAutoUpdateTimerCounter();
};
*/
