#include "WebService.h"

const uint16_t weathersvr::WebService::OPC_NS_INDEX = 1;
const utility::string_t weathersvr::WebService::ENDPOINT_API_OPENAQ = U("https://api.openaq.org/v1/");
const utility::string_t weathersvr::WebService::PATH_API_OPENAQ_COUNTRIES = U("countries");
const utility::string_t weathersvr::WebService::PATH_API_OPENAQ_LOCATIONS = U("locations");
const utility::string_t weathersvr::WebService::PATH_API_OPENAQ_MEASUREMENTS = U("measurements");
const utility::string_t weathersvr::WebService::PARAM_API_OPENAQ_COUNTRY = U("country");
const utility::string_t weathersvr::WebService::PARAM_API_OPENAQ_LIMIT = U("limit");

const utility::string_t weathersvr::WebService::ENDPOINT_API_DARKSKY = U("https://api.darksky.net/forecast");
const utility::string_t weathersvr::WebService::PARAM_API_DARKSKY_EXCLUDE = U("exclude");
const utility::string_t weathersvr::WebService::PARAM_API_DARKSKY_UNITS = U("units");
const std::string weathersvr::WebService::PARAM_VALUE_API_DARKSKY_MINUTELY = "minutely";
const std::string weathersvr::WebService::PARAM_VALUE_API_DARKSKY_HOURLY = "hourly";
const std::string weathersvr::WebService::PARAM_VALUE_API_DARKSKY_DAILY = "daily";

pplx::task<web::json::value> weathersvr::WebService::fetchAllCountries() {

	web::uri_builder uriBuilder(ENDPOINT_API_OPENAQ);
	uriBuilder.append_path(PATH_API_OPENAQ_COUNTRIES);

	web::http::client::http_client client(uriBuilder.to_string());
	return client.request(web::http::methods::GET)
		.then([](web::http::http_response requestResponse)
	{
		std::cout << "fetchAllCountries() request completed!" << std::endl;

		return requestResponse.extract_json();
	})
		.then([](web::json::value jsonValue)
	{
		std::cout << "JSON extracted from fetchAllCountries() completed!" << std::endl;
		auto results = jsonValue.at(U("results"));
		return results;
	});
}

pplx::task<web::json::value> weathersvr::WebService::fetchAllLocations(const std::string& countryName, const uint32_t limit) {
	web::uri_builder uriBuilder(ENDPOINT_API_OPENAQ);
	uriBuilder.append_path(PATH_API_OPENAQ_LOCATIONS);
	uriBuilder.append_query(PARAM_API_OPENAQ_COUNTRY, utility::conversions::to_string_t(countryName));
	uriBuilder.append_query(PARAM_API_OPENAQ_LIMIT, utility::conversions::to_string_t(std::to_string(limit)));

	web::http::client::http_client client(uriBuilder.to_string());
	return client.request(web::http::methods::GET)
		.then([](web::http::http_response requestResponse)
	{
		std::cout << "fetchAllLocations() request completed!" << std::endl;

		return requestResponse.extract_json();
	})
		.then([](web::json::value jsonValue)
	{
		std::cout << "JSON extracted from fetchAllLocations() completed!" << std::endl;
		auto results = jsonValue.at(U("results"));
		return results;
	});
}

pplx::task<web::json::value> weathersvr::WebService::fetchWeather(const double& latitude, const double& longitude) {
	std::string coordinatesPath = std::to_string(latitude) + "," + std::to_string(longitude);
	std::string excludeQuery = WebService::PARAM_VALUE_API_DARKSKY_MINUTELY 
		+ "," + WebService::PARAM_VALUE_API_DARKSKY_HOURLY 
		+ "," + WebService::PARAM_VALUE_API_DARKSKY_DAILY;

	web::uri_builder uriBuilder(ENDPOINT_API_DARKSKY);
	uriBuilder.append_path(settings.getKeyApiDarksky());
	uriBuilder.append_path(utility::conversions::to_string_t(coordinatesPath));
	uriBuilder.append_query(WebService::PARAM_API_DARKSKY_EXCLUDE, 
		utility::conversions::to_string_t(excludeQuery));
	uriBuilder.append_query(WebService::PARAM_API_DARKSKY_UNITS, settings.getUnits());

	web::http::client::http_client client(uriBuilder.to_string());
	return client.request(web::http::methods::GET)
		.then([](web::http::http_response requestResponse)
	{
		std::cout << "fetchWeather() request completed!" << std::endl;

		return requestResponse.extract_json();
	})
		.then([](web::json::value jsonValue)
	{
		std::cout << "JSON extracted from fetchWeather() completed!" << std::endl;
		return jsonValue;
	});
}

void weathersvr::WebService::setServer(UA_Server * uaServer) {
	server = uaServer;
}

void weathersvr::WebService::setSettings(const Settings& settingsObj) {
	settings = settingsObj;
}

void weathersvr::WebService::setAllCountries(const std::vector<CountryData>& allCountries) {
	fetchedAllCountries = allCountries;
}
