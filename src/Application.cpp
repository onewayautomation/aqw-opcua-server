#include <signal.h>
#include <stdlib.h>

#include <fstream>
#include <thread>
#include <algorithm>

//amalgamated version of open62541
#include "open62541.h"

#include "Settings.h"
#include "WebService.h"



//Global variables - be aware of them.

//Service responsible for REST calls on the internet.
weatherserver::WebService* webService;

UA_Boolean running = true;

static void stopHandler(int sig) {
  UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received signal %d", sig);
  running = false;
}

namespace weatherserver {

  /*
  Update dataValue for weatherVariableName node in OPC UA information model for the location that passed this weatherData object.

  @param dataValue - data value of the variable that will be updated in OPC UA information model.
  @param weatherData - object with new data.
  @param weatherVariableName - weather variable browse name to check which variable node to update.
  */
  static void updateWeatherVariable(UA_DataValue& dataValue, const WeatherData& weatherData, const std::string& weatherVariableName) {
    if (weatherVariableName == WeatherData::BROWSE_LATITUDE) {
      UA_Double latitudeValue = weatherData.getLatitude();
      UA_Variant_setScalarCopy(&dataValue.value, &latitudeValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_LONGITUDE) {
      UA_Double longitudeValue = weatherData.getLongitude();
      UA_Variant_setScalarCopy(&dataValue.value, &longitudeValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_TIMEZONE) {
      UA_String timezoneValue = UA_STRING(const_cast<char*>(weatherData.getTimezone().c_str()));
      UA_Variant_setScalarCopy(&dataValue.value, &timezoneValue, &UA_TYPES[UA_TYPES_STRING]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_ICON) {
      UA_String iconValue = UA_STRING(const_cast<char*>(weatherData.getCurrentlyIcon().c_str()));
      UA_Variant_setScalarCopy(&dataValue.value, &iconValue, &UA_TYPES[UA_TYPES_STRING]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_TEMPERATURE) {
      UA_Double temperatureValue = weatherData.getCurrentlyTemperature();
      UA_Variant_setScalarCopy(&dataValue.value, &temperatureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_APPARENT_TEMPERATURE) {
      UA_Double apparentTemperatureValue = weatherData.getCurrentlyApparentTemperature();
      UA_Variant_setScalarCopy(&dataValue.value, &apparentTemperatureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_HUMIDITY) {
      UA_Double humidityValue = weatherData.getCurrentlyHumidity();
      UA_Variant_setScalarCopy(&dataValue.value, &humidityValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_PRESSURE) {
      UA_Double pressureValue = weatherData.getCurrentlyPressure();
      UA_Variant_setScalarCopy(&dataValue.value, &pressureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_WIND_SPEED) {
      UA_Double windSpeedValue = weatherData.getCurrentlyWindSpeed();
      UA_Variant_setScalarCopy(&dataValue.value, &windSpeedValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_WIND_BEARING) {
      UA_Double windBearingValue = weatherData.getCurrentlyWindBearing();
      UA_Variant_setScalarCopy(&dataValue.value, &windBearingValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
    else if (weatherVariableName == WeatherData::BROWSE_CLOUD_COVER) {
      UA_Double cloudCoverValue = weatherData.getCurrentlyCloudCover();
      UA_Variant_setScalarCopy(&dataValue.value, &cloudCoverValue, &UA_TYPES[UA_TYPES_DOUBLE]);
      dataValue.hasValue = true;
    }
  }

  /*
  Callback method for every read request of the weather variables in the OPC information model.

  This method is going to be called for the first time when a weather variable node is added to the model and also
  for every read request afterwards.

  Weather variables need to be initialized for the first call and need to be updated if it was more than <read interval> minutes
  after last read request.

  Because it's a callback method from the open62541 library, you can not pass additional parameters to use as local variables,
  consequently the data necessary needs to be searched from the node id and web service.
  */
  static UA_StatusCode readRequest(UA_Server* server, const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* nodeId, void* nodeContext, UA_Boolean sourceTimeStamp, const UA_NumericRange* range, UA_DataValue* dataValue)
  {
    (void)range; //TODO: for weather data it does not make sense to return range of values, check OPC UA Spec, maybe should return error in case if range is not null.
    (void)sourceTimeStamp; //TODO: maybe this argument should be used.
    (void)sessionContext;
    (void)sessionId;
    (void)server;
    (void)nodeContext;

    if (nodeId->identifierType == UA_NODEIDTYPE_STRING && nodeId->namespaceIndex == WebService::OPC_NS_INDEX) {
      size_t length = nodeId->identifier.string.length;
      UA_Byte* data = nodeId->identifier.string.data;

      /*
      It needs to get the country code and location name from the nodeId to look for them in the web service's map.
      The nodeId is composed as: Countries.CountryCode.LocationName.WeatherVariable
      Two country code letters will be returned at position 10 and 11 (starting from 0).
      */
      std::string nodeIdName(reinterpret_cast<char*>(data), length);
      std::string countryCode = nodeIdName.substr(10, 2);

      // Search for the country in the list of countries of the web service.
      auto& countries = webService->getAllCountries();
      auto itCountry = countries.find(countryCode);
      if (itCountry != countries.end()) {
        /*
        The location name MAY be returned at position 13 until the first '.' after that.
        When looking for a specific location, check if it was found (iterator) because some locations has '.' in its name.
        In this case, if the location (iterator) was not found, we continue searching for the next '.'
        until find the correct location name.
        */
        size_t posDot = nodeIdName.find(".", 13);
        std::string locationName = nodeIdName.substr(13, posDot - 13);

        // Search for the location in the list of locations inside the country of the web service.
        auto& locations = itCountry->second.getLocations();
        auto itLocation = locations.find(locationName);

        while (itLocation == locations.end() && posDot != std::string::npos) {
          posDot = nodeIdName.find(".", posDot + 1);
          if (posDot != std::string::npos)
            locationName = nodeIdName.substr(13, posDot - 13);
          else
            locationName = nodeIdName.substr(13);
          itLocation = locations.find(locationName);
        }

        if (itLocation != locations.end()) {
          /*
          The variable name will be returned at position after the first '.' of the search of the location
          until the end of the node id.
          */
          auto& location = itLocation->second;

          std::string weatherVariableName = nodeIdName.substr(posDot + 1);

          // Get current time to compare with the time when the Location was downloaded.
          auto now = std::chrono::system_clock::now();
          std::chrono::minutes intervalBetweenDownloads = std::chrono::duration_cast<std::chrono::minutes>(now - location.getReadLastTime());

          /* The weather data will be downloaded only for the first time or after a interval of minutes specified by the constant WebService::INTERVAL_DOWNLOAD_WEATHER_DATA. */
          if (!(location.getHasBeenReceivedWeatherData()) || intervalBetweenDownloads.count() >= webService->getSettings().getIntervalWeatherDataDownload()) {
            try {
              webService->fetchWeather(location.getLatitude(), location.getLongitude()).then([&](web::json::value response)
                {
                  location.setWeatherData(WeatherData::parseJson(response));
                  location.setHasBeenReceivedWeatherData(true);
                  location.setReadLastTime(now);
                }).wait();
            }
            catch (const std::exception & e) {
              UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK, "Error on requestWeather method: [%s]", e.what());
            }
          }
          updateWeatherVariable(*dataValue, location.getWeatherData(), weatherVariableName);
        }
      }
      else {
        // TODO: Log
      }
    }

    return UA_STATUSCODE_GOOD;
  }

  /*
  Request weather from the web service for the specified "location". Add its values as DataSourceVariableNodes to the OPC UA information model.

  @param parentLocationNodeId - nodeId that we are going to use as a parent for these weather object nodes.
  */
  static void requestWeather(UA_Server* server, LocationData& location, const UA_NodeId& parentLocationNodeId) {

    // Flag to control how many time this the function requestWeather is called during the get node method of the UA_ServerConfig.
    location.setIsAddingWeatherToAddressSpace(true);

    // #################### Latitude variable node
    /* Creates the identifier for the node id of the new variable node class
    The identifier for the node id of every variable will be: Countries.CountryCode.LocationName.Variable */
    std::string parentNameId = static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID)
      + "." + location.getCountryCode() + "." + location.getName();
    std::string latitudeNameId = parentNameId + "." + WeatherData::BROWSE_LATITUDE;
    UA_NodeId latitudeVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(latitudeNameId.c_str()));
    // Creates the variable node class attribute under the parent object.
    UA_VariableAttributes latitudeVarAttr = UA_VariableAttributes_default;
    char locale[] = "en-US";
    char latitudeVarAttrDesc[] = "The latitude of a location (in decimal degrees). Positive is north, negative is south.";
    latitudeVarAttr.description = UA_LOCALIZEDTEXT(locale, latitudeVarAttrDesc);
    latitudeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_LATITUDE);

    UA_DataSource latitudeVarDataSource;
    latitudeVarDataSource.read = readRequest;
    latitudeVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, latitudeVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_LATITUDE),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), latitudeVarAttr, latitudeVarDataSource, NULL, NULL);

    // #################### Longitude variable node
    /* Creates the identifier for the node id of the new variable node class
    The identifier for the node id of every variable will be: Countries.CountryCode.LocationName.Variable */
    std::string longitudeNameId = parentNameId + "." + WeatherData::BROWSE_LONGITUDE;
    UA_NodeId longitudeVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(longitudeNameId.c_str()));
    // Creates the variable node class attribute under the parent object.
    UA_VariableAttributes longitudeVarAttr = UA_VariableAttributes_default;
    char longitudeVarAttrDesc[] = "The longitude of a location (in decimal degrees). Positive is east, negative is west.";
    longitudeVarAttr.description = UA_LOCALIZEDTEXT(locale, longitudeVarAttrDesc);
    longitudeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_LONGITUDE);

    UA_DataSource longitudeVarDataSource;
    longitudeVarDataSource.read = readRequest;
    longitudeVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, longitudeVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_LONGITUDE),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), longitudeVarAttr, longitudeVarDataSource, NULL, NULL);

