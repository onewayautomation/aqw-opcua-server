#pragma once

#include <cpprest/http_client.h>

#include "open62541.h"

#include "Settings.h"
#include "CountryData.h"
#include "LocationData.h"
#include "WeatherData.h"

namespace weatherserver {

  class WebService {

  public:

    WebService(const Settings& settingsObj);

    // ########## Methods to fetch data from the APIs services.

    /*
    Makes http requests to Open AQ API to fetch country data.

    @return task for JSON array value containing country data objects.

    Check the CountryData class to see the JSON representation.
    */
    pplx::task<web::json::value> fetchAllCountries();

    /*
    Makes http requests to Open AQ API to fetch locations data for the country specified by the "countryCode" parameter.

    @param countryCode - two letter ISO code that represents the country.
    @param limit - number of locations to be returned. Default = 100 and max = 10000.
    @return task for JSON array value containing all location data objects.

    Check the LocationData class to see the JSON representation.
    */
    pplx::task<web::json::value> fetchAllLocations(const std::string& countryCode, const uint32_t limit = 100);

    /*
    Makes http requests to Dark Sky API to fetch weather data for a specific location specified by latitude and longitude.

    @return task for JSON object value containing weather data.

    Check the WeatherData class to see the JSON representation.
    */
    pplx::task<web::json::value> fetchWeather(const double latitude, const double longitude);

    void setServer(UA_Server* uaServer);
    void setAllCountries(const std::map<std::string, CountryData>& allCountries);

    UA_Server* getServer() { return server; }
    const Settings& getSettings() { return settings; }
    std::map<std::string, CountryData>& getAllCountries() { return fetchedAllCountries; }

    //String constants for API services: endpoints, keys, paths, queries etc.
    static const uint16_t OPC_NS_INDEX;
    static const utility::string_t ENDPOINT_API_OPENAQ;
    static const utility::string_t PATH_API_OPENAQ_COUNTRIES;
    static const utility::string_t PATH_API_OPENAQ_LOCATIONS;
    static const utility::string_t PATH_API_OPENAQ_MEASUREMENTS;
    static const utility::string_t PARAM_API_OPENAQ_COUNTRY;
    static const utility::string_t PARAM_API_OPENAQ_LIMIT;

    static const utility::string_t ENDPOINT_API_DARKSKY;
    static const utility::string_t PARAM_API_DARKSKY_EXCLUDE;
    static const utility::string_t PARAM_API_DARKSKY_UNITS;
    static const std::string PARAM_VALUE_API_DARKSKY_MINUTELY;
    static const std::string PARAM_VALUE_API_DARKSKY_HOURLY;
    static const std::string PARAM_VALUE_API_DARKSKY_DAILY;

    /*
    DarkSky URI example: https://api.darksky.net/forecast/KEY/latitude,longitude?exclude=minutely,hourly,daily?units=si

    With the URI above, the request returns current weather conditions, excluding
    a minute-by-minute forecast for the next hour (where available),
    an hour-by-hour forecast for the next 48 hours,
    and a day-by-day forecast for the next week.

    And this information is in SI units.
    */

  private:

    UA_Server* server{ nullptr };
    Settings settings;
    std::map<std::string, CountryData> fetchedAllCountries;
  };
}
