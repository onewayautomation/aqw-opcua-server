#include "WebService.h"

using namespace weathersvr;

const uint16_t WebService::OPC_NS_INDEX = 1;
const utility::string_t WebService::ENDPOINT_API_OPENAQ = U("https://api.openaq.org/v1/");;
const utility::string_t WebService::PATH_API_OPENAQ_COUNTRIES = U("countries");
const utility::string_t WebService::PATH_API_OPENAQ_LOCATIONS = U("locations");
const utility::string_t WebService::PATH_API_OPENAQ_MEASUREMENTS = U("measurements");
const utility::string_t WebService::PARAM_API_OPENAQ_COUNTRY = U("country");

const utility::string_t WebService::ENDPOINT_API_DARKSKY = U("https://api.darksky.net/forecast");
// TODO: Replace the value of this constant with your DarkSky API Key
const utility::string_t WebService::KEY_API_DARKSKY = U("");
const utility::string_t WebService::PARAM_API_DARKSKY_EXCLUDE = U("exclude");
const utility::string_t WebService::PARAM_API_DARKSKY_UNITS = U("units");
const std::string WebService::PARAM_VALUE_API_DARKSKY_MINUTELY = "minutely";
const std::string WebService::PARAM_VALUE_API_DARKSKY_HOURLY = "hourly";
const std::string WebService::PARAM_VALUE_API_DARKSKY_DAILY = "daily";
// TODO: Change the interval in minutes to control the download of weather data.
const short WebService::INTERVAL_DOWNLOAD_WEATHER_DATA = 15;

pplx::task<web::json::value> WebService::fetchAllCountries() {

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

pplx::task<web::json::value> WebService::fetchAllLocations(const std::string& countryName) {
	web::uri_builder uriBuilder(ENDPOINT_API_OPENAQ);
	uriBuilder.append_path(PATH_API_OPENAQ_LOCATIONS);
	uriBuilder.append_query(PARAM_API_OPENAQ_COUNTRY, utility::conversions::to_string_t(countryName));

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
	uriBuilder.append_path(KEY_API_DARKSKY);
	uriBuilder.append_path(utility::conversions::to_string_t(coordinatesPath));
	uriBuilder.append_query(WebService::PARAM_API_DARKSKY_EXCLUDE, 
		utility::conversions::to_string_t(excludeQuery));
	uriBuilder.append_query(WebService::PARAM_API_DARKSKY_UNITS, U("si"));

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

void WebService::setAllCountries(const std::vector<CountryData>& allCountries) {
	fetchedAllCountries = allCountries;
}
