#pragma once
#include "cpprest/http_client.h"
#include <fstream>
#include <iostream>

namespace weathersvr {
	class Settings {
	public:
		Settings();
		/*
		Open the file passed to the formal parameter fileName and set the settings to variables of this class.
		@param char* fileName - the path containing the settings file name and extension that is passed to command line arguments.
		*/
		void setup(char* fileName);

		const utility::string_t& getKeyApiDarksky() const { return keyApiDarksky; }
		const utility::string_t& getUnits() const { return units; }
		const short getIntervalDownloadWeatherData() const { return intervalDownloadWeatherData; }

		static const utility::string_t OPC_UA_SERVER;
		static const utility::string_t API_OPENAQ;
		static const utility::string_t API_DARKSKY;
		static const utility::string_t PARAM_NAME_API_DARKSKY_API_KEY;
		static const utility::string_t PARAM_NAME_API_DARKSKY_UNITS;
		static const utility::string_t PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA;
	private:
		void setDefaultValues();
		/*
		Check if the values present in the settings.json file related to the dark sky api are valid before set them to respective variables. If is not valid, the default values will be kept.
		This function may throw an exception web::json::json_exception.
		@param web::json::value& jsonObj - The Json OBJECT which the values will be parsed and validated.
		*/
		void validateValuesFromDarkSky(web::json::value& jsonObj);

		utility::string_t keyApiDarksky;
		utility::string_t units;
		short intervalDownloadWeatherData;
	};
}
