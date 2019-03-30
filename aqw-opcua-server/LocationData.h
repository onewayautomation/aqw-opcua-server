#pragma once
#include "cpprest/http_client.h"
#include "WeatherData.h"
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

namespace weathersvr {
	/* A LocationData Class represents a JSON object returned from the Open AQ Platform API.
	* Examples of json objects represeting cities from the API:
	{"location":"ALGOMA","city":"ONTARIO","country":"CA","count":9290,"sourceNames"["AirNow"],
	"lastUpdated":"2018-08-01T15:00:00.000Z","firstUpdated":"2016-03-06T19:00:00.000Z","parameters"["o3"],
	"distance":5907356,"sourceName":"AirNow","coordinates":{"latitude":47.035,"longitude":-84.3811}},

	{"location":"AYLESFORD MOUNTAIN","city":"NOVA SCOTIA","country":"CA","count":42184,"sourceNames":"AirNow"],
	"lastUpdated":"2019-03-16T16:00:00.000Z","firstUpdated":"2016-03-06T19:00:00.000Z","parameters"["no2","o3","pm25"],
	"distance":4765644,"sourceName":"AirNow","coordinates":{"latitude":45.0061,"longitude":-65}}
	*/
	class LocationData {
	public:
		LocationData(std::string name, std::string city, std::string countryCode, double latitude, double longitude, 
			bool hasBeenReceivedWeatherData = false, bool isInitialized = false, bool isAddingWeatherToAddressSpace = false);
		/*
		This constructor version is used for initialize temporary LocationData objects to look for them inside a vector of LocationData objects.
		Example: when using find function from algorithm library.
		*/
		LocationData(std::string name, std::string countryCode);

		/*
		Gets a new JSON value AS AN OBJECT from the cpprestsdk returned from the API request and
		parse it to a new LocationData object.
		@param web::json::value& json - A JSON value as an object returned from the API request.
		@return LocationData - A new LocationData object parsed from the JSON object returned from the API.
		*/
		static LocationData parseJson(web::json::value& json);

		/*
		Gets a new JSON value AS AN ARRAY from the cpprestsdk returned from the API request and
		parse every JSON object inside the array to a new LocationData object, adding it to a vector.
		@param web::json::value& jsonArray - A JSON value as an array returned from the API request.
		@return std::vector<LocationData> - A new vector of LocationData objects parsed from the 
		JSON array returned from the API.
		*/
		static std::vector<LocationData> parseJsonArray(web::json::value& jsonArray);
		
		void setHasBeenReceivedWeatherData(const bool received);
		void setIsInitialized(const bool initialized);
		void setIsAddingWeatherToAddressSpace(const bool addingWeatherToAddressSpace);
		void setWeatherData(const WeatherData weather);
		void setReadLastTime(const std::chrono::system_clock::time_point time);

		std::string getName() const { return name; }
		std::string getCity() const { return city; }
		std::string getCountryCode() const { return countryCode; }
		double getLatitude() const { return latitude; }
		double getLongitude() const { return longitude; }
		/* Indetifies that this object node has received weather data information for the first time. Remember to set the weather data to this object with setWeatherData(const WeatherData weathe). */
		bool getHasBeenReceivedWeatherData() const { return hasBeenReceivedWeatherData; }
		/* Indetifies that this object node was added to the OPC UA address space. */
		bool getIsInitialized() const { return isInitialized; }
		/* Indetifies that the weather data variables nodes are currently being adding to the OPC UA address space. */
		bool getIsAddingWeatherToAddressSpace() const { return isAddingWeatherToAddressSpace; }
		WeatherData& getWeatherData() { return weatherData; }
		std::chrono::system_clock::time_point getReadLastTime() const { return readLastTime; }

		bool operator<(const LocationData& rhs) const;
		bool operator==(const LocationData& rhs) const;
		bool operator!=(const LocationData& rhs) const;

		// Constants Representing the string(key) of the pair string:value of JSON objects.
		static const utility::string_t KEY_NAME;
		static const utility::string_t KEY_CITY_NAME;
		static const utility::string_t KEY_COUNTRY_CODE;
		static const utility::string_t KEY_COORDINATES;
		static const utility::string_t KEY_LATITUDE;
		static const utility::string_t KEY_LONGITUDE;
		static const short INVALID_LATITUDE;
		static const short INVALID_LONGITUDE;

		// C-Style String representing the browse/display name of the variables nodes in OPC UA.
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
