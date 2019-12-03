#include "CountryData.h"

namespace weatherserver {

  const utility::string_t CountryData::KEY_NAME = U("name");
  const utility::string_t CountryData::KEY_CODE = U("code");
  const utility::string_t CountryData::KEY_CITIES = U("cities");
  const utility::string_t CountryData::KEY_LOCATIONS = U("locations");

  char CountryData::COUNTRIES_FOLDER_NODE_ID[] = "Countries";
  char CountryData::BROWSE_NAME[] = "CountryName";
  char CountryData::BROWSE_CODE[] = "CountryCode";
  char CountryData::BROWSE_CITIES_NUMBER[] = "CountryCitiesNumber";
  char CountryData::BROWSE_LOCATIONS_NUMBER[] = "CountryLocationsNumber";

  CountryData::CountryData(std::string name, std::string code, uint32_t cities, uint32_t locations, bool isInitialized)
    : name { name },
      code { code },
      citiesNumber { cities },
      locationsNumber { locations },
      isInitialized { isInitialized } {}

  CountryData::CountryData()
    : citiesNumber(0),
      locationsNumber(0),
      isInitialized(false) {}

  CountryData CountryData::parseJson(web::json::value& json) {

    std::string name;
    std::string code;
    uint32_t cities = 0;
    uint32_t locations = 0;

    try {
      if (json.has_field(KEY_NAME))
        name = utility::conversions::to_utf8string(json.at(KEY_NAME).as_string());
      if (json.has_field(KEY_CODE))
        code = utility::conversions::to_utf8string(json.at(KEY_CODE).as_string());
      if (json.has_field(KEY_CITIES))
        cities = json.at(KEY_CITIES).as_integer();
      if (json.has_field(KEY_LOCATIONS))
        locations = json.at(KEY_LOCATIONS).as_integer();
      if (name.empty())
        name = code;
    }
    catch (const web::json::json_exception & ex) {
      auto stringValue = utility::conversions::to_utf8string(json.serialize());
      std::cout << "Exception caught while parsing JSON object with country data: " << ex.what() << std::endl
        << "name = " << name << std::endl
        << "code = " << code << std::endl
        << "cities = " << cities << std::endl
        << "locations = " << locations << std::endl;
      std::cout << "Json value as text: " << stringValue << std::endl;
    }

    return CountryData(name, code, cities, locations);
  }

  std::map<std::string, CountryData> CountryData::parseJsonArray(web::json::value& jsonArray) {

    std::map<std::string, CountryData> allCountries;

    if (jsonArray.is_array()) {
      for (size_t i{ 0 }; i < jsonArray.size(); i++) {
        auto country = jsonArray[i];
        CountryData countryData = CountryData::parseJson(country);
        if (!countryData.name.empty() && !countryData.code.empty()) {
          allCountries[countryData.code] = countryData;
        }
        else {
          std::cout << "Country name or code is empty - skipped one entry." << std::endl;
        }
      }
    }

    std::cout << "Retrieved information for " << allCountries.size() << " countries" << std::endl;

    return allCountries;
  }

  void CountryData::setLocationsNumber(const uint32_t newLocationsNumber) {
    locationsNumber = newLocationsNumber;
  }

  void CountryData::setIsInitialized(const bool initialized) {
    isInitialized = initialized;
  }

  void CountryData::setLocations(const std::map<std::string, LocationData>& allLocations) {
    locations = allLocations;
  }
}
