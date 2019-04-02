#pragma once
#include "cpprest/http_client.h"
#include <fstream>
#include <iostream>

namespace weathersvr {
	class Settings {
	public:
		Settings();

		void setup(char* fileName);
		void setDefaultValues();

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
		utility::string_t keyApiDarksky;
		utility::string_t units;
		short intervalDownloadWeatherData;
	};
}
