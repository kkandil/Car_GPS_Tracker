

class GPSHandler;




#define MODE_NORMAL   0
#define MODE_STOLLEN  1



class GPSHandler {
public:
  GPSHandler();
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
};
