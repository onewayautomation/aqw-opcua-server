#include <signal.h>
#include <stdlib.h>

#include <fstream>
#include <thread>
#include <algorithm>

//amalgamated version of open62541
#include "open62541.h"

#include "Settings.h"
#include "WebService.h"



// TODO: Global Variables, Be aware of them.
// Service responsible for REST calls on the internet.
weatherserver::WebService* webService;

UA_Boolean running = true;

static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

namespace weatherserver {
    /*
    Update all the weather variables nodes of the location supplied by parameter from the web service.
    Map these objects returned from the web service in OPC UA objects and put them available in the address space.
    @param UA_DataValue *dataValue - The data value of the variable node that will be updated on OPC UA.
    @param const WeatherData& weatherData - The WeatherData object the data will be requested from.
    @param std::string weatherVariableName - The weather browse name to check which variable node to update.
    */
    static void updateWeatherVariables(UA_DataValue* dataValue, const WeatherData& weatherData, std::string weatherVariableName) {
        if (weatherVariableName == WeatherData::BROWSE_LATITUDE) {
            UA_Double latitudeValue = weatherData.getLatitude();
            UA_Variant_setScalarCopy(&(dataValue->value), &latitudeValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_LONGITUDE) {
            UA_Double longitudeValue = weatherData.getLongitude();
            UA_Variant_setScalarCopy(&dataValue->value, &longitudeValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_TIMEZONE) {
            UA_String timezoneValue = UA_STRING_ALLOC(weatherData.getTimezone().c_str());
            UA_Variant_setScalarCopy(&dataValue->value, &timezoneValue, &UA_TYPES[UA_TYPES_STRING]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_ICON) {
            UA_String iconValue = UA_STRING_ALLOC(weatherData.getCurrentlyIcon().c_str());
            UA_Variant_setScalarCopy(&dataValue->value, &iconValue, &UA_TYPES[UA_TYPES_STRING]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_TEMPERATURE) {
            UA_Double temperatureValue = weatherData.getCurrentlyTemperature();
            UA_Variant_setScalarCopy(&dataValue->value, &temperatureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_APPARENT_TEMPERATURE) {
            UA_Double apparentTemperatureValue = weatherData.getCurrentlyApparentTemperature();
            UA_Variant_setScalarCopy(&dataValue->value, &apparentTemperatureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_HUMIDITY) {
            UA_Double humidityValue = weatherData.getCurrentlyHumidity();
            UA_Variant_setScalarCopy(&dataValue->value, &humidityValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_PRESSURE) {
            UA_Double pressureValue = weatherData.getCurrentlyPressure();
            UA_Variant_setScalarCopy(&dataValue->value, &pressureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_WIND_SPEED) {
            UA_Double windSpeedValue = weatherData.getCurrentlyWindSpeed();
            UA_Variant_setScalarCopy(&dataValue->value, &windSpeedValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_WIND_BEARING) {
            UA_Double windBearingValue = weatherData.getCurrentlyWindBearing();
            UA_Variant_setScalarCopy(&dataValue->value, &windBearingValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
        else if (weatherVariableName == WeatherData::BROWSE_CLOUD_COVER) {
            UA_Double cloudCoverValue = weatherData.getCurrentlyCloudCover();
            UA_Variant_setScalarCopy(&dataValue->value, &cloudCoverValue, &UA_TYPES[UA_TYPES_DOUBLE]);
            dataValue->hasValue = true;
        }
    }

    /*
    Callback method for every read request of the weather variables in the OPC address space.
    This method is going to be call for the first time when a weather variable node is added to the address space and also for every read request.
    The weather variables need to be initialized for the first call and be updated in case of the time passed between requests is more than 15 minutes.
    Because it's a callback method from the open62541 library, you can not pass additional parameters to use as local variables, consequently the data necessary needs to be searched from the node id and web service.
    */
    static UA_StatusCode readRequest(UA_Server* server, const UA_NodeId* sessionId, void* sessionContext,
        const UA_NodeId* nodeId, void* nodeContext, UA_Boolean sourceTimeStamp, const UA_NumericRange* range, UA_DataValue* dataValue) {

        if (nodeId->identifierType == UA_NODEIDTYPE_STRING && nodeId->namespaceIndex == WebService::OPC_NS_INDEX) {
            size_t length = nodeId->identifier.string.length;
            UA_Byte* data = nodeId->identifier.string.data;
            /*
            It needs to get the country code and location name from the node id to look for them in the web service's vector.
            The NodeId is composed as: Countries.CountryCode.LocationName.WeatherVariable
            The country code will be returned at position 10 (counting starting from pos 0) and 2 letters.
            */
            std::string nodeIdName(reinterpret_cast<char*>(data), length);
            std::string countryCode = nodeIdName.substr(10, 2);
            // Search for the country in the list of countries of the web service.
            auto searchCountry = CountryData{ countryCode };
            auto itCountry = std::find(webService->getAllCountries().begin(), webService->getAllCountries().end(), searchCountry);

            /*
            The location name MAY be returned at position 13 until the first '.' after that.
            When looking for a specific location, check if it was found (iterator) because some locations has '.' in its name.
            In this case, if the location (iterator) was not found, we continue searching for the next '.' until find the correct location name.
            */
            size_t posDot = nodeIdName.find(".", 13);
            std::string locationName = nodeIdName.substr(13, posDot - 13);
            // Search for the location in the list of locations inside the country of the web service.
            auto searchLocation = LocationData{ locationName, countryCode };
            auto itLocation = std::find(itCountry->getLocations().begin(), itCountry->getLocations().end(), searchLocation);

            /* While the location is not found, continue checking for dots within its name. */
            while (itLocation == itCountry->getLocations().end()) {
                posDot = nodeIdName.find(".", posDot + 1);
                locationName = nodeIdName.substr(13, posDot - 13);
                // Search for the location in the list of locations inside the country of the web service.
                searchLocation = LocationData{ locationName, countryCode };
                itLocation = std::find(itCountry->getLocations().begin(), itCountry->getLocations().end(), searchLocation);
            }

            /* The variable name will be returned at position after the first '.' of the search of the location until the end of the node id. */
            std::string weatherVariableName = nodeIdName.substr(posDot + 1);

            // Get current time to compare with the time when the Location was downloaded.
            auto now = std::chrono::system_clock::now();
            std::chrono::minutes intervalBetweenDownloads = std::chrono::duration_cast<std::chrono::minutes>(now - itLocation->getReadLastTime());

            /* The weather data will be downloaded only for the first time or after a interval of minutes specified by the constant WebService::INTERVAL_DOWNLOAD_WEATHER_DATA. */
            if (!(itLocation->getHasBeenReceivedWeatherData()) || intervalBetweenDownloads.count() >= webService->getSettings().getIntervalWeatherDataDownload()) {
                try {
                    webService->fetchWeather(itLocation->getLatitude(), itLocation->getLongitude()).then([&](web::json::value response) {
                        itLocation->setWeatherData(WeatherData::parseJson(response));
                        itLocation->setHasBeenReceivedWeatherData(true);
                        itLocation->setReadLastTime(now);
                        }).wait();
                }
                catch (const std::exception& e) {
                    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
                        "Error on requestWeather method!");
                    UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
                        e.what());
                }
            }

            updateWeatherVariables(dataValue, itLocation->getWeatherData(), weatherVariableName);
        }

        return UA_STATUSCODE_GOOD;
    }

    /*
    Request all the weather information of the location supplied by parameter from the web service.
    Map these objects returned from the web service in OPC UA objects and put them available in the address space.
    @param *server - The OPC UA server where the objects will be added in the address space view.
    @param &location - The LocationData object which the weather will be requested for.
    @param UA_NodeId parentNodeId - The parent node id of the object where the variables will be added in OPC UA.
    */
    static void requestWeather(UA_Server* server, LocationData& location, UA_NodeId parentNodeId) {
        /* Flag to control how many time this the function requestWeather is called during the get node method of the UA_ServerConfig. */
        location.setIsAddingWeatherToAddressSpace(true);

        // #################### Latitude variable node
        /* Creates the identifier for the node id of the new variable node class
        The identifier for the node id of every variable will be: Countries.CountryCode.LocationName.Variable */
        std::string parentNameId = static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID)
            + "." + location.getCountryCode() + "." + location.getName();
        std::string latitudeNameId = parentNameId + "." + WeatherData::BROWSE_LATITUDE;
        UA_NodeId latitudeVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            latitudeNameId.c_str());
        // Creates the variable node class attribute under the parent object.
        UA_VariableAttributes latitudeVarAttr = UA_VariableAttributes_default;
        char locale[] = "en-US";
        char latitudeVarAttrDesc[] = "The latitude of a location (in decimal degrees). Positive is north, negative is south.";
        latitudeVarAttr.description = UA_LOCALIZEDTEXT(locale, latitudeVarAttrDesc);
        latitudeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_LATITUDE);

        UA_DataSource latitudeVarDataSource;
        latitudeVarDataSource.read = readRequest;
        latitudeVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, latitudeVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_LATITUDE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), latitudeVarAttr, latitudeVarDataSource, NULL, NULL);

        // #################### Longitude variable node
        /* Creates the identifier for the node id of the new variable node class
        The identifier for the node id of every variable will be: Countries.CountryCode.LocationName.Variable */
        std::string longitudeNameId = parentNameId + "." + WeatherData::BROWSE_LONGITUDE;
        UA_NodeId longitudeVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            longitudeNameId.c_str());
        // Creates the variable node class attribute under the parent object.
        UA_VariableAttributes longitudeVarAttr = UA_VariableAttributes_default;
        char longitudeVarAttrDesc[] = "The longitude of a location (in decimal degrees). Positive is east, negative is west.";
        longitudeVarAttr.description = UA_LOCALIZEDTEXT(locale, longitudeVarAttrDesc);
        longitudeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_LONGITUDE);

        UA_DataSource longitudeVarDataSource;
        longitudeVarDataSource.read = readRequest;
        longitudeVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, longitudeVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_LONGITUDE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), longitudeVarAttr, longitudeVarDataSource, NULL, NULL);

        // #################### Timezone variable node
        std::string timezoneNameId = parentNameId + "." + WeatherData::BROWSE_TIMEZONE;
        UA_NodeId timezoneVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            timezoneNameId.c_str());
        UA_VariableAttributes timezoneVarAttr = UA_VariableAttributes_default;
        char timezoneVarAttrDesc[] = "The IANA timezone name for the requested location.";
        timezoneVarAttr.description = UA_LOCALIZEDTEXT(locale, timezoneVarAttrDesc);
        timezoneVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_TIMEZONE);

        UA_DataSource timezoneVarDataSource;
        timezoneVarDataSource.read = readRequest;
        timezoneVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, timezoneVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_TIMEZONE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), timezoneVarAttr, timezoneVarDataSource, NULL, NULL);

        // #################### Icon variable node
        std::string iconNameId = parentNameId + "." + WeatherData::BROWSE_ICON;
        UA_NodeId iconVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            iconNameId.c_str());
        UA_VariableAttributes iconVarAttr = UA_VariableAttributes_default;
        char iconVarAttrDesc[] = "A machine-readable text icon of this data point, suitable for selecting an icon for display.";
        iconVarAttr.description = UA_LOCALIZEDTEXT(locale, iconVarAttrDesc);
        iconVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_ICON);

        UA_DataSource iconVarDataSource;
        iconVarDataSource.read = readRequest;
        iconVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, iconVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_ICON),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), iconVarAttr, iconVarDataSource, NULL, NULL);

        // #################### Temperature variable node
        std::string temperatureNameId = parentNameId + "." + WeatherData::BROWSE_TEMPERATURE;
        UA_NodeId temperatureVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            temperatureNameId.c_str());
        UA_VariableAttributes temperatureVarAttr = UA_VariableAttributes_default;
        char temperatureVarAttrDesc[] = "The air temperature in degrees Celsius (if units=si during request) or Fahrenheit.";
        temperatureVarAttr.description = UA_LOCALIZEDTEXT(locale, temperatureVarAttrDesc);
        temperatureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_TEMPERATURE);

        UA_DataSource temperatureVarDataSource;
        temperatureVarDataSource.read = readRequest;
        temperatureVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, temperatureVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_TEMPERATURE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), temperatureVarAttr, temperatureVarDataSource, NULL, NULL);

        // #################### Apparent temperature variable node
        std::string apparentTemperatureNameId = parentNameId + "." + WeatherData::BROWSE_APPARENT_TEMPERATURE;
        UA_NodeId apparentTemperatureVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            apparentTemperatureNameId.c_str());
        UA_VariableAttributes apparentTemperatureVarAttr = UA_VariableAttributes_default;
        char apparentTemperatureVarAttrDesc[] = "The apparent (or �feels like�) temperature in degrees Celsius (if units=si during request) or Fahrenheit.";
        apparentTemperatureVarAttr.description = UA_LOCALIZEDTEXT(locale, apparentTemperatureVarAttrDesc);
        apparentTemperatureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_APPARENT_TEMPERATURE);

        UA_DataSource apparentTemperatureVarDataSource;
        apparentTemperatureVarDataSource.read = readRequest;
        apparentTemperatureVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, apparentTemperatureVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_APPARENT_TEMPERATURE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), apparentTemperatureVarAttr, apparentTemperatureVarDataSource, NULL, NULL);

        // #################### Humidity variable node
        std::string humidityNameId = parentNameId + "." + WeatherData::BROWSE_HUMIDITY;
        UA_NodeId humidityVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            humidityNameId.c_str());
        UA_VariableAttributes humidityVarAttr = UA_VariableAttributes_default;
        char humidityVarAttrDesc[] = "The relative humidity, between 0 and 1, inclusive.";
        humidityVarAttr.description = UA_LOCALIZEDTEXT(locale, humidityVarAttrDesc);
        humidityVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_HUMIDITY);

        UA_DataSource humidityVarDataSource;
        humidityVarDataSource.read = readRequest;
        humidityVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, humidityVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_HUMIDITY),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), humidityVarAttr, humidityVarDataSource, NULL, NULL);

        // #################### Pressure variable node
        std::string pressureNameId = parentNameId + "." + WeatherData::BROWSE_PRESSURE;
        UA_NodeId pressureVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            pressureNameId.c_str());
        UA_VariableAttributes pressureVarAttr = UA_VariableAttributes_default;
        char pressureVarAttrDesc[] = "The sea-level air pressure in Hectopascals (if units=si during request) or millibars.";
        pressureVarAttr.description = UA_LOCALIZEDTEXT(locale, pressureVarAttrDesc);
        pressureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_PRESSURE);

        UA_DataSource pressureVarDataSource;
        pressureVarDataSource.read = readRequest;
        pressureVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, pressureVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_PRESSURE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), pressureVarAttr, pressureVarDataSource, NULL, NULL);

        // #################### Wind speed variable node
        std::string windSpeedNameId = parentNameId + "." + WeatherData::BROWSE_WIND_SPEED;
        UA_NodeId windSpeedVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            windSpeedNameId.c_str());
        UA_VariableAttributes windSpeedVarAttr = UA_VariableAttributes_default;
        char windSpeedVarAttrDesc[] = "The wind speed in meters per second (if units=si during request) or miles per hour.";
        windSpeedVarAttr.description = UA_LOCALIZEDTEXT(locale, windSpeedVarAttrDesc);
        windSpeedVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_WIND_SPEED);

        UA_DataSource windSpeedVarDataSource;
        windSpeedVarDataSource.read = readRequest;
        windSpeedVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, windSpeedVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_WIND_SPEED),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), windSpeedVarAttr, windSpeedVarDataSource, NULL, NULL);

        // #################### Wind Bearing variable node
        std::string windBearingNameId = parentNameId + "." + WeatherData::BROWSE_WIND_BEARING;
        UA_NodeId windBearingVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            windBearingNameId.c_str());
        UA_VariableAttributes windBearingVarAttr = UA_VariableAttributes_default;
        char windBearingVarAttrDesc[] = "The direction that the wind is coming from in degrees, with true north at 0� and progressing clockwise. (If windSpeed is zero, then this value should be ignored.)";
        windBearingVarAttr.description = UA_LOCALIZEDTEXT(locale, windBearingVarAttrDesc);
        windBearingVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_WIND_BEARING);

        UA_DataSource windBearingVarDataSource;
        windBearingVarDataSource.read = readRequest;
        windBearingVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, windBearingVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_WIND_BEARING),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), windBearingVarAttr, windBearingVarDataSource, NULL, NULL);

        // #################### Cloud cover variable node
        std::string cloudCoverNameId = parentNameId + "." + WeatherData::BROWSE_CLOUD_COVER;
        UA_NodeId cloudCoverVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
            cloudCoverNameId.c_str());
        UA_VariableAttributes cloudCoverVarAttr = UA_VariableAttributes_default;
        char cloudCoverVarAttrDesc[] = "The percentage of sky occluded by clouds, between 0 and 1, inclusive.";
        cloudCoverVarAttr.description = UA_LOCALIZEDTEXT(locale, cloudCoverVarAttrDesc);
        cloudCoverVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_CLOUD_COVER);

        UA_DataSource cloudCoverVarDataSource;
        cloudCoverVarDataSource.read = readRequest;
        cloudCoverVarDataSource.write = NULL;
        UA_Server_addDataSourceVariableNode(server, cloudCoverVarNodeId, parentNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_CLOUD_COVER),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), cloudCoverVarAttr, cloudCoverVarDataSource, NULL, NULL);

        /* Flag to control how many time this the function requestWeather is called during the get node method of the UA_ServerConfig. */
        location.setIsAddingWeatherToAddressSpace(false);
    }

    /*
    Request all the Locations objects from the web service and its subcomponents.
    Map these objects returned from the web service in OPC UA objects and put them available in the address space.
    @param *server - The OPC UA server where the objects will be added in the address space view.
    @param &country - The CountryData object which the locations will be requested for. This object will be modified
    in this function. Locations will be added to it.
    @param UA_NodeId parentNodeId - The parent node id of the object where the variables will be added in OPC UA.
    */
    static void requestLocations(UA_Server* server, CountryData& country, UA_NodeId parentNodeId) {
        try {
            // ReSharper disable once CppExpressionWithoutSideEffects
            webService->fetchAllLocations(country.getCode(), country.getLocationsNumber()).then([&](web::json::value response) {
                country.setLocations(LocationData::parseJsonArray(response));

                for (size_t i{ 0 }; i < country.getLocations().size(); i++) {
                    std::string locationName = country.getLocations().at(i).getName();
                    std::string locationCity = country.getLocations().at(i).getCity();
                    std::string locationCountryCode = country.getLocations().at(i).getCountryCode();
                    double locationLatitude = country.getLocations().at(i).getLatitude();
                    double locationLongitude = country.getLocations().at(i).getLongitude();
                    /* Creates the identifier for the node id of the new Location object
                    The identifier for the node id of every location object will be: Countries.CountryCode.LocationName */
                    std::string countries{ CountryData::COUNTRIES_FOLDER_NODE_ID };
                    std::string locationObjNameId =
                        static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID)
                        + "." + locationCountryCode + "." + locationName;
                    /* Creates an Location object node containing all the weather information related to it. */
                    UA_NodeId locationObjId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
                        locationObjNameId.c_str());
                    UA_ObjectAttributes locationObjAttr = UA_ObjectAttributes_default;
                    char locale[] = "en-US";
                    char desc[] = "Location object containing weather information";
                    locationObjAttr.description = UA_LOCALIZEDTEXT(locale, desc);
                    locationObjAttr.displayName = UA_LOCALIZEDTEXT_ALLOC(locale, locationName.c_str());
                    UA_Server_addObjectNode(server, locationObjId, parentNodeId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                        UA_QUALIFIEDNAME_ALLOC(WebService::OPC_NS_INDEX, locationName.c_str()),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), locationObjAttr, NULL, NULL);

                    std::string flagInitializeVarNameId = locationObjNameId + "." + LocationData::BROWSE_FLAG_INITIALIZE;
                    UA_NodeId flagInitializeVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, flagInitializeVarNameId.c_str());
                    UA_VariableAttributes flagInitializeVarAttr = UA_VariableAttributes_default;
                    UA_Boolean flagInitializeValue = true;
                    UA_Variant_setScalar(&flagInitializeVarAttr.value, &flagInitializeValue, &UA_TYPES[UA_TYPES_BOOLEAN]);
                    char flagInitializeAttrDesc[] = "Auxiliary variable to indicate when to download weather data for this location.";
                    flagInitializeVarAttr.description = UA_LOCALIZEDTEXT(locale, flagInitializeAttrDesc);
                    flagInitializeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, LocationData::BROWSE_FLAG_INITIALIZE);
                    UA_Server_addVariableNode(server, flagInitializeVarNodeId, locationObjId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                        UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, LocationData::BROWSE_FLAG_INITIALIZE),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), flagInitializeVarAttr, NULL, NULL);

                    country.getLocations().at(i).setIsInitialized(true);
                }
                }).wait();
        }
        catch (const std::exception& e) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
                "Error on requestLocations method!");
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
                e.what());
        }
    }

    /*
    Request all the Countries objects from the web service and its subcomponents.
    Map these objects returned from the web service in OPC UA objects and put them available in the address space.
    @param *server - The OPC UA server where the objects will be added in the address space view.
    @param UA_NodeId parentNodeId - The parent node id of the object where the variables will be added in OPC UA.
    */
    void requestCountries(UA_Server* server, const UA_NodeId& parentNodeId) {
        try {
            webService->fetchAllCountries().then([&](web::json::value response) {
                webService->setAllCountries(CountryData::parseJsonArray(response));

                for (size_t i{ 0 }; i < webService->getAllCountries().size(); i++) {
                    std::string countryName = webService->getAllCountries().at(i).getName();
                    std::string countryCode = webService->getAllCountries().at(i).getCode();
                    uint32_t countryCitiesNumber = webService->getAllCountries().at(i).getCitiesNumber();
                    uint32_t countryLocationsNumber = webService->getAllCountries().at(i).getLocationsNumber();

                    /* Creates the identifier for the node id of the new Country object
                    The identifier for the node id of every Country object will be: Countries.CountryCode */
                    std::string countryObjNameId = static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID) + "." + countryCode;
                    /* Creates an Country object node class of the folder type to containing some
                    attributes/member variables and organizes all the locations objects under it. */
                    UA_NodeId countryObjId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, countryObjNameId.c_str());
                    UA_ObjectAttributes countryObjAttr = UA_ObjectAttributes_default;
                    char locale[] = "en-US";
                    char countryObjAttrDesc[] = "Country object with attributes and locations information.";
                    countryObjAttr.description = UA_LOCALIZEDTEXT(locale, countryObjAttrDesc);
                    countryObjAttr.displayName = UA_LOCALIZEDTEXT_ALLOC(locale, countryName.c_str());
                    UA_Server_addObjectNode(server, countryObjId, parentNodeId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                        UA_QUALIFIEDNAME_ALLOC(WebService::OPC_NS_INDEX, countryName.c_str()),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), countryObjAttr, NULL, NULL);

                    std::string nameVarNameId = countryObjNameId + "." + CountryData::BROWSE_NAME;
                    UA_NodeId nameVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, nameVarNameId.c_str());
                    UA_VariableAttributes nameVarAttr = UA_VariableAttributes_default;
                    UA_String nameValue = UA_STRING_ALLOC(countryName.c_str());
                    UA_Variant_setScalar(&nameVarAttr.value, &nameValue, &UA_TYPES[UA_TYPES_STRING]);
                    char nameVarAttrDesc[] = "The name of a country";
                    nameVarAttr.description = UA_LOCALIZEDTEXT(locale, nameVarAttrDesc);
                    nameVarAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::BROWSE_NAME);
                    UA_Server_addVariableNode(server, nameVarNodeId, countryObjId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                        UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, CountryData::BROWSE_NAME),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), nameVarAttr, NULL, NULL);

                    std::string codeVarNameId = countryObjNameId + "." + CountryData::BROWSE_CODE;
                    UA_NodeId codeVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, codeVarNameId.c_str());
                    UA_VariableAttributes codeVarAttr = UA_VariableAttributes_default;
                    UA_String codeValue = UA_STRING_ALLOC(countryCode.c_str());
                    UA_Variant_setScalar(&codeVarAttr.value, &codeValue, &UA_TYPES[UA_TYPES_STRING]);
                    char codeVarAttrDesc[] = "2 letters ISO code representing the Country Name";
                    codeVarAttr.description = UA_LOCALIZEDTEXT(locale, codeVarAttrDesc);
                    codeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::BROWSE_CODE);
                    UA_Server_addVariableNode(server, codeVarNodeId, countryObjId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                        UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, CountryData::BROWSE_CODE),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), codeVarAttr, NULL, NULL);

                    std::string citiesNumberVarNameId = countryObjNameId + "." + CountryData::BROWSE_CITIES_NUMBER;
                    UA_NodeId citiesNumberVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, citiesNumberVarNameId.c_str());
                    UA_VariableAttributes citiesNumberVarAttr = UA_VariableAttributes_default;
                    UA_UInt32 citiesNumberValue = countryCitiesNumber;
                    UA_Variant_setScalar(&citiesNumberVarAttr.value, &citiesNumberValue, &UA_TYPES[UA_TYPES_UINT32]);
                    char citiesNumberVarAttrDesc[] = "Number of cities belonged to a country. It can be city or province";
                    citiesNumberVarAttr.description = UA_LOCALIZEDTEXT(locale, citiesNumberVarAttrDesc);
                    citiesNumberVarAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::BROWSE_CITIES_NUMBER);
                    UA_Server_addVariableNode(server, citiesNumberVarNodeId, countryObjId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                        UA_QUALIFIEDNAME(1, CountryData::BROWSE_CITIES_NUMBER),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), citiesNumberVarAttr, NULL, NULL);

                    std::string locationsNumberVarNameId = countryObjNameId + "." + CountryData::BROWSE_LOCATIONS_NUMBER;
                    UA_NodeId locationsNumberVarNodeId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX,
                        locationsNumberVarNameId.c_str());
                    UA_VariableAttributes locationsNumberVarAttr = UA_VariableAttributes_default;
                    UA_UInt32 locationsNumberValue = countryLocationsNumber;
                    UA_Variant_setScalar(&locationsNumberVarAttr.value, &locationsNumberValue, &UA_TYPES[UA_TYPES_UINT32]);
                    char locationsNumberVarAttrDesc[] = "Number of cities belonged to a country. It can be city or province";
                    locationsNumberVarAttr.description = UA_LOCALIZEDTEXT(locale, locationsNumberVarAttrDesc);
                    locationsNumberVarAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::BROWSE_LOCATIONS_NUMBER);
                    UA_Server_addVariableNode(server, locationsNumberVarNodeId, countryObjId,
                        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                        UA_QUALIFIEDNAME(1, CountryData::BROWSE_LOCATIONS_NUMBER),
                        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), locationsNumberVarAttr, NULL, NULL);

                    webService->getAllCountries().at(i).setIsInitialized(true);
                }
                }).wait();
        }
        catch (const std::exception& e) {
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
                "Error on requestCountries method!");
            UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
                e.what());
        }
    }

    /*
    Add the Countries object node class in the address space and call the request for the other countries sub objects and its subcomponents.
    @param *server - The OPC UA server where the objects will be added in the address space view.
    */
    static void addCountries(UA_Server* server) {
        // This will control how many times this method executes. It's to be executed ONLY ONCE!
        static bool wasItCalled = false;

        // If was not executed yet, add the countries objects. Otherwise, do nothing.
        if (!wasItCalled) {
            wasItCalled = true;

            // Creates an Countries object node class of the folder type to organizes all the locations objects under it.
            UA_NodeId countriesObjId = UA_NODEID_STRING(WebService::OPC_NS_INDEX,
                CountryData::COUNTRIES_FOLDER_NODE_ID);
            UA_ObjectAttributes countriesObjAttr = UA_ObjectAttributes_default;
            char locale[] = "en-US";
            char countriesObjAttrDesc[] = "Organizes all the Countries object with their respective information";
            countriesObjAttr.description = UA_LOCALIZEDTEXT(locale, countriesObjAttrDesc);
            countriesObjAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::COUNTRIES_FOLDER_NODE_ID);
            UA_Server_addObjectNode(server, countriesObjId,
                UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, CountryData::COUNTRIES_FOLDER_NODE_ID),
                UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), countriesObjAttr, NULL, NULL);

            requestCountries(server, countriesObjId);
        }
    }

    /*
    Function pointer to UA_NodeMap_getNode function that is initialized on the UA_ServerConfig_new_default() through the UA_Nodestore.
    This default function needs to be called from our custom get node function that is assigned to the UA_Nodestore from the UA_ServerConfig.
    */
    const UA_Node* (*defaultGetNode)(void* nodestoreContext, const UA_NodeId* nodeId);

    /*
    This function is called a lot of times, specially when browse requests from the client are send to the server.
    When clients request to read any specific country object node, the locations of that country will be downloaded if not exists.
    When clients request to read any specific location object node, the weather data of that location will be downloaded or updated.
    The default function from UA_Nodestore from the UA_ServerConfig needs to be returned.
    */
    const UA_Node* customGetNode(void* nodestoreContext, const UA_NodeId* nodeId) {
        if (nodeId->identifierType == UA_NODEIDTYPE_STRING && nodeId->namespaceIndex == WebService::OPC_NS_INDEX) {
            size_t length = nodeId->identifier.string.length;
            UA_Byte* data = nodeId->identifier.string.data;
            std::string nodeIdName(reinterpret_cast<char*>(data), length);

            /*
            The NodeId is composed as : Countries.CountryCode.LocationName.WeatherVariable
            If length of node id is greather than 13, it means at least the client is requesting to read a specific country or its sub nodes.
            */
            if (length > 13) {
                std::string countryCode = nodeIdName.substr(10, 2);
                std::string countryObjNameId = static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID) + "." + countryCode;
                UA_NodeId countryObjId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, countryObjNameId.c_str());
                // Search for the country in the list of countries of the web service.
                auto searchCountry = CountryData{ countryCode };
                auto itCountry = std::find(webService->getAllCountries().begin(), webService->getAllCountries().end(), searchCountry);

                // Only download locations if they don't exist and the country has been initialized (added to the address space).
                if (itCountry->getIsInitialized() && itCountry->getLocations().size() == 0) {
                    requestLocations(webService->getServer(), *itCountry, countryObjId);
                }
                // Only try to download weather data if locations has been added to the country being read.
                if (itCountry->getIsInitialized() && itCountry->getLocations().size() > 0) {
                    /*
                    If find the dot after beginning of location's name, it MAY mean the client is requesting to read a specific location or it just mean the location name has a '.' WITHIN the name.
                    For example, the following nodes id do not mean the client is requesting to read the location:
                    Countries.CA.Brandon
                    Countries.CA.Main St.
                    And the following nodes id mean the client is requesting to read the location:
                    Countries.CA.Brandon.FlagInitialize
                    Countries.CA.Main St.FlagInitialize
                    */
                    size_t posDot = nodeIdName.find(".", 13);

                    /* If we don't find the dot on the first search, for sure that is not a request to read the location. */
                    if (posDot != std::string::npos) {
                        /* The location name MAY be returned at position 13 until the first '.' after that. */
                        std::string locationName = nodeIdName.substr(13, posDot - 13);
                        // Search for the location in the list of locations inside the country of the web service.
                        auto searchLocation = LocationData{ locationName, countryCode };
                        auto itLocation = std::find(itCountry->getLocations().begin(), itCountry->getLocations().end(), searchLocation);

                        /* While the location is not found, continue checking for dots within its name. */
                        while (itLocation == itCountry->getLocations().end()) {
                            posDot = nodeIdName.find(".", posDot + 1);
                            locationName = nodeIdName.substr(13, posDot - 13);
                            // Search for the location in the list of locations inside the country of the web service.
                            searchLocation = LocationData{ locationName, countryCode };
                            itLocation = std::find(itCountry->getLocations().begin(), itCountry->getLocations().end(), searchLocation);
                        }

                        std::string locationObjNameId = countryObjNameId + "." + locationName;
                        /*
                        The location name was found correctly. Check if there are more characters after the location name comparing the full node id name's size to the node id until the location name.
                        */
                        if (nodeIdName.size() > locationObjNameId.size()) {
                            UA_NodeId locationObjId = UA_NODEID_STRING_ALLOC(WebService::OPC_NS_INDEX, locationObjNameId.c_str());
                            /*
                            Only try to download weather data if they not exist in the address space.
                            The isAddingWeatherToAddressSpace boolean variable in LocationData controls when we are adding the weather data in the OPC UA address space, that being said the requestWeather function bellow will not be called more than once.
                            */
                            if (itLocation->getIsInitialized() && !(itLocation->getHasBeenReceivedWeatherData()) && !(itLocation->getIsAddingWeatherToAddressSpace())) {
                                requestWeather(webService->getServer(), *itLocation, locationObjId);
                            }
                        }
                    }
                }
            }
        }

        return defaultGetNode(nodestoreContext, nodeId);
    }
}

