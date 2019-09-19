#include "Settings.h"

namespace weatherserver {

    const utility::string_t OPC_UA_SERVER = U("opc_ua_server");
    const utility::string_t API_OPENAQ = U("openaq_api");
    const utility::string_t API_DARKSKY = U("darksky_api");
    const utility::string_t PARAM_NAME_API_DARKSKY_API_KEY = U("api_key");
    const utility::string_t PARAM_NAME_API_DARKSKY_UNITS = U("param_units");
    const utility::string_t PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA = U("interval_download");

    const std::string SETTINGS_FILE_NAME = "settings.json";

    Settings::Settings(std::string& currentDir) {
        //assign default values
        keyApiDarksky = U("");
        units = U("si");
        intervalWeatherDataDownload = 10;
        port_number = 48484;
        endpointUrl = "opc.tcp://localhost:48484";
        hostName = "localhost";

        settingsAreValid = false; //empty DarkSkyAPI = invalid

        processSettingsFile(currentDir);
    }

    void Settings::processSettingsFile(std::string& currentDir) {
        try {
            std::fstream inputFile{ currentDir.append(SETTINGS_FILE_NAME) };

            std::cout << "################################################" << std::endl;

            if (!inputFile)
            {
                std::cerr << "Could not open the settings file: " << SETTINGS_FILE_NAME << std::endl;
                std::cerr << "Check if the file was copied correctly at build stage." << std::endl;
                std::cout << "################################################" << std::endl << std::endl;
                return;
            }

            std::cout << "File opened successfully. Building settings..." << std::endl;

            auto jsonFile = web::json::value::parse(inputFile);
            if (!validateValuesFromDarkSky(jsonFile.at(API_DARKSKY))) {
                std::cerr << "Invalid Dark Sky API key. Terminating..." << std::endl;
                std::cout << "################################################" << std::endl << std::endl;
                return false;
            }

            this->port_number = jsonFile.at(U("opc_ua_server")).at(U("port-number")).as_integer();
            this->endpointUrl = utility::conversions::to_utf8string(jsonFile.at(U("opc_ua_server")).at(U("endpoint-url")).as_string());
            this->hostName = utility::conversions::to_utf8string(jsonFile.at(U("opc_ua_server")).at(U("host-name")).as_string());

            std::cout << "Build completed successfully!!!" << std::endl;
        }
        catch (const web::json::json_exception& e) {
            std::cerr << "Error parsing the settings json file: " << e.what() << std::endl;
            return false;
        }

        std::cout << "Weather data units: " << utility::conversions::to_utf8string(units) << std::endl;
        std::cout << "Interval in minutes for automatic update of weather data: " << intervalWeatherDataDownload << std::endl;

        std::cout << "################################################" << std::endl << std::endl;

        return true;
    }

    bool Settings::validateValuesFromDarkSky(web::json::value& jsonObj) {
        //Set the values from the Json file's DarkSky object.
        keyApiDarksky = jsonObj.at(PARAM_NAME_API_DARKSKY_API_KEY).as_string();

        //Simple checks: empty or with spaces - invalid key for sure.
        if (keyApiDarksky.empty()) {
            std::cout << "Empty Dark Sky API key. No weather data will be available." << std::endl;
            return false;
        }
        else {
            for (int i = 0; i < keyApiDarksky.length(); i++) {
                if (iswspace(keyApiDarksky[i])) {
                    std::cout << "Dark Sky API key has spaces - invalid entry. No weather data will be available." << std::endl;
                    return false;
                }
            }
        }

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
        int tempInterval = static_cast<short>(jsonObj.at(PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA).as_integer());
        if (tempInterval >= 1 && tempInterval <= 60)
            intervalWeatherDataDownload = tempInterval;

        return true;
    }
}