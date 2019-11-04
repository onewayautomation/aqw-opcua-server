#pragma once

#include <string>
#include <map>

#include <cpprest/http_client.h>

#include "LocationData.h"

namespace weatherserver {

  /*
  CountryData class represents a JSON object returned from the Open AQ API.

  Examples:

    {"name":"Andorra","code":"AD","cities":2,"locations":3,"count":60285},
    {"name":"Argentina","code":"AR","cities":1,"locations":4,"count":14976}
  */
  class CountryData {

  public:

    //Default constructor, required to use with STL classes.
    CountryData();

    CountryData(std::string name, std::string code, uint32_t cities, uint32_t locations, bool isInitialized = false);

    /*
    Gets JSON value AS AN OBJECT and parses it to a new CountryData object.

    @param json - one of the JSON values inside JSON array returned from the request to Open AQ API.
    */
    static CountryData parseJson(web::json::value& json);

    /*
    Gets JSON value AS AN ARRAY and parses every JSON object inside into CountryData object, adding it to a map if it has correct values.
    Correct values - name & country code are not empty.

    @param jsonArray - array of JSON values returned from Open AQ API request.
    @return map of CountryData objects with two letter country code as a key.
    */
    static std::map<std::string, CountryData> parseJsonArray(web::json::value& jsonArray);

    //Identifies that this object node was added to the OPC UA information model.
    void setIsInitialized(const bool initialized);

    void setLocationsNumber(const uint32_t newLocationsNumber);

    void setLocations(const std::map<std::string, LocationData>& allLocations);

    const std::string& getName() const { return name; }
    const std::string& getCode() const { return code; }
    uint32_t getCitiesNumber() const { return citiesNumber; }
    uint32_t getLocationsNumber() const { return locationsNumber; }
    bool getIsInitialized() const { return isInitialized; }
    std::map<std::string, LocationData>& getLocations() { return locations; }

    //String constants representing "names" in name/value pairs of JSON objects representing country data.
    static const utility::string_t KEY_NAME;
    static const utility::string_t KEY_CODE;
    static const utility::string_t KEY_CITIES;
    static const utility::string_t KEY_LOCATIONS;

    //C-style strings representing display names of the nodes in OPC UA information model.
    static char COUNTRIES_FOLDER_NODE_ID[];
    static char BROWSE_NAME[];
    static char BROWSE_CODE[];
    static char BROWSE_CITIES_NUMBER[];
    static char BROWSE_LOCATIONS_NUMBER[];

  private:

    std::string name;
    std::string code;
    uint32_t citiesNumber;
    uint32_t locationsNumber;
    bool isInitialized;
    std::map<std::string, LocationData> locations;
  };
}
