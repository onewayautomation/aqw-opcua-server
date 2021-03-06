#pragma once

#include <fstream>
#include <iostream>

#include <cpprest/http_client.h>
#include "CountryData.h"
#include "LocationData.h"

namespace weatherserver {

  class Settings {

  public:

    /*
      Assign default values to variables.
      Attempt to open "settings.json" file to get Dark Sky API key and other settings to override dafault values.
      @settingsFilePath should contain full path to a settings file.
    */
    Settings(const std::string& settingsFilePath);

    bool areValid() const { return settingsAreValid; }

    const utility::string_t& getKeyApiDarksky() const { return keyApiDarksky; }
    const utility::string_t& getUnits() const { return units; }
    int getIntervalWeatherDataDownload() const { return intervalWeatherDataDownload; }
    const std::map<std::string, CountryData>& getCountries() const { return countries; }

    //this call will perform an insertion if countryCode key is not found!
    const std::map<std::string, LocationData>& getLocations(const std::string& countryCode) { return locations[countryCode]; }

    static const utility::string_t OPC_UA_SERVER;
    static const utility::string_t API_OPENAQ;
    static const utility::string_t API_DARKSKY;
    static const utility::string_t PARAM_NAME_API_DARKSKY_API_KEY;
    static const utility::string_t PARAM_NAME_API_DARKSKY_UNITS;
    static const utility::string_t PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA;

    int port_number;
    std::string endpointUrl;
    std::string hostName;

  private:

    /*
    This function supposed to be called from constructor only.
    If there is a problem opening file, parsing it or Dark Sky API key seems to be invalid - set the flag to terminate the program.
    */
    void processSettingsFile(const std::string& settingsFilePath);

    /*
    This function supposed to be called from processSettingsFile only.
    Check if the values related to the Dark Sky API from settings.json file are valid before assigning them to respective variables.
    */
    bool validateValuesFromDarkSky(web::json::value& jsonObj);

    utility::string_t keyApiDarksky;
    utility::string_t units;
    int intervalWeatherDataDownload;
    bool settingsAreValid = false;

    //Countries and locations that were passed through settings file.
    std::map<std::string, CountryData> countries;
    std::map<std::string, std::map<std::string, LocationData>> locations;
  };
}
