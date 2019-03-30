#pragma once
#include "cpprest/http_client.h"
#include "LocationData.h"
#include <string>
#include <vector>

namespace weathersvr {
	/* 
	A CountryData Class represents a JSON object returned from the Open AQ Platform API.
	Examples of json objects represeting countries from the API:
	{"name":"Andorra","code":"AD","cities":2,"locations":3,"count":60285},
	{"name":"Argentina","code":"AR","cities":1,"locations":4,"count":14976}
	*/
	class CountryData {
	public:
		CountryData(std::string name, std::string code, uint32_t cities, uint32_t locations, bool isInitialized = false);
		/* 
		This constructor version is used for initialize temporary CountryData objects to look for them inside a vector of CountryData objects.
		Example: when using find function from algorithm library.
		*/
		CountryData(std::string code);

		/* 
		Gets a new JSON value AS AN OBJECT from the cpprestsdk returned from the API request and
		parse it to a new CountryData object.
		@param web::json::value& json - A JSON value as an object returned from the API request.
		@return CountryData - A new CountryData object parsed from the JSON object returned from the API. 
		*/
		static CountryData parseJson(web::json::value& json);

		/*
		Gets a new JSON value AS AN ARRAY from the cpprestsdk returned from the API request and
		parse every JSON object inside the array to a new CountryData object, adding it to a vector.
		@param web::json::value& jsonArray - A JSON value as an array returned from the API request.
		@return std::vector<CountryData> - A new vector of CountryData objects parsed from the 
		JSON array returned from the API.
		*/
		static std::vector<CountryData> parseJsonArray(web::json::value& jsonArray);
		
		/* Indetifies that this object node was added to the OPC UA address space. */
		void setIsInitialized(const bool initialized);
		void setLocations(const std::vector<LocationData>& allLocations);

		std::string getName() const { return name; }
		std::string getCode() const { return code; }
		uint32_t getCitiesNumber() const { return citiesNumber; }
		uint32_t getLocationsNumber() const { return locationsNumber; }
		bool getIsInitialized() const { return isInitialized; }
		std::vector<LocationData>& getLocations() { return locations; }

		bool operator<(const CountryData& rhs) const;
		bool operator==(const CountryData& rhs) const;
		bool operator!=(const CountryData& rhs) const;

		// Constants Representing the string(key) of the pair string:value of JSON objects.
		static const utility::string_t KEY_NAME; // Key country's name in the JSON result.
		static const utility::string_t KEY_CODE; // Key country's code in the JSON result.
		static const utility::string_t KEY_CITIES; // Key country's city in the JSON result.
		static const utility::string_t KEY_LOCATIONS; // Key country's location in the JSON result.

		// C-Style String representing the browse/display name of the variables nodes in OPC UA.
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
		std::vector<LocationData> locations {};
	};
}