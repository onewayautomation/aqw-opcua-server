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
        : name{ name }, code{ code }, citiesNumber{ cities }, locationsNumber{ locations }, isInitialized{ isInitialized }
    {}

    CountryData::CountryData(std::string code)
        : CountryData{ "", code, 0, 0 }
    {}


	CountryData::CountryData(): citiesNumber(0), locationsNumber(0), isInitialized(false)
	{
	}

	CountryData CountryData::parseJson(web::json::value& json) {

        std::string name;
        std::string code;
        // added default values so there are no unitialized variable errors when parsing fails
        uint32_t cities = 0;
        uint32_t locations = 0;

        try
        {
            name = utility::conversions::to_utf8string(json.at(KEY_NAME).as_string());
            code = utility::conversions::to_utf8string(json.at(KEY_CODE).as_string());
            cities = json.at(KEY_CITIES).as_integer();
            locations = json.at(KEY_LOCATIONS).as_integer();
        }
        catch (std::exception& ex) //TODO - catch more specific type of exception
        {
            std::cout << "Exception caught while parsing JSON file with countries data: " << ex.what() << std::endl
                << "name (default - empty) = " << name << std::endl
                << "code (default - empty) = " << code << std::endl
                << "cities (default - 0) = " << cities << std::endl
                << "locations (default - 0) = " << locations << std::endl;
        }
        return CountryData(name, code, cities, locations);
    }

    std::map<std::string, CountryData> CountryData::parseJsonArray(web::json::value& jsonArray) {
		std::map<std::string, CountryData> allCountries;
        try
        {
            if (jsonArray.is_array()) {
                for (size_t i{ 0 }; i < jsonArray.size(); i++) {
                    auto country = jsonArray[i];
                    CountryData countryData = CountryData::parseJson(country);
                    if (!countryData.name.empty() && !countryData.code.empty())
                    {
						allCountries[countryData.code] = countryData;
                    }
                    else
                    {
                        std::cout << "Country name or code is empty - skipped one entry." << std::endl;
                    }
                }
            }
        }
        catch (std::exception& ex) //TODO - catch more specific type of exception
        {
			// TODO: Log exception message
            throw ex;
        }

        std::cout << "Retrieved information for " << allCountries.size() << " countries" << std::endl;
        return std::move(allCountries);
    }

    void CountryData::setIsInitialized(const bool initialized) {
        isInitialized = initialized;
    }

    void CountryData::setLocations(const std::map<std::string, LocationData>& allLocations) {
        locations = allLocations;
    }

    bool CountryData::operator<(const CountryData& rhs) const {
        return this->code < rhs.code;
    }

    bool CountryData::operator==(const CountryData& rhs) const {
        return this->code == rhs.code;
    }

    bool CountryData::operator!=(const CountryData& rhs) const {
        return !(*this == rhs);
    }
}
