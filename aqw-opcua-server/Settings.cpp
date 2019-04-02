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
		auto darkSkyObj = jsonFile.at(API_DARKSKY);

		// Set the values from the Json file's DarkSky object.
		keyApiDarksky = darkSkyObj.at(PARAM_NAME_API_DARKSKY_API_KEY).as_string();
		units = darkSkyObj.at(PARAM_NAME_API_DARKSKY_UNITS).as_string();
		intervalDownloadWeatherData = static_cast<short>(darkSkyObj.at(PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA).as_integer());

		std::cout << "Build completed successfully!!!" << std::endl;
	} catch (const web::json::json_exception& e) {
		std::cerr << "Error parsing the settings json file: " << e.what() << std::endl;
		std::cerr << "Default values will be used (Except for the DarkSky API_KEY)" << std::endl;
	}
	std::cout << "################################################" << std::endl << std::endl;
}

void weathersvr::Settings::setDefaultValues() {
	keyApiDarksky = U("");
	units = U("si");
	intervalDownloadWeatherData = 10;
}
