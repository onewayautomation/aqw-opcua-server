#include "open62541.h"

#include "WebService.h"
#include <signal.h>
#include <stdlib.h>
#include <thread>

UA_Boolean running = true;
static void stopHandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
	running = false;
}

/*
Request all the weather information of the location supplied by parameter from the web service.
Map these objects returned from the web service in OPC UA objects and put them available in the address space.
@param *server - The OPC UA server where the objects will be added in the address space view.
@param *webService - Service responsible for REST call on the internet.
@param &location - The LocationData object which the weather will be requested for.
@param UA_NodeId parentNodeId - The parent node id of the object where the variables will be added in OPC UA.
*/
static void requestWeather(UA_Server *server, weathersvr::WebService *webService
	, weathersvr::LocationData& location, UA_NodeId parentNodeId) {
	try {
		// Creates and define the location attributes/variable node class
		webService->fetchWeather(location.getLatitude(), location.getLongitude()).then([&](web::json::value response) {

			weathersvr::WeatherData weatherData = weathersvr::WeatherData::parseJson(response);

			/* Creates the identifier for the node id of the new variable node class
			The identifier for the node id of every variable will be: Countries.CountryCode.LocationName.Variable */
			std::string parentNameId = static_cast<std::string>(weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID)
				+ "." + location.getCountryCode() + "." + location.getName();
			std::string latitudeNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_LATITUDE;
			UA_NodeId latitudeVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				latitudeNameId.c_str());
			// Creates the variable node class attribute under the parent object.
			UA_VariableAttributes latitudeVarAttr = UA_VariableAttributes_default;
			UA_Double latitudeValue = weatherData.getLatitude();
			UA_Variant_setScalar(&latitudeVarAttr.value, &latitudeValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char locale[] = "en-US";
			char latitudeVarAttrDesc[] = "The latitude of a location (in decimal degrees). Positive is north, negative is south.";
			latitudeVarAttr.description = UA_LOCALIZEDTEXT(locale, latitudeVarAttrDesc);
			latitudeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_LATITUDE);
			UA_Server_addVariableNode(server, latitudeVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_LATITUDE),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), latitudeVarAttr, NULL, NULL);

			/* Creates the identifier for the node id of the new variable node class
			The identifier for the node id of every variable will be: Countries.CountryCode.LocationName.Variable */
			std::string longitudeNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_LONGITUDE;
			UA_NodeId longitudeVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				longitudeNameId.c_str());
			// Creates the variable node class attribute under the parent object.
			UA_VariableAttributes longitudeVarAttr = UA_VariableAttributes_default;
			UA_Double longitudeValue = weatherData.getLongitude();
			UA_Variant_setScalar(&longitudeVarAttr.value, &longitudeValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char longitudeVarAttrDesc[] = "The longitude of a location (in decimal degrees). Positive is east, negative is west.";
			longitudeVarAttr.description = UA_LOCALIZEDTEXT(locale, longitudeVarAttrDesc);
			longitudeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_LONGITUDE);
			UA_Server_addVariableNode(server, longitudeVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_LONGITUDE),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), longitudeVarAttr, NULL, NULL);

			std::string timezoneNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_TIMEZONE;
			UA_NodeId timezoneVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				timezoneNameId.c_str());
			UA_VariableAttributes timezoneVarAttr = UA_VariableAttributes_default;
			UA_String timezoneValue = UA_STRING_ALLOC(weatherData.getTimezone().c_str());
			UA_Variant_setScalar(&timezoneVarAttr.value, &timezoneValue, &UA_TYPES[UA_TYPES_STRING]);
			char timezoneVarAttrDesc[] = "The IANA timezone name for the requested location.";
			timezoneVarAttr.description = UA_LOCALIZEDTEXT(locale, timezoneVarAttrDesc);
			timezoneVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_TIMEZONE);
			UA_Server_addVariableNode(server, timezoneVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_TIMEZONE),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), timezoneVarAttr, NULL, NULL);

			std::string iconNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_ICON;
			UA_NodeId iconVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				iconNameId.c_str());
			UA_VariableAttributes iconVarAttr = UA_VariableAttributes_default;
			UA_String iconValue = UA_STRING_ALLOC(weatherData.getCurrentlyIcon().c_str());
			UA_Variant_setScalar(&iconVarAttr.value, &iconValue, &UA_TYPES[UA_TYPES_STRING]);
			char iconVarAttrDesc[] = "A machine-readable text icon of this data point, suitable for selecting an icon for display.";
			iconVarAttr.description = UA_LOCALIZEDTEXT(locale, iconVarAttrDesc);
			iconVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_ICON);
			UA_Server_addVariableNode(server, iconVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_ICON),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), iconVarAttr, NULL, NULL);

			std::string temperatureNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_TEMPERATURE;
			UA_NodeId temperatureVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				temperatureNameId.c_str());
			UA_VariableAttributes temperatureVarAttr = UA_VariableAttributes_default;
			UA_Double temperatureValue = weatherData.getCurrentlyTemperature();
			UA_Variant_setScalar(&temperatureVarAttr.value, &temperatureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char temperatureVarAttrDesc[] = "The air temperature in degrees Celsius (if units=si during request) or Fahrenheit.";
			temperatureVarAttr.description = UA_LOCALIZEDTEXT(locale, temperatureVarAttrDesc);
			temperatureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_TEMPERATURE);
			UA_Server_addVariableNode(server, temperatureVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_TEMPERATURE),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), temperatureVarAttr, NULL, NULL);

			std::string apparentTemperatureNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_APPARENT_TEMPERATURE;
			UA_NodeId apparentTemperatureVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				apparentTemperatureNameId.c_str());
			UA_VariableAttributes apparentTemperatureVarAttr = UA_VariableAttributes_default;
			UA_Double apparentTemperatureValue = weatherData.getCurrentlyApparentTemperature();
			UA_Variant_setScalar(&apparentTemperatureVarAttr.value, &apparentTemperatureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char apparentTemperatureVarAttrDesc[] = "The apparent (or “feels like”) temperature in degrees Celsius (if units=si during request) or Fahrenheit.";
			apparentTemperatureVarAttr.description = UA_LOCALIZEDTEXT(locale, apparentTemperatureVarAttrDesc);
			apparentTemperatureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_APPARENT_TEMPERATURE);
			UA_Server_addVariableNode(server, apparentTemperatureVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_APPARENT_TEMPERATURE),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), apparentTemperatureVarAttr, NULL, NULL);

			std::string humidityNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_HUMIDITY;
			UA_NodeId humidityVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				humidityNameId.c_str());
			UA_VariableAttributes humidityVarAttr = UA_VariableAttributes_default;
			UA_Double humidityValue = weatherData.getCurrentlyHumidity();
			UA_Variant_setScalar(&humidityVarAttr.value, &humidityValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char humidityVarAttrDesc[] = "The relative humidity, between 0 and 1, inclusive.";
			humidityVarAttr.description = UA_LOCALIZEDTEXT(locale, humidityVarAttrDesc);
			humidityVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_HUMIDITY);
			UA_Server_addVariableNode(server, humidityVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_HUMIDITY),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), humidityVarAttr, NULL, NULL);

			std::string pressureNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_PRESSURE;
			UA_NodeId pressureVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				pressureNameId.c_str());
			UA_VariableAttributes pressureVarAttr = UA_VariableAttributes_default;
			UA_Double pressureValue = weatherData.getCurrentlyPressure();
			UA_Variant_setScalar(&pressureVarAttr.value, &pressureValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char pressureVarAttrDesc[] = "The sea-level air pressure in Hectopascals (if units=si during request) or millibars.";
			pressureVarAttr.description = UA_LOCALIZEDTEXT(locale, pressureVarAttrDesc);
			pressureVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_PRESSURE);
			UA_Server_addVariableNode(server, pressureVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_PRESSURE),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), pressureVarAttr, NULL, NULL);

			std::string windSpeedNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_WIND_SPEED;
			UA_NodeId windSpeedVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				windSpeedNameId.c_str());
			UA_VariableAttributes windSpeedVarAttr = UA_VariableAttributes_default;
			UA_Double windSpeedValue = weatherData.getCurrentlyWindSpeed();
			UA_Variant_setScalar(&windSpeedVarAttr.value, &windSpeedValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char windSpeedVarAttrDesc[] = "The wind speed in meters per second (if units=si during request) or miles per hour.";
			windSpeedVarAttr.description = UA_LOCALIZEDTEXT(locale, windSpeedVarAttrDesc);
			windSpeedVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_WIND_SPEED);
			UA_Server_addVariableNode(server, windSpeedVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_WIND_SPEED),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), windSpeedVarAttr, NULL, NULL);

			std::string cloudCoverNameId = parentNameId + "." + weathersvr::WeatherData::BROWSE_CLOUD_COVER;
			UA_NodeId cloudCoverVarNodeId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
				cloudCoverNameId.c_str());
			UA_VariableAttributes cloudCoverVarAttr = UA_VariableAttributes_default;
			UA_Double cloudCoverValue = weatherData.getCurrentlyCloudCover();
			UA_Variant_setScalar(&cloudCoverVarAttr.value, &cloudCoverValue, &UA_TYPES[UA_TYPES_DOUBLE]);
			char cloudCoverVarAttrDesc[] = "The percentage of sky occluded by clouds, between 0 and 1, inclusive.";
			cloudCoverVarAttr.description = UA_LOCALIZEDTEXT(locale, cloudCoverVarAttrDesc);
			cloudCoverVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::WeatherData::BROWSE_CLOUD_COVER);
			UA_Server_addVariableNode(server, cloudCoverVarNodeId, parentNodeId,
				UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
				UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::WeatherData::BROWSE_CLOUD_COVER),
				UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), cloudCoverVarAttr, NULL, NULL);

			/*std::cout << std::fixed << std::setprecision(6)
				<< "Latitude: " << weatherData.getLatitude()
				<< " / Longitude: " << weatherData.getLatitude()
				<< " / Timezone: " << weatherData.getTimezone()
				<< " / Summary: " << weatherData.getCurrentlyIcon()
				<< " / Temperature: " << weatherData.getCurrentlyTemperature()
				<< " / Apparent Temperature: " << weatherData.getCurrentlyApparentTemperature()
				<< " / Humidity: " << weatherData.getCurrentlyHumidity()
				<< " / Pressure: " << weatherData.getCurrentlyPressure()
				<< " / Wind Speed: " << weatherData.getCurrentlyWindSpeed()
				<< " / Cloud Cover: " << weatherData.getCurrentlyCloudCover()
				<< std::endl;*/
		}).wait();
	} catch (...) {
		UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
			"Error during request to the web service!");
	}
}

