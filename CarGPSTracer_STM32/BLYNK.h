

//class BLYNK;

extern void gpsUpdateTimer();


bool BLYNKBegin();

void BLYNKRun();

void SendEmail(String subject, String body);
void SendNotification(char *msg);
void UpdateLocation(float latitude, float longitude, float speed, float satellites, bool reset);
void UpdateMapLocation(int pointIndex, float latitude, float longitude);
void UpdateDataValidLed(bool state);
void UpdatePrevDataValidLed(bool state);

void UpdatePIDData(int engine_col_temp, double engine_speed, int vehicle_speed, int throttle_pos, double runtime_since_engine_start);

void TerminalWriteLine(String msg, bool forcePrint);
void TerminalWrite(String msg);
void TerminalFlush();
void BLYNK_UpdateVehicleState(bool state);
int GetDay();
int GetMonth();
int GetYear();
int GetHour();
int GetMinute();
int GetSecond();

float GetLastKnownLat(void);
float GetLastKnownLon(void);
/*
class BLYNK {
public:
  BLYNK();
  ~BLYNK();

  bool Begin();

  void Run();

  void SendEmail(char *subject, char *body);
  void SendNotification(char *msg);
  void UpdateLocation(float latitude, float longitude, float speed, float satellites, bool reset);
  void UpdateMapLocation(int pointIndex, float latitude, float longitude);
  void UpdateDataValidLed(bool state);
  void UpdatePrevDataValidLed(bool state);

  void UpdatePIDData(int engine_col_temp, double engine_speed, int vehicle_speed, int throttle_pos, double runtime_since_engine_start);

  void TerminalWriteLine(char *msg);
  void TerminalWrite(char *msg);
  void TerminalFlush();
  int GetDay();
  int GetMonth();
  int GetYear();
  int GetHour();
  int GetMinute();
  int GetSecond();
 
};
*/
 
