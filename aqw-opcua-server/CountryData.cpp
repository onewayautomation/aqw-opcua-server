#include "CountryData.h"

const utility::string_t weathersvr::CountryData::KEY_NAME = U("name");
const utility::string_t weathersvr::CountryData::KEY_CODE = U("code");
const utility::string_t weathersvr::CountryData::KEY_CITIES = U("cities");
const utility::string_t weathersvr::CountryData::KEY_LOCATIONS = U("locations");

char weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID[] = "Countries";
char weathersvr::CountryData::BROWSE_NAME[] = "CountryName";
char weathersvr::CountryData::BROWSE_CODE[] = "CountryCode";
char weathersvr::CountryData::BROWSE_CITIES_NUMBER[] = "CountryCitiesNumber";
char weathersvr::CountryData::BROWSE_LOCATIONS_NUMBER[] = "CountryLocationsNumber";

weathersvr::CountryData::CountryData(std::string name, std::string code, uint32_t cities, uint32_t locations, bool isInitialized)
	: name {name}, code {code}, citiesNumber {cities}, locationsNumber {locations}, isInitialized {isInitialized}
{}

weathersvr::CountryData::CountryData(std::string code)
	: CountryData {"", code, 0, 0}
{}

weathersvr::CountryData weathersvr::CountryData::parseJson(web::json::value& json) {
	return CountryData(
		utility::conversions::to_utf8string(json.at(KEY_NAME).as_string()), // Converts from wstring to string
		utility::conversions::to_utf8string(json.at(KEY_CODE).as_string()),
		json.at(KEY_CITIES).as_integer(),
		json.at(KEY_LOCATIONS).as_integer());;
}

std::vector<weathersvr::CountryData> weathersvr::CountryData::parseJsonArray(web::json::value& jsonArray) {
	std::vector<CountryData> vectorAllCountries;
	if (jsonArray.is_array()) {
		for (size_t i {0}; i < jsonArray.size(); i++) {
			auto country = jsonArray[i];
			CountryData countryData = CountryData::parseJson(country);
			vectorAllCountries.push_back(countryData);
		}
	}

	return vectorAllCountries;
}

void weathersvr::CountryData::setIsInitialized(const bool initialized) {
	isInitialized = initialized;
}

void weathersvr::CountryData::setLocations(const std::vector<LocationData>& allLocations) {
	locations = allLocations;
}

bool weathersvr::CountryData::operator<(const CountryData& rhs) const {
	return this->code < rhs.code;
}

bool weathersvr::CountryData::operator==(const CountryData& rhs) const {
	return this->code == rhs.code;
}

bool weathersvr::CountryData::operator!=(const CountryData& rhs) const {
	return !(*this == rhs);
}