/*
Request all the Locations objects from the web service and its subcomponents.
Map these objects returned from the web service in OPC UA objects and put them available in the address space.
@param *server - The OPC UA server where the objects will be added in the address space view.
@param *webService - Service responsible for REST call on the internet.
@param &country - The CountryData object which the locations will be requested for. This object will be modified
in this function. Locations will be added to it.
@param UA_NodeId parentNodeId - The parent node id of the object where the variables will be added in OPC UA.
*/
static void requestLocations(UA_Server *server, weathersvr::WebService *webService
	, weathersvr::CountryData& country, UA_NodeId parentNodeId) {
	// This will control how many times this method executes. It's to be executed ONLY ONCE!
	static bool wasitCalled = false;

	// If was not executed yet, call the web request.
	if (!wasitCalled) {
		/* TODO: 
		Setting to true avoid to be executed again. This needs to be removed after the tests are done. 
		With this variable set to true here we only request all the locations and weather information for the first country.
		*/
		wasitCalled = true;

		try {
			std::vector<weathersvr::LocationData> allLocations;
			webService->fetchAllLocations(country.getCode()).then([&](web::json::value response) {
				allLocations = weathersvr::LocationData::parseJsonArray(response);
				country.setLocations(allLocations);

				for (size_t i {0}; i < allLocations.size(); i++) {
					std::string locationName = allLocations.at(i).getName();
					std::string locationCity = allLocations.at(i).getCity();
					std::string locationCountryCode = allLocations.at(i).getCountryCode();
					double locationLatitude = allLocations.at(i).getLatitude();
					double locationLongitude = allLocations.at(i).getLongitude();
					/* Creates the identifier for the node id of the new Location object
					The identifier for the node id of every location object will be: Countries.CountryCode.LocationName */
					std::string countries {weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID};
					std::string locationNameId =
						static_cast<std::string>(weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID)
						+ "." + locationCountryCode + "." + locationName;
					/* Creates an Location object node containing all the weather information related to it. */
					UA_NodeId locationObjId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX,
						locationNameId.c_str());
					UA_ObjectAttributes locationObjAttr = UA_ObjectAttributes_default;
					char locale[] = "en-US";
					char desc[] = "Location object containing weather information";
					locationObjAttr.description = UA_LOCALIZEDTEXT(locale, desc);
					locationObjAttr.displayName = UA_LOCALIZEDTEXT_ALLOC(locale, locationName.c_str());
					UA_Server_addObjectNode(server, locationObjId, parentNodeId,
						UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
						UA_QUALIFIEDNAME_ALLOC(weathersvr::WebService::OPC_NS_INDEX, locationName.c_str()),
						UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE), locationObjAttr, NULL, NULL);

					requestWeather(server, webService, allLocations.at(i), locationObjId);
				}
			}).wait();
		} catch (...) {
			UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
				"Error during request to the web service!");
		}
	}
}

