#pragma once

#include <string>
#include <vector>

#include <cpprest/http_client.h>

namespace weatherserver {

  /*WeatherData class represents a JSON object returned from the Dark Sky API.

  Example (excluding some blocks):

    {"latitude":47.035,"longitude":-84.3811,"timezone":"America/Toronto","currently":
      {"time":1553128225,"icon":"Flurries","icon":"snow","nearestStormDistance":0,
       "precipIntensity":0.0052,"precipProbability":0.27,"precipType":"snow","temperature":34.07,
       "apparentTemperature":34.07,"dewPoint":32.53,"humidity":0.94,"pressure":1014.45,"windSpeed":2.81,
       "windGust":6.2,"windBearing":172,"cloudCover":0.98,"uvIndex":0,"visibility":1.22,"ozone":407.48},
     "flags":{"sources":["nearest-precip","ecpa","cmc","gfs","hrrr","icon","isd","madis","nam","sref","darksky"],
              "darksky-unavailable":"Dark Sky covers the given location, but all stations are currently unavailable.","units":"us"},
     "offset":-4}
  */
  class WeatherData {

  public:

    WeatherData(double latitude, double longitude, std::string timezone, std::string icon, double temperature,
      double apparentTemperature, double humidity, double pressure, double windSpeed, double windBearing, double cloudCover);

    WeatherData();

    /*
    Gets JSON value AS AN OBJECT and parses it to a new WeatherData object.

    @param web::json::value& json - JSON object returned from the request to Dark Sky API.
    */
    static WeatherData parseJson(web::json::value& json);

    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }
    const std::string& getTimezone() const { return timezone; }
    const std::string& getCurrentlyIcon() const { return icon; }
    double getCurrentlyTemperature() const { return temperature; }
    double getCurrentlyApparentTemperature() const { return apparentTemperature; }
    double getCurrentlyHumidity() const { return humidity; }
    double getCurrentlyPressure() const { return pressure; }
    double getCurrentlyWindSpeed() const { return windSpeed; }
    double getCurrentlyWindBearing() const { return windBearing; }
    double getCurrentlyCloudCover() const { return cloudCover; }

    //String constants representing "names" in name/value pairs of JSON objects representing weather data.
    static const utility::string_t KEY_LATITUDE;
    static const utility::string_t KEY_LONGITUDE;
    static const utility::string_t KEY_TIMEZONE;
    static const utility::string_t KEY_ICON;
    static const utility::string_t KEY_TEMPERATURE;
    static const utility::string_t KEY_APARENT_TEMPERATURE;
    static const utility::string_t KEY_HUMIDIY;
    static const utility::string_t KEY_PRESSURE;
    static const utility::string_t KEY_WINDSPEED;
    static const utility::string_t KEY_WINDBEARING;
    static const utility::string_t KEY_CLOUD_COVER;
    static const utility::string_t KEY_CURRENTLY;

    //C-style strings representing display names of the nodes in OPC UA information model.
    static char BROWSE_LATITUDE[];
    static char BROWSE_LONGITUDE[];
    static char BROWSE_TIMEZONE[];
    static char BROWSE_ICON[];
    static char BROWSE_TEMPERATURE[];
    static char BROWSE_APPARENT_TEMPERATURE[];
    static char BROWSE_HUMIDITY[];
    static char BROWSE_PRESSURE[];
    static char BROWSE_WIND_SPEED[];
    static char BROWSE_WIND_BEARING[];
    static char BROWSE_CLOUD_COVER[];

  private:

    double latitude;
    double longitude;
    std::string timezone;
    std::string icon;
    double temperature;
    double apparentTemperature;
    double humidity;
    double pressure;
    double windSpeed;
    double windBearing;
    double cloudCover;
  };
}

