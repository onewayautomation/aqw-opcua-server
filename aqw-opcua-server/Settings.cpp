#include "Settings.h"

const utility::string_t weathersvr::Settings::OPC_UA_SERVER = U("opc_ua_server");
const utility::string_t weathersvr::Settings::API_OPENAQ = U("openaq_api");
const utility::string_t weathersvr::Settings::API_DARKSKY = U("darksky_api");
const utility::string_t weathersvr::Settings::PARAM_NAME_API_DARKSKY_API_KEY = U("api_key");
const utility::string_t weathersvr::Settings::PARAM_NAME_API_DARKSKY_UNITS = U("param_units");
const utility::string_t weathersvr::Settings::PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA = U("interval_download");

weathersvr::Settings::Settings() {
	setDefaultValues();
}

void weathersvr::Settings::setup(char * fileName) {
	try {
		std::fstream inputFile {fileName};

		std::cout << "################################################" << std::endl;
		if (!inputFile) {
			std::cerr << "Could not open the settings file: " << fileName << std::endl;
			std::cerr << "Check if the file name and extension are correctly. Also check if the path to the file was passed correctly." << std::endl;
			std::cout << "################################################" << std::endl << std::endl;
			return;
		}
		std::cout << "Building settings..." << std::endl;

		auto jsonFile = web::json::value::parse(inputFile);
		validateValuesFromDarkSky(jsonFile.at(API_DARKSKY));

		std::cout << "Build completed successfully!!!" << std::endl;
	} catch (const web::json::json_exception& e) {
		std::cerr << "Error parsing the settings json file: " << e.what() << std::endl;
		std::cerr << "Default values will be used (Except for the DarkSky API_KEY)" << std::endl;
	}

	std::wcout << "Weather data units: " << units << std::endl;
	std::cout << "Interval in minutes for automatic update of weather data: " << intervalDownloadWeatherData << std::endl;

	std::cout << "################################################" << std::endl << std::endl;
}

void weathersvr::Settings::setDefaultValues() {
	keyApiDarksky = U("");
	units = U("si");
	intervalDownloadWeatherData = 10;
}

void weathersvr::Settings::validateValuesFromDarkSky(web::json::value & jsonObj) {
	// Set the values from the Json file's DarkSky object.

	keyApiDarksky = jsonObj.at(PARAM_NAME_API_DARKSKY_API_KEY).as_string();
	/* `units` - Return weather conditions in the requested units, should be one of the following:
	auto: automatically select units based on geographic location
	ca: same as si, except that windSpeed and windGust are in kilometers per hour
	uk2: same as si, except that nearestStormDistance and visibility are in miles, and windSpeed and windGust in miles per hour
	us: Imperial units (the default)
	si: SI units
	*/
	utility::string_t tempUnits = jsonObj.at(PARAM_NAME_API_DARKSKY_UNITS).as_string();
	if (tempUnits == U("auto") || tempUnits == U("ca") || tempUnits == U("uk2") || tempUnits == U("us") || tempUnits == U("si"))
		units = tempUnits;

	/* The interval for downloading allowed is from 1 to 60 minutes. */
	short tempInterval = static_cast<short>(jsonObj.at(PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA).as_integer());
	if (tempInterval >= 1 && tempInterval <= 60)
		intervalDownloadWeatherData = tempInterval;
}
