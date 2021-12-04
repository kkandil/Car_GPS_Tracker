

class GPSHandler;


struct strGPS_DATA{
  float latitude;
  float longitude;
  float speed;
  float satellites;
  bool data_valid;
  bool prev_data_valid; 

  float home_lat;
  float home_lon;
  bool isAtHome;
  float travelDistance;
};



class GPSHandler {
public:
  GPSHandler();
  ~GPSHandler();

  bool Begin();

  void UpdateGPSData();

  void smartDelay(unsigned long ms);

  strGPS_DATA GetGPSData();
  void SetTimer1Duration(int duration);
  void SetTimer2Duration(int duration);
  void SetTimer3Duration(int duration);
  void SetStandStillThreshold(float threshold);
};
