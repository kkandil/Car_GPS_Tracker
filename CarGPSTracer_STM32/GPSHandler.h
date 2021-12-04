

class GPSHandler;


struct strGPS_DATA{
  float latitude;
  float longitude;
  float speed;
  float satellites;
  bool data_valid;
  bool prev_data_valid; 

  float park_location_lat;
  float park_location_lon;
  bool is_vehicle_parked;
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
  void SetParkTimer1Duration(int duration);
  void SetParkTimer2Duration(int duration);
  void SetGeofencingTimer1Duration(int duration);
  void SetGeofencingTimer2Duration(int duration);
  void SetGeofencingRadius(float radius);
};
