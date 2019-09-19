#pragma once

#include <fstream>
#include <iostream>

#include <cpprest/http_client.h>

namespace weatherserver {

    class Settings {

        public:

            Settings();

            //Open the SETTINGS_FILE_NAME file and assign values to variables.
            bool setup();

            const utility::string_t& getKeyApiDarksky() const { return keyApiDarksky; }
            const utility::string_t& getUnits() const { return units; }
            const int getIntervalWeatherDataDownload() const { return intervalWeatherDataDownload; }

            static const utility::string_t OPC_UA_SERVER;
            static const utility::string_t API_OPENAQ;
            static const utility::string_t API_DARKSKY;
            static const utility::string_t PARAM_NAME_API_DARKSKY_API_KEY;
            static const utility::string_t PARAM_NAME_API_DARKSKY_UNITS;
            static const utility::string_t PARAM_NAME_API_DARKSKY_INTERVAL_DOWNLOAD_WEATHER_DATA;

        private:

            /*
            Check if the values present in the settings.json file related to the dark sky api are valid before set them to respective variables. If is not valid, the default values will be kept.
            This function may throw an exception web::json::json_exception.
            @param web::json::value& jsonObj - The Json OBJECT which values will be parsed and validated.
            */
            bool validateValuesFromDarkSky(web::json::value& jsonObj);

            utility::string_t keyApiDarksky;
            utility::string_t units;
            int intervalWeatherDataDownload;

            //was public, changed to private
            int port_number;
            std::string endpointUrl;
            std::string hostName;
    };
}
