#pragma once

#include <string>
#include <map>
#include <chrono>
#include <ctime>

#include <cpprest/http_client.h>

#include "WeatherData.h"

namespace weatherserver {

  /*
  LocationData class represents a JSON object returned from the Open AQ API.

  Examples:

    {"location":"ALGOMA","city":"ONTARIO","country":"CA","count":9290,"sourceNames"["AirNow"],
     "lastUpdated":"2018-08-01T15:00:00.000Z","firstUpdated":"2016-03-06T19:00:00.000Z","parameters"["o3"],
     "distance":5907356,"sourceName":"AirNow","coordinates":{"latitude":47.035,"longitude":-84.3811}},

    {"location":"AYLESFORD MOUNTAIN","city":"NOVA SCOTIA","country":"CA","count":42184,"sourceNames":"AirNow"],
     "lastUpdated":"2019-03-16T16:00:00.000Z","firstUpdated":"2016-03-06T19:00:00.000Z","parameters"["no2","o3","pm25"],
     "distance":4765644,"sourceName":"AirNow","coordinates":{"latitude":45.0061,"longitude":-65}}
  */
  class LocationData {

  public:

    //Default constructor to use in STL classes.
    LocationData();

    LocationData(std::string name, std::string city, std::string countryCode, double latitude, double longitude,
      bool hasBeenReceivedWeatherData = false, bool isInitialized = false, bool isAddingWeatherToAddressSpace = false);

    /*
    Gets JSON value AS AN OBJECT and parses it to a new LocationData object.

    @param json - one of the JSON values inside JSON array returned from the request to Open AQ API.
    */
    static LocationData parseJson(web::json::value& json);

    /*
    Gets JSON value AS AN ARRAY and parses every JSON object inside into LocationData object, adding it to a map if it has correct values.
    Correct values - longitude & latitude are within 0-180 range.

    @param jsonArray - array of JSON values returned from Open AQ API request.
    @return map of LocationData objects with LocationData.name as a key.
    */
    static std::map<std::string, LocationData> parseJsonArray(web::json::value& jsonArray);

    //Identifies that this object node was added to the OPC UA information model.
    void setIsInitialized(const bool initialized);

    //Identifies that this object has received weather data for the first time.
    void setHasBeenReceivedWeatherData(const bool received);

    //Identifies that weather data variable nodes are currently being added to the OPC UA information model.
    void setIsAddingWeatherToAddressSpace(const bool addingWeatherToAddressSpace);

    void setWeatherData(const WeatherData& weather);
    void setReadLastTime(const std::chrono::system_clock::time_point& time);

    const std::string& getName() const { return name; }
    const std::string& getCity() const { return city; }
    const std::string& getCountryCode() const { return countryCode; }
    double getLatitude() const { return latitude; }
    double getLongitude() const { return longitude; }

    bool getIsInitialized() const { return isInitialized; }
    bool getHasBeenReceivedWeatherData() const { return hasBeenReceivedWeatherData; }
    bool getIsAddingWeatherToAddressSpace() const { return isAddingWeatherToAddressSpace; }
    WeatherData& getWeatherData() { return weatherData; }
    std::chrono::system_clock::time_point getReadLastTime() const { return readLastTime; }

    //String constants representing "names" in name/value pairs of JSON objects representing location data.
    static const utility::string_t KEY_NAME;
    static const utility::string_t KEY_CITY_NAME;
    static const utility::string_t KEY_COUNTRY_CODE;
    static const utility::string_t KEY_COORDINATES;
    static const utility::string_t KEY_LATITUDE;
    static const utility::string_t KEY_LONGITUDE;

    static const size_t INVALID_LATITUDE;
    static const size_t INVALID_LONGITUDE;

    //C-style strings representing display names of the nodes in OPC UA information model.
    static char BROWSE_FLAG_INITIALIZE[];

  private:

    std::string name;
    std::string city;
    std::string countryCode;
    double latitude;
    double longitude;
    bool hasBeenReceivedWeatherData;
    bool isInitialized;
    bool isAddingWeatherToAddressSpace;
    WeatherData weatherData;
    std::chrono::system_clock::time_point readLastTime;
  };
}
