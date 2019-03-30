#include "LocationData.h"

using namespace weathersvr;

const utility::string_t LocationData::KEY_NAME = U("location");
const utility::string_t LocationData::KEY_CITY_NAME = U("city");
const utility::string_t LocationData::KEY_COUNTRY_CODE = U("country");
const utility::string_t LocationData::KEY_COORDINATES = U("coordinates");
const utility::string_t LocationData::KEY_LATITUDE = U("latitude");
const utility::string_t LocationData::KEY_LONGITUDE = U("longitude");
const short LocationData::INVALID_LATITUDE = 91;
const short LocationData::INVALID_LONGITUDE = 181;

char LocationData::BROWSE_FLAG_INITIALIZE[] = "FlagInitialize";

weathersvr::LocationData::LocationData(std::string name, std::string city, std::string countryCode,	double latitude, double longitude, 
	bool hasBeenReceivedWeatherData, bool isInitialized, bool isAddingWeatherToAddressSpace)
	: name {name}, city {city}, countryCode {countryCode}, latitude {latitude}, longitude {longitude}, 
	hasBeenReceivedWeatherData {hasBeenReceivedWeatherData}, isInitialized {isInitialized}, isAddingWeatherToAddressSpace {isAddingWeatherToAddressSpace}
{
	readLastTime = std::chrono::system_clock::now();
}

weathersvr::LocationData::LocationData(std::string name, std::string countryCode)
	: LocationData {name, "", countryCode, INVALID_LATITUDE, INVALID_LONGITUDE}
{}

LocationData LocationData::parseJson(web::json::value& json) {
	// Converts from wstring to string
	std::string lName = utility::conversions::to_utf8string(json.at(KEY_NAME).as_string());
	std::string lCity = utility::conversions::to_utf8string(json.at(KEY_CITY_NAME).as_string());
	std::string lCountryCode = utility::conversions::to_utf8string(json.at(KEY_COUNTRY_CODE).as_string());

	double lLatitude = INVALID_LATITUDE;
	double lLongittude = INVALID_LONGITUDE;
	try {
		auto coordinates = json.at(KEY_COORDINATES);
		lLatitude = coordinates.at(KEY_LATITUDE).as_double();
		lLongittude = coordinates.at(KEY_LONGITUDE).as_double();
	} catch (const web::json::json_exception&) {
		/* Some locations does not provide coordinators. For example Australia and Brazil
		have locations with no coordinates object in the Json array. In this case we use an
		invalid latitude and longitude represented by the constants when a exception is caught. */
	}
	return LocationData(lName, lCity, lCountryCode, lLatitude, lLongittude);
}

std::vector<LocationData> LocationData::parseJsonArray(web::json::value& jsonArray) {
	std::vector<LocationData> vectorAllLocations;
	if (jsonArray.is_array()) {
		for (size_t i {0}; i < jsonArray.size(); i++) {
			auto location = jsonArray[i];
			LocationData locationData = LocationData::parseJson(location);

			//Only add the location to the vector if has valid coordinates.
			if (locationData.getLatitude() != weathersvr::LocationData::INVALID_LATITUDE
				&& locationData.getLongitude() != weathersvr::LocationData::INVALID_LONGITUDE)
				vectorAllLocations.push_back(locationData);
		}
	}

	return vectorAllLocations;
}

void weathersvr::LocationData::setHasBeenReceivedWeatherData(const bool received) {
	hasBeenReceivedWeatherData = received;
}

void weathersvr::LocationData::setIsInitialized(const bool initialized) {
	isInitialized = initialized;
}

void weathersvr::LocationData::setIsAddingWeatherToAddressSpace(const bool addingWeatherToAddressSpace) {
	isAddingWeatherToAddressSpace = addingWeatherToAddressSpace;
}

void weathersvr::LocationData::setWeatherData(const WeatherData weather) {
	weatherData = weather;
}

void weathersvr::LocationData::setReadLastTime(const std::chrono::system_clock::time_point time) {
	readLastTime = time;
}

bool weathersvr::LocationData::operator<(const LocationData& rhs) const {
	return ((this->name < rhs.name) && (this->countryCode < rhs.countryCode));
}

bool weathersvr::LocationData::operator==(const LocationData& rhs) const {
	return (this->name == rhs.name && this->countryCode == rhs.countryCode);
}

bool weathersvr::LocationData::operator!=(const LocationData & rhs) const {
	return !(*this == rhs);
}
