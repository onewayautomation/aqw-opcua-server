#include "LocationData.h"

namespace weatherserver {

    const utility::string_t LocationData::KEY_NAME = U("location");
    const utility::string_t LocationData::KEY_CITY_NAME = U("city");
    const utility::string_t LocationData::KEY_COUNTRY_CODE = U("country");
    const utility::string_t LocationData::KEY_COORDINATES = U("coordinates");
    const utility::string_t LocationData::KEY_LATITUDE = U("latitude");
    const utility::string_t LocationData::KEY_LONGITUDE = U("longitude");
    const short LocationData::INVALID_LATITUDE = 91;
    const short LocationData::INVALID_LONGITUDE = 181;

    char LocationData::BROWSE_FLAG_INITIALIZE[] = "FlagInitialize";

    LocationData::LocationData(std::string name, std::string city, std::string countryCode, double latitude, double longitude,
        bool hasBeenReceivedWeatherData, bool isInitialized, bool isAddingWeatherToAddressSpace)
        : name { name },
          city { city },
          countryCode { countryCode },
          latitude { latitude },
          longitude { longitude },
          hasBeenReceivedWeatherData { hasBeenReceivedWeatherData },
          isInitialized { isInitialized },
          isAddingWeatherToAddressSpace { isAddingWeatherToAddressSpace }
    {
        readLastTime = std::chrono::system_clock::now();
    }

	LocationData::LocationData()
    : isInitialized(false),
      hasBeenReceivedWeatherData(false),
      isAddingWeatherToAddressSpace(false),
      latitude(INVALID_LATITUDE),
      longitude(INVALID_LONGITUDE) {}

	LocationData LocationData::parseJson(web::json::value& json) {

        std::string lName;
        std::string lCity;
        std::string lCountryCode;
        double lLatitude = INVALID_LATITUDE;
        double lLongitude = INVALID_LONGITUDE;

        try {
            // Converts from wstring to string
            lName = utility::conversions::to_utf8string(json.at(KEY_NAME).as_string());
            lCity = utility::conversions::to_utf8string(json.at(KEY_CITY_NAME).as_string());
            lCountryCode = utility::conversions::to_utf8string(json.at(KEY_COUNTRY_CODE).as_string());

            auto coordinates = json.at(KEY_COORDINATES);
            lLatitude = coordinates.at(KEY_LATITUDE).as_double();
            lLongitude = coordinates.at(KEY_LONGITUDE).as_double();
        }
        catch (const web::json::json_exception& ex) {
            /* Some locations does not provide coordinators. For example Australia and Brazil
            have locations with no coordinates object in the Json array. In this case we use an
            invalid latitude and longitude represented by the constants when a exception is caught. */
          std::cout << "Exception caught while parsing JSON file with locations data: " << ex.what() << std::endl
            << "country code = " << lCountryCode << std::endl
            << "location = " << lName << std::endl
            << "city = " << lCity << std::endl
            << "longitude = " << lLongitude << std::endl
            << "latitude = " << lLatitude << std::endl;
        }
        return LocationData(lName, lCity, lCountryCode, lLatitude, lLongitude);
    }

    std::map<std::string, LocationData> LocationData::parseJsonArray(web::json::value& jsonArray) {
        std::map<std::string, LocationData> allLocations;
        if (jsonArray.is_array()) {
          for (size_t i{ 0 }; i < jsonArray.size(); i++) {
            auto location = jsonArray[i];
            LocationData locationData = LocationData::parseJson(location);

            //Only add the location to the vector if has valid coordinates.
            if (locationData.getLatitude() != LocationData::INVALID_LATITUDE
              && locationData.getLongitude() != LocationData::INVALID_LONGITUDE)
              allLocations[locationData.name] = locationData;
            else {
              std::cout << "Invalid coordinates - skipped one entry." << std::endl;
            }
          }
        }
        return allLocations;
    }

    void LocationData::setHasBeenReceivedWeatherData(const bool received) {
        hasBeenReceivedWeatherData = received;
    }

    void LocationData::setIsInitialized(const bool initialized) {
        isInitialized = initialized;
    }

    void LocationData::setIsAddingWeatherToAddressSpace(const bool addingWeatherToAddressSpace) {
        isAddingWeatherToAddressSpace = addingWeatherToAddressSpace;
    }

    void LocationData::setWeatherData(const WeatherData weather) {
        weatherData = weather;
    }

    void LocationData::setReadLastTime(const std::chrono::system_clock::time_point time) {
        readLastTime = time;
    }
}