int main(int argc, char* argv[]) {

    //extracting current directory
    std::string fullPath(argv[0]);
    std::string execDir = fullPath.substr(0, fullPath.find_last_of("/\\") + 1); //including separator since we are adding file name later
    std::cout << "Executable directory: " << execDir << std::endl;

    /*
      Assign default values to variables.
      Attempt to open "settings.json" file to get Dark Sky API key and other settings to override dafault values.
      If there is a problem opening file, parsing or Dark Sky API key seems to be invalid - terminate the program.
    */
    weatherserver::Settings settings(execDir);
    if (!settings.areValid()) {
        std::cout << "Invalid settings values. Terminating..." << std::endl;
        return EXIT_FAILURE;
    }

    weatherserver::WebService ws(settings);
    webService = &ws;

    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    //UA_Server *server = UA_Server_new();
    //UA_ServerConfig *config = UA_Server_getConfig(server);

    //UA_Nodestore* ns = (UA_Nodestore*) UA_Server_getNodestore(server);

    //defaultGetNode = ns->getNode;

    //ns->getNode = customGetNode;

    //UA_ServerConfig_setDefault(config);
    /* 1.0rc5 version

    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    */

    // 0.3 version

    UA_ServerConfig* config = UA_ServerConfig_new_default();
    weatherserver::defaultGetNode = config->nodestore.getNode;
    config->nodestore.getNode = weatherserver::customGetNode;

    UA_Server* server = UA_Server_new(config);


    webService->setServer(server);

    /* Should the server networklayer block (with a timeout) until a message
    arrives or should it return immediately? */
    UA_Boolean waitInternal = false;

    UA_StatusCode retval = UA_Server_run_startup(server);
    if (retval != UA_STATUSCODE_GOOD) {
        std::cout << "Couldn't start the UA server. Terminating..." << std::endl;
        return EXIT_FAILURE;
    }

    while (running) {
        /* timeout is the maximum possible delay (in millisec) until the next
        _iterate call. Otherwise, the server might miss an internal timeout
        or cannot react to messages with the promised responsiveness. */
        /* If multicast discovery server is enabled, the timeout does not not consider new input data (requests) on the mDNS socket.
        * It will be handled on the next call, which may be too late for requesting clients.
        * if needed, the select with timeout on the multicast socket server->mdnsSocket (see example in mdnsd library)
        */
        UA_UInt16 timeout = UA_Server_run_iterate(server, waitInternal);

        // HERE you can add any node to the server you like.
        // Make sure that you only call any created method once in this loop.
        weatherserver::addCountries(server);

        /* Now we can use the max timeout to do something else. In this case, we just sleep. */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    retval = UA_Server_run_shutdown(server);
    UA_Server_delete(server);

    //UA_ServerConfig_clean(config);

    //0.3 version. no config delete in 1.0rc5

    UA_ServerConfig_delete(config);

    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}