/*
Request all the Countries objects from the web service and its subcomponents.
Map these objects returned from the web service in OPC UA objects and put them available in the address space.
@param *server - The OPC UA server where the objects will be added in the address space view.
@param *webService - Service responsible for REST call on the internet.
*/
static void requestCountries(UA_Server *server, weathersvr::WebService *webService) {
	// This will control how many times this method executes. It's to be executed ONLY ONCE!
	static bool wasitCalled = false;

	// If was not executed yet, call the web request.
	if (!wasitCalled) {
		wasitCalled = true;

		// Creates an Countries object node class of the folder type to organizes all the locations objects under it.
		UA_NodeId countriesObjId = UA_NODEID_STRING(weathersvr::WebService::OPC_NS_INDEX,
			weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID);
		UA_ObjectAttributes countriesObjAttr = UA_ObjectAttributes_default;
		char locale[] = "en-US";
		char countriesObjAttrDesc[] = "Organizes all the Countries object with their respective information";
		countriesObjAttr.description = UA_LOCALIZEDTEXT(locale, countriesObjAttrDesc);
		countriesObjAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID);
		UA_Server_addObjectNode(server, countriesObjId,
			UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
			UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
			UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID),
			UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), countriesObjAttr, NULL, NULL);

		try {
			std::vector<weathersvr::CountryData> allCountries;
			webService->fetchAllCountries().then([&](web::json::value response) {
				allCountries = weathersvr::CountryData::parseJsonArray(response);
				webService->setAllCountries(allCountries);

				for (size_t i {0}; i < allCountries.size(); i++) {
					std::string countryName = allCountries.at(i).getName();
					std::string countryCode = allCountries.at(i).getCode();
					uint32_t countryCitiesNumber = allCountries.at(i).getCitiesNumber();
					uint32_t countryLocationsNumber = allCountries.at(i).getLocationsNumber();

					/* Creates the identifier for the node id of the new Country object
					The identifier for the node id of every Country object will be: Countries.CountryCode */
					std::string countryObjNameId =
						static_cast<std::string>(weathersvr::CountryData::COUNTRIES_FOLDER_NODE_ID) + "." + countryCode;
					/* Creates an Country object node class of the folder type to containing some
					attributes/member variables and organizes all the locations objects under it. */
					UA_NodeId countryObjId = UA_NODEID_STRING_ALLOC(weathersvr::WebService::OPC_NS_INDEX, countryObjNameId.c_str());
					UA_ObjectAttributes countryObjAttr = UA_ObjectAttributes_default;
					char locale[] = "en-US";
					char countryObjAttrDesc[] = "Country object with attributes and locations information.";
					countryObjAttr.description = UA_LOCALIZEDTEXT(locale, countryObjAttrDesc);
					countryObjAttr.displayName = UA_LOCALIZEDTEXT_ALLOC(locale, countryName.c_str());
					UA_Server_addObjectNode(server, countryObjId, countriesObjId,
						UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
						UA_QUALIFIEDNAME_ALLOC(weathersvr::WebService::OPC_NS_INDEX, countryName.c_str()),
						UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), countryObjAttr, NULL, NULL);

					requestLocations(server, webService, allCountries.at(i), countryObjId);

					UA_VariableAttributes nameVarAttr = UA_VariableAttributes_default;
					UA_String nameValue = UA_STRING_ALLOC(countryName.c_str());
					UA_Variant_setScalar(&nameVarAttr.value, &nameValue, &UA_TYPES[UA_TYPES_STRING]);
					char nameVarAttrDesc[] = "The name of a country";
					nameVarAttr.description = UA_LOCALIZEDTEXT(locale, nameVarAttrDesc);
					nameVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::CountryData::BROWSE_NAME);
					UA_Server_addVariableNode(server, UA_NODEID_NULL, countryObjId,
						UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
						UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::CountryData::BROWSE_NAME),
						UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), nameVarAttr, NULL, NULL);

					UA_VariableAttributes codeVarAttr = UA_VariableAttributes_default;
					UA_String codeValue = UA_STRING_ALLOC(countryCode.c_str());
					UA_Variant_setScalar(&codeVarAttr.value, &codeValue, &UA_TYPES[UA_TYPES_STRING]);
					char codeVarAttrDesc[] = "2 letters ISO code representing the Country Name";
					codeVarAttr.description = UA_LOCALIZEDTEXT(locale, codeVarAttrDesc);
					codeVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::CountryData::BROWSE_CODE);
					UA_Server_addVariableNode(server, UA_NODEID_NULL, countryObjId,
						UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
						UA_QUALIFIEDNAME(weathersvr::WebService::OPC_NS_INDEX, weathersvr::CountryData::BROWSE_CODE),
						UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), codeVarAttr, NULL, NULL);

					//UA_VariableAttributes citiesNumberVarAttr = UA_VariableAttributes_default;
					//UA_UInt32 citiesNumberValue = countryCitiesNumber;
					//UA_Variant_setScalar(&citiesNumberVarAttr.value, &citiesNumberValue, &UA_TYPES[UA_TYPES_UINT32]);
					//char citiesNumberVarAttrDesc[] = "Number of cities belonged to a country. It can be city or province";
					//citiesNumberVarAttr.description = UA_LOCALIZEDTEXT(locale, citiesNumberVarAttrDesc);
					//citiesNumberVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::CountryData::BROWSE_CITIES_NUMBER);
					//UA_Server_addVariableNode(server, UA_NODEID_NULL, countryObjId,
					//	UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
					//	UA_QUALIFIEDNAME(1, weathersvr::CountryData::BROWSE_CITIES_NUMBER),
					//	UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), citiesNumberVarAttr, NULL, NULL);

					//UA_VariableAttributes locationsNumberVarAttr = UA_VariableAttributes_default;
					//UA_UInt32 locationsNumberValue = countryLocationsNumber;
					//UA_Variant_setScalar(&locationsNumberVarAttr.value, &locationsNumberValue, &UA_TYPES[UA_TYPES_UINT32]);
					//char locationsNumberVarAttrDesc[] = "Number of cities belonged to a country. It can be city or province";
					//locationsNumberVarAttr.description = UA_LOCALIZEDTEXT(locale, locationsNumberVarAttrDesc);
					//locationsNumberVarAttr.displayName = UA_LOCALIZEDTEXT(locale, weathersvr::CountryData::BROWSE_LOCATIONS_NUMBER);
					//UA_Server_addVariableNode(server, UA_NODEID_NULL, countryObjId,
					//	UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
					//	UA_QUALIFIEDNAME(1, weathersvr::CountryData::BROWSE_LOCATIONS_NUMBER),
					//	UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), locationsNumberVarAttr, NULL, NULL);
				}
			}).wait();
		} catch (...) {
			UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_NETWORK,
				"Error during request to the web service!");
		}
	}
}

int main() {
	signal(SIGINT, stopHandler);
	signal(SIGTERM, stopHandler);

	UA_ServerConfig *config = UA_ServerConfig_new_default();
	UA_Server *server = UA_Server_new(config);
	weathersvr::WebService ws;
	weathersvr::WebService* webService = &ws;

	/* Should the server networklayer block (with a timeout) until a message
	arrives or should it return immediately? */
	UA_Boolean waitInternal = false;

	UA_StatusCode retval = UA_Server_run_startup(server);
	if (retval != UA_STATUSCODE_GOOD)
		return 1;

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
		requestCountries(server, webService);

		/* Now we can use the max timeout to do something else. In this case, we just sleep. */
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	retval = UA_Server_run_shutdown(server);
	UA_Server_delete(server);
	UA_ServerConfig_delete(config);
	return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}