 
#define MODE_NORMAL   0
#define MODE_STOLLEN  1

bool GPS_Begin();

void GPS_UpdateGPSData();

void GPS_SmartDelay(unsigned long ms);

void GPS_HandleLocationUpdate();
void GPS_HandleStollenMode();
void GPS_HandleGeofencingMode();
 
void GPS_SetGeofencingRadius(float radius);

void GPS_SetUpdateGpsData(bool state);
void GPS_SetIsAutoUpdate(bool state);
void GPS_ResetAutoUpdateTimer();
void GPS_SetAutoUpdateTimerDuration(int duration);
void GPS_SetOperationMode(int mode);
void GPS_ResetStollenModeUpdateTimer();
void GPS_SetStollenModeUpdateDuration(int duration);
void GPS_SetGeofencingCheckEnable(bool state);
void GPS_SetPrintGeofencingDataEnable(bool state);
void GPS_SetGeofencingSendEmail(bool state);
/*
class GPSHandler {
public:
  GPSHandler(BLYNK BLYNK);
  ~GPSHandler();

  bool Begin();

  void UpdateGPSData();

  void smartDelay(unsigned long ms);

  void HandleLocationUpdate();
  void HandleStollenMode();
  void HandleGeofencingMode();

  //strGPS_DATA GetGPSData();
  void SetParkTimer1Duration(int duration);
  void SetParkTimer2Duration(int duration);
  void SetGeofencingTimer1Duration(int duration);
  void SetGeofencingTimer2Duration(int duration);
  void SetGeofencingRadius(float radius);

  void SetUpdateGpsData(bool state);
  void SetIsAutoUpdate(bool state);
  void ResetAutoUpdateTimer();
  void SetAutoUpdateTimerDuration(int duration);
  void SetOperationMode(int mode);
  void ResetStollenModeUpdateTimer();
  void SetStollenModeUpdateDuration(int duration);
  void SetGeofencingCheckEnable(bool state);
  void SetPrintGeofencingDataEnable(bool state);
};
*/
