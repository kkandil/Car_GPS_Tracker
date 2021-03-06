
#include <Arduino.h>


enum enOBD_STATE {
  IDLE_STATE = 0,
  WAIT_RESPONCE_ENGINE_COL_TEMP_05 = 0x05,
  WAIT_RESPONCE_ENGINE_SPEED_0C = 0x0C,
  WAIT_RESPONCE_VEHICLE_SPEED_0D = 0x0D,
  WAIT_RESPONCE_THROTTLE_POS_11 = 0x11,
  WAIT_RESPONCE_RUNTIME_SINCE_ENGINE_START_1F = 0x1F,
  WAIT_SEGMENTED,
};

struct strOBD_Data{
  int engine_col_temp;
  int engine_col_temp_val;
  double engine_speed;
  int engine_speed_val;
  int vehicle_speed;
  int vehicle_speed_val;
  int throttle_pos;
  int throttle_pos_val;
  int runtime_since_engine_start;
  int runtime_since_engine_start_val;
};


class OBDIIHandler;


class OBDIIHandler {
public:
  OBDIIHandler(int CS_pin, int INT_pin);
  ~OBDIIHandler();

  bool Begin();

  bool ReadOBDData(unsigned long* rxID, byte* dlc, byte* rxBuf);

  bool SendPIDRequest(enOBD_STATE PID);

  void HandleOBD();

  bool GetDataReady();

  bool SetDataReady(bool data_ready_state);

  strOBD_Data GetPIDData();
};