    // #################### Timezone variable node
    std::string timezoneNameId = parentNameId + "." + WeatherData::BROWSE_TIMEZONE;
    UA_NodeId timezoneVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(timezoneNameId.c_str()));
    UA_VariableAttributes timezoneVarAttr = UA_VariableAttributes_default;
    char timezoneVarAttrDesc[] = "The IANA timezone name for the requested location.";
    timezoneVarAttr.description = UA_LOCALIZEDTEXT(locale, timezoneVarAttrDesc);
    timezoneVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_TIMEZONE);

    UA_DataSource timezoneVarDataSource;
    timezoneVarDataSource.read = readRequest;
    timezoneVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, timezoneVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_TIMEZONE),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), timezoneVarAttr, timezoneVarDataSource, NULL, NULL);

    // #################### Icon variable node
    std::string iconNameId = parentNameId + "." + WeatherData::BROWSE_ICON;
    UA_NodeId iconVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(iconNameId.c_str()));
    UA_VariableAttributes iconVarAttr = UA_VariableAttributes_default;
    char iconVarAttrDesc[] = "A machine-readable text icon of this data point, suitable for selecting an icon for display.";
    iconVarAttr.description = UA_LOCALIZEDTEXT(locale, iconVarAttrDesc);
    iconVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_ICON);

    UA_DataSource iconVarDataSource;
    iconVarDataSource.read = readRequest;
    iconVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, iconVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_ICON),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), iconVarAttr, iconVarDataSource, NULL, NULL);

    // #################### Temperature variable node
    std::string temperatureNameId = parentNameId + "." + WeatherData::BROWSE_TEMPERATURE;
    UA_NodeId temperatureVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(temperatureNameId.c_str()));
    UA_VariableAttributes temperatureVarAttr = UA_VariableAttributes_default;
    char temperatureVarAttrDesc[] = "The air temperature in degrees Celsius (if units=si during request) or Fahrenheit.";
    temperatureVarAttr.description = UA_LOCALIZEDTEXT(locale, temperatureVarAttrDesc);
    temperatureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_TEMPERATURE);

    UA_DataSource temperatureVarDataSource;
    temperatureVarDataSource.read = readRequest;
    temperatureVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, temperatureVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_TEMPERATURE),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), temperatureVarAttr, temperatureVarDataSource, NULL, NULL);

    // #################### Apparent temperature variable node
    std::string apparentTemperatureNameId = parentNameId + "." + WeatherData::BROWSE_APPARENT_TEMPERATURE;
    UA_NodeId apparentTemperatureVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(apparentTemperatureNameId.c_str()));
    UA_VariableAttributes apparentTemperatureVarAttr = UA_VariableAttributes_default;
    char apparentTemperatureVarAttrDesc[] = "The apparent (or `feels like`) temperature in degrees Celsius (units=si) or Fahrenheit.";
    apparentTemperatureVarAttr.description = UA_LOCALIZEDTEXT(locale, apparentTemperatureVarAttrDesc);
    apparentTemperatureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_APPARENT_TEMPERATURE);

    UA_DataSource apparentTemperatureVarDataSource;
    apparentTemperatureVarDataSource.read = readRequest;
    apparentTemperatureVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, apparentTemperatureVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_APPARENT_TEMPERATURE),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), apparentTemperatureVarAttr, apparentTemperatureVarDataSource, NULL, NULL);

    // #################### Humidity variable node
    std::string humidityNameId = parentNameId + "." + WeatherData::BROWSE_HUMIDITY;
    UA_NodeId humidityVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(humidityNameId.c_str()));
    UA_VariableAttributes humidityVarAttr = UA_VariableAttributes_default;
    char humidityVarAttrDesc[] = "The relative humidity, between 0 and 1, inclusive.";
    humidityVarAttr.description = UA_LOCALIZEDTEXT(locale, humidityVarAttrDesc);
    humidityVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_HUMIDITY);

    UA_DataSource humidityVarDataSource;
    humidityVarDataSource.read = readRequest;
    humidityVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, humidityVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_HUMIDITY),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), humidityVarAttr, humidityVarDataSource, NULL, NULL);

    // #################### Pressure variable node
    std::string pressureNameId = parentNameId + "." + WeatherData::BROWSE_PRESSURE;
    UA_NodeId pressureVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(pressureNameId.c_str()));
    UA_VariableAttributes pressureVarAttr = UA_VariableAttributes_default;
    char pressureVarAttrDesc[] = "The sea-level air pressure in Hectopascals (if units=si during request) or millibars.";
    pressureVarAttr.description = UA_LOCALIZEDTEXT(locale, pressureVarAttrDesc);
    pressureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_PRESSURE);

    UA_DataSource pressureVarDataSource;
    pressureVarDataSource.read = readRequest;
    pressureVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, pressureVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_PRESSURE),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), pressureVarAttr, pressureVarDataSource, NULL, NULL);

    // #################### Wind speed variable node
    std::string windSpeedNameId = parentNameId + "." + WeatherData::BROWSE_WIND_SPEED;
    UA_NodeId windSpeedVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(windSpeedNameId.c_str()));
    UA_VariableAttributes windSpeedVarAttr = UA_VariableAttributes_default;
    char windSpeedVarAttrDesc[] = "The wind speed in meters per second (if units=si during request) or miles per hour.";
    windSpeedVarAttr.description = UA_LOCALIZEDTEXT(locale, windSpeedVarAttrDesc);
    windSpeedVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_WIND_SPEED);

    UA_DataSource windSpeedVarDataSource;
    windSpeedVarDataSource.read = readRequest;
    windSpeedVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, windSpeedVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_WIND_SPEED),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), windSpeedVarAttr, windSpeedVarDataSource, NULL, NULL);

    // #################### Wind Bearing variable node
    std::string windBearingNameId = parentNameId + "." + WeatherData::BROWSE_WIND_BEARING;
    UA_NodeId windBearingVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(windBearingNameId.c_str()));
    UA_VariableAttributes windBearingVarAttr = UA_VariableAttributes_default;
    char windBearingVarAttrDesc[] = "The direction that the wind is coming from in degrees, with true north at 0� and progressing clockwise. (If windSpeed is zero, then this value should be ignored.)";
    windBearingVarAttr.description = UA_LOCALIZEDTEXT(locale, windBearingVarAttrDesc);
    windBearingVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_WIND_BEARING);

    UA_DataSource windBearingVarDataSource;
    windBearingVarDataSource.read = readRequest;
    windBearingVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, windBearingVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_WIND_BEARING),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), windBearingVarAttr, windBearingVarDataSource, NULL, NULL);

    // #################### Cloud cover variable node
    std::string cloudCoverNameId = parentNameId + "." + WeatherData::BROWSE_CLOUD_COVER;
    UA_NodeId cloudCoverVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(cloudCoverNameId.c_str()));
    UA_VariableAttributes cloudCoverVarAttr = UA_VariableAttributes_default;
    char cloudCoverVarAttrDesc[] = "The percentage of sky occluded by clouds, between 0 and 1, inclusive.";
    cloudCoverVarAttr.description = UA_LOCALIZEDTEXT(locale, cloudCoverVarAttrDesc);
    cloudCoverVarAttr.displayName = UA_LOCALIZEDTEXT(locale, WeatherData::BROWSE_CLOUD_COVER);

    UA_DataSource cloudCoverVarDataSource;
    cloudCoverVarDataSource.read = readRequest;
    cloudCoverVarDataSource.write = NULL;
    UA_Server_addDataSourceVariableNode(server, cloudCoverVarNodeId, parentLocationNodeId,
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
      UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, WeatherData::BROWSE_CLOUD_COVER),
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), cloudCoverVarAttr, cloudCoverVarDataSource, NULL, NULL);

    /* Flag to control how many time this the function requestWeather is called during the get node method of the UA_ServerConfig. */
    location.setIsAddingWeatherToAddressSpace(false);
  }

  /*
  Checking if locations number in the server model matches with passed value. Trying to adjust if its different.
  Returns true if:
      - model locations number was successfuly read and already matches with passed value;
      - value is different, but adjustment (write) was successful.
  Returns false if:
      - couldn't read existing value from the model;
      - existing value was different and adjustment (write) failed.
  */
  static bool validateLocationsNumberInTheModel(UA_Server& server, const CountryData& country, uint32_t valueToValidate) {

    bool validationFlag = false;

    //crafting NodeID for our locations number attribute inside the information model
    std::string locationsNumberAttributeString = std::string(CountryData::COUNTRIES_FOLDER_NODE_ID) + "."
      + country.getCode() + "." + std::string(CountryData::BROWSE_LOCATIONS_NUMBER);

    std::cout << "Checking locations number for the following ID: " << locationsNumberAttributeString << std::endl;

    UA_NodeId locationsNumberAttributeNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX,
      const_cast<char*>(locationsNumberAttributeString.c_str()));

    UA_Variant valueInModel;
    UA_Variant_init(&valueInModel);
    UA_StatusCode retvalRead = UA_Server_readValue(&server, locationsNumberAttributeNodeId, &valueInModel);

    if (retvalRead == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&valueInModel, &UA_TYPES[UA_TYPES_UINT32])) {

      UA_UInt32 valueInModelUInt = *(UA_UInt32*)valueInModel.data;
      std::cout << "Value in the information model: " << valueInModelUInt << std::endl;

      UA_UInt32 valueToValidateUInt = valueToValidate;
      std::cout << "Actual value: " << valueToValidateUInt << std::endl;

      if (valueInModelUInt != valueToValidateUInt) {
        std::cout << "Trying to adjust..." << std::endl;

        UA_Variant_setScalarCopy(&valueInModel, &valueToValidateUInt, &UA_TYPES[UA_TYPES_UINT32]);
        UA_StatusCode retvalWrite = UA_Server_writeValue(&server, locationsNumberAttributeNodeId, valueInModel);

        if (retvalWrite == UA_STATUSCODE_GOOD) {

          std::cout << "Successfuly put new value in the model. " << std::endl;
          validationFlag = true;
        }
        else {
          //validationFlag still false
          std::cout << "Adjustment failed." << std::endl;
        }
      }
      else {
        std::cout << "Number of locations in the model already matches with actual value." << std::endl;
        validationFlag = true;
      }
    }
    else {
      //validationFlag still false
      std::cout << "Coudn't check number of locations in the information model for this country: " << country.getName() << std::endl;
    }

    UA_Variant_deleteMembers(&valueInModel);
    return validationFlag;
  }

  /*
  Request locations from the web service for the specified "country". Add them as ObjectNodes to the OPC UA information model.
  Add VariableNode with "initialized" flag for them.

  @param parentCountryNodeId - nodeId for our "country". We use it as a parent for all locations in OPC UA model.
  */
  static void requestLocations(UA_Server* server, CountryData& country, const UA_NodeId& parentCountryNodeId) {
    try {

      uint32_t currentLocationsNumber = country.getLocationsNumber();

      webService->fetchAllLocations(country.getCode(), currentLocationsNumber).then([&](web::json::value response)
        {
          country.setLocations(LocationData::parseJsonArray(response));

          uint32_t parseMapSize = (uint32_t)country.getLocations().size();

          if (currentLocationsNumber != parseMapSize) {
            std::cout << "Locations number parameter for this country doesn't match with actual JSON parse. Validating the model..." << std::endl;
            if (validateLocationsNumberInTheModel(*server, country, parseMapSize)) {
              country.setLocationsNumber(parseMapSize);
              std::cout << "Validation successful. New locations number: " << parseMapSize << std::endl;
            }
            else {
              std::cout << "Validation failed. Keeping current number." << std::endl;
            }
          }

          // Note that both itLocation and location variables are used as reference to the original object, not copy.
          // Therefore changes apply to originals.
          for (auto& itLocation : country.getLocations()) {
            auto& location = itLocation.second;
            std::string locationName = location.getName();
            std::string locationCity = location.getCity();
            std::string locationCountryCode = location.getCountryCode();
            /* Creates the identifier for the node id of the new Location object
            The identifier for the node id of every location object will be: Countries.CountryCode.LocationName */
            std::string countries{ CountryData::COUNTRIES_FOLDER_NODE_ID };
            std::string locationObjNameId =
              static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID)
              + "." + locationCountryCode + "." + locationName;
            /* Creates an Location object node containing all the weather information related to it. */
            UA_NodeId locationObjId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(locationObjNameId.c_str()));
            UA_ObjectAttributes locationObjAttr = UA_ObjectAttributes_default;
            char locale[] = "en-US";
            char desc[] = "Location object containing weather information";
            locationObjAttr.description = UA_LOCALIZEDTEXT(locale, desc);
            locationObjAttr.displayName = UA_LOCALIZEDTEXT(locale, const_cast<char*>(locationName.c_str()));

            auto addResult = UA_Server_addObjectNode(server, locationObjId, parentCountryNodeId,
              UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
              UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, const_cast<char*>(locationName.c_str())),
              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), locationObjAttr, NULL, NULL);
            if (addResult == UA_STATUSCODE_GOOD)
            {
              std::string flagInitializeVarNameId = locationObjNameId + "." + LocationData::BROWSE_FLAG_INITIALIZE;
              UA_NodeId flagInitializeVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(flagInitializeVarNameId.c_str()));
              UA_VariableAttributes flagInitializeVarAttr = UA_VariableAttributes_default;
              UA_Boolean flagInitializeValue = true;
              UA_Variant_setScalar(&flagInitializeVarAttr.value, &flagInitializeValue, &UA_TYPES[UA_TYPES_BOOLEAN]);
              char flagInitializeAttrDesc[] = "Auxiliary variable to indicate when to download weather data for this location.";
              flagInitializeVarAttr.description = UA_LOCALIZEDTEXT(locale, flagInitializeAttrDesc);
              flagInitializeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, LocationData::BROWSE_FLAG_INITIALIZE);
              auto addVariableResult = UA_Server_addVariableNode(server, flagInitializeVarNodeId, locationObjId,
                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, LocationData::BROWSE_FLAG_INITIALIZE),
                UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), flagInitializeVarAttr, NULL, NULL);
              if (addVariableResult != UA_STATUSCODE_GOOD)
              {
                UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                  "Failed to add OPC UA node for variable %s.%s.%s, error code = 0x%x",
                  locationCountryCode.c_str(), locationName.c_str(), flagInitializeVarNameId.c_str(), addResult);
              }
              location.setIsInitialized(true);
            }
            else
            {
              UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER,
                "Failed to add OPC UA node for location %s.%s, error code = 0x%x",
                locationCountryCode.c_str(), locationName.c_str(), addResult);
            }
          }
        }).wait();
    }
    catch (const std::exception & e) { //TODO - catch more specific type of exception
      UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK, "Error on requestLocations method: [%s]", e.what());
    }
  }

  /*
  Request available countries from the web service. Add them as ObjectNodes to the OPC UA information model.
  Add additional country parameters as VariableNodes.

  @param rootNodeId - nodeId for the root "Countries" node.
  */
  void requestCountries(UA_Server* server, const UA_NodeId& rootNodeId) {
    try {
      webService->fetchAllCountries().then([&](web::json::value response) {
        webService->setAllCountries(CountryData::parseJsonArray(response));
        auto& countries = webService->getAllCountries();

        for (auto itCountry = countries.begin(); itCountry != countries.end(); itCountry++) {
          auto& country = itCountry->second;
          std::string countryName = country.getName();
          std::string countryCode = country.getCode();
          uint32_t countryCitiesNumber = country.getCitiesNumber();
          uint32_t countryLocationsNumber = country.getLocationsNumber();

          /* Creates the identifier for the node id of the new Country object
          The identifier for the node id of every Country object will be: Countries.CountryCode */
          std::string countryObjNameId = static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID) + "." + countryCode;
          /* Creates a Country object node class of the folder type to containing some
          attributes/member variables and organizes all the locations objects under it. */
          UA_NodeId countryObjId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(countryObjNameId.c_str()));
          UA_ObjectAttributes countryObjAttr = UA_ObjectAttributes_default;
          char locale[] = "en-US";
          char countryObjAttrDesc[] = "Country object with attributes and locations information.";
          countryObjAttr.description = UA_LOCALIZEDTEXT(locale, countryObjAttrDesc);
          countryObjAttr.displayName = UA_LOCALIZEDTEXT(locale, const_cast<char*>(countryName.c_str()));
          UA_Server_addObjectNode(server, countryObjId, rootNodeId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, const_cast<char*>(countryName.c_str())),
            UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), countryObjAttr, NULL, NULL);

          std::string nameVarNameId = countryObjNameId + "." + CountryData::BROWSE_NAME;
          UA_NodeId nameVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(nameVarNameId.c_str()));
          UA_VariableAttributes nameVarAttr = UA_VariableAttributes_default;
          UA_String nameValue = UA_STRING(const_cast<char*>(countryName.c_str()));
          UA_Variant_setScalar(&nameVarAttr.value, &nameValue, &UA_TYPES[UA_TYPES_STRING]);
          char nameVarAttrDesc[] = "The name of a country";
          nameVarAttr.description = UA_LOCALIZEDTEXT(locale, nameVarAttrDesc);
          nameVarAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::BROWSE_NAME);
          UA_Server_addVariableNode(server, nameVarNodeId, countryObjId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, CountryData::BROWSE_NAME),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), nameVarAttr, NULL, NULL);

          std::string codeVarNameId = countryObjNameId + "." + CountryData::BROWSE_CODE;
          UA_NodeId codeVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(codeVarNameId.c_str()));
          UA_VariableAttributes codeVarAttr = UA_VariableAttributes_default;
          UA_String codeValue = UA_STRING(const_cast<char*>(countryCode.c_str()));
          UA_Variant_setScalar(&codeVarAttr.value, &codeValue, &UA_TYPES[UA_TYPES_STRING]);
          char codeVarAttrDesc[] = "2 letters ISO code representing the Country Name";
          codeVarAttr.description = UA_LOCALIZEDTEXT(locale, codeVarAttrDesc);
          codeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, CountryData::BROWSE_CODE);
          UA_Server_addVariableNode(server, codeVarNodeId, countryObjId,
            UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
            UA_QUALIFIEDNAME(WebService::OPC_NS_INDEX, CountryData::BROWSE_CODE),
            UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), codeVarAttr, NULL, NULL);

          std::string citiesNumberVarNameId = countryObjNameId + "." + CountryData::BROWSE_CITIES_NUMBER;
          UA_NodeId citiesNumberVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(citiesNumberVarNameId.c_str()));
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
          UA_NodeId locationsNumberVarNodeId = UA_NODEID_STRING(WebService::OPC_NS_INDEX,
            const_cast<char*>(locationsNumberVarNameId.c_str()));
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

          country.setIsInitialized(true);
        }
        }).wait();
    }
    catch (const std::exception & e) { //TODO - catch more specific type of exception
      UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK, "Error on requestCountries method: [%s]",  e.what());
    }
  }

  /*
  Add root "Countries" object node in the information model and request other countries to be added as childs.
  @param server - our OPC UA server where these objects will be added in the information model.
  */
  static void addCountries(UA_Server* server) {

    // This will control how many times this method executes. It's to be executed ONLY ONCE!
    static bool wasItCalled = false;

    // If was not executed yet, add the countries objects. Otherwise, do nothing.
    if (!wasItCalled) {
      wasItCalled = true;

      // Creates a Countries object node class of the folder type to organize all the locations objects under it.
      UA_NodeId countriesObjId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, CountryData::COUNTRIES_FOLDER_NODE_ID);
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
  Function pointer to UA_NodeMap_getNode function that is initialized in UA_ServerConfig_new_default() through UA_Nodestore.
  This default function needs to be called from our customGetNode function.
  */
  const UA_Node* (*defaultGetNode)(void* nodestoreContext, const UA_NodeId* nodeId);

  /*
  This function is called a lot of times, especially when browse requests from the client are send to the server.
  When client requests to read any specific country, locations for that country will be downloaded (for the 1st time).
  When client requests to read any specific location, weather data for that location will be downloaded (or updated).
  The default function from UA_Nodestore from the UA_ServerConfig needs to be returned.
  */
  const UA_Node* customGetNode(void* nodestoreContext, const UA_NodeId* nodeId) {
    // Within this function nodes are added dynamically at runtime.
    // Before adding new node, SDK checks if it exists, by calling getNode function: on those calls need to call original function.
    // Variable "processing" is used to make sure that when nodes are added original function is used.
    static bool processing = false;

    if (!processing) {
      processing = true;
      if (nodeId->identifierType == UA_NODEIDTYPE_STRING && nodeId->namespaceIndex == WebService::OPC_NS_INDEX) {
        size_t length = nodeId->identifier.string.length;
        UA_Byte* data = nodeId->identifier.string.data;
        std::string nodeIdName(reinterpret_cast<char*>(data), length);

        /*
        The NodeId is composed as : Countries.CountryCode.LocationName.WeatherVariable
        If length of node id is greater than 13, it means at least the client is requesting to read a specific country or its sub nodes.
        */
        if (length > 13) {
          std::string countryCode = nodeIdName.substr(10, 2);
          std::string countryObjNameId = static_cast<std::string>(CountryData::COUNTRIES_FOLDER_NODE_ID) + "." + countryCode;
          UA_NodeId countryObjId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(countryObjNameId.c_str()));
          // Search for the country in the list of countries of the web service.
          auto& allCountries = webService->getAllCountries();
          auto itCountry = allCountries.find(countryCode);
          if (itCountry != allCountries.end()) {
            auto& country = itCountry->second;

            // Only download locations if they don't exist and the country has been initialized (added to the address space).
            if (country.getIsInitialized()) {
              if (country.getLocations().size() == 0)
                requestLocations(webService->getServer(), country, countryObjId);

              // Only try to download weather data if locations has been added to the country being read.
              if (country.getLocations().size() > 0) {
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
                std::string locationName;

                /* If we don't find the dot on the first search, for sure that is not a request to read the location. */
                if (posDot != std::string::npos) {
                  /* The location name MAY be returned at position 13 until the first '.' after that. */
                  locationName = nodeIdName.substr(13, posDot - 13);

                  // Search for the location in the list of locations inside the country of the web service.
                  auto& locations = country.getLocations();
                  auto itLocation = locations.find(locationName);
                  /* While the location is not found, continue checking for dots within its name. */
                  while (itLocation == locations.end() && posDot != std::string::npos) {
                    posDot = nodeIdName.find(".", posDot + 1);
                    if (posDot != std::string::npos)
                      locationName = nodeIdName.substr(13, posDot - 13);
                    else
                      locationName = nodeIdName.substr(13);
                    itLocation = locations.find(locationName);
                  }
                  if (itLocation != locations.end()) {
                    auto& location = itLocation->second;
                    std::string locationObjNameId = countryObjNameId + "." + locationName;
                    /*
                    The location name was found correctly. Check if there are more characters after the location name comparing the full node id name's size to the node id until the location name.
                    */
                    if (nodeIdName.size() > locationObjNameId.size()) {
                      UA_NodeId locationObjId = UA_NODEID_STRING(WebService::OPC_NS_INDEX, const_cast<char*>(locationObjNameId.c_str()));
                      /*
                      Only try to download weather data if they not exist in the address space.
                      The isAddingWeatherToAddressSpace boolean variable in LocationData controls when we are adding the weather data in the OPC UA address space, that being said the requestWeather function bellow will not be called more than once.
                      */
                      if (location.getIsInitialized() && !(location.getHasBeenReceivedWeatherData()) && !(location.getIsAddingWeatherToAddressSpace())) {
                        requestWeather(webService->getServer(), location, locationObjId);
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      processing = false;
    }
    return defaultGetNode(nodestoreContext, nodeId);
  }
}

int main(int argc, char* argv[]) {

  std::string settingsPath;

  if (argc > 1) {
    settingsPath = argv[1];
    std::cout << "Accepted custom settings file path as a parameter: " << settingsPath << std::endl;
  }
  else {
    //default settings file location - executable directory
    std::string fullPath(argv[0]);
    std::string execDir = fullPath.substr(0, fullPath.find_last_of("/\\") + 1); //including separator since we are adding a file name
    std::cout << "Executable directory: " << execDir << std::endl;
    settingsPath = execDir + "settings.json";
    std::cout << "Using default settings file: " << settingsPath << std::endl;
  }

  /*
    Assign default values to variables.
    Attempt to open "settings.json" file to get Dark Sky API key and other settings to override dafault values.
    If there is a problem opening file, parsing or Dark Sky API key seems to be invalid - terminate the program.
  */
  weatherserver::Settings settings(settingsPath);
  if (!settings.areValid()) {
    std::cout << "Invalid settings. Terminating..." << std::endl;
    return EXIT_FAILURE;
  }

  weatherserver::WebService ws(settings);

  custom_port_number = settings.port_number;
  if (!settings.endpointUrl.empty())
    custom_endpoint_url = settings.endpointUrl.c_str();

  webService = &ws;

  signal(SIGINT, stopHandler);
  signal(SIGTERM, stopHandler);

  UA_ServerConfig* config = UA_ServerConfig_new_default();

  if (!settings.hostName.empty()) {
    UA_String ourHostName = UA_String_fromChars(settings.hostName.c_str());
    UA_ServerConfig_set_customHostname(config, ourHostName);
    UA_String_deleteMembers(&ourHostName);
  }

  weatherserver::defaultGetNode = config->nodestore.getNode;
  config->nodestore.getNode = weatherserver::customGetNode;

  UA_Server* server = UA_Server_new(config);

  webService->setServer(server);

  weatherserver::addCountries(server);

  UA_StatusCode retval = UA_Server_run(server, &running);

  UA_Server_delete(server);
  UA_ServerConfig_delete(config);

  return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
