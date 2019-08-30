#pragma once
#include "cpprest/http_client.h"
#include "open62541.h"
#include "Settings.h"
#include "CountryData.h"
#include "LocationData.h"
#include "WeatherData.h"

namespace weathersvr {
	class WebService {
	public:

		// ########## Methos to fetch data from the APIs services.
		/*
		Makes a http requests to an api web service to fetch all the countries.
		@return web::json::value - A JSON value as an array containing all the countries objects.
		Check the CountryData Class to see the JSON representation.
		*/
		pplx::task<web::json::value> fetchAllCountries();

		/*
		Makes a http requests to an api web service to fetch all the locations of the country specified by the countryCode formal parameter.
		@param const std::string& countryCode - The 2 letters ISO code that represents the country name.
		@param const uint32_t limit - The number of locations to be returned. Default = 100 and Max = 10000.
		@return web::json::value - A JSON value as an array containing all the locations objects.
		Check the LocationData Class to see the JSON representation.
		*/
		pplx::task<web::json::value> fetchAllLocations(const std::string& countryCode, const uint32_t limit = 100);

		/*
		Makes a http requests to an api web service to fetch the weather data from a specific location specified by latitude and longitude.
		@param const double& latitude - The location's latitude that weather data will be requested.
		@param const double& longitude - The location's longitude that weather data will be requested.
		@return web::json::value - A JSON value as an object containing weather data.
		Check the WeatherData Class to see the JSON representation.
		*/
		pplx::task<web::json::value> fetchWeather(const double& latitude, const double& longitude);
		
		void setServer(UA_Server* uaServer);
		void setSettings(const Settings& settingsObj);
		void setAllCountries(const std::vector<CountryData>& allCountries);

		UA_Server* getServer() { return server; }
		const Settings& getSettings() { return settings; }
		std::vector<CountryData>& getAllCountries() { return fetchedAllCountries; }
	
		// Constants endpoints, keys, paths, querys etc to all the API services.
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

		/* DarkSky URI example:
		https://api.darksky.net/forecast/KEY/latitude,longitude?exclude=minutely,hourly,daily?units=si 
		With the URI above, the request returns the current weather conditions, excluding a 
		minute-by-minute forecast for the next hour (where available), an hour-by-hour forecast for the 
		next 48 hours, and a day-by-day forecast for the next week.
		The weather information are also returned in the SI units.
		*/

	private:
		UA_Server* server {nullptr};
		Settings settings;
		std::vector<CountryData> fetchedAllCountries {};
	};
}
