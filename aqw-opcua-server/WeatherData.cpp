#include "WeatherData.h"

using namespace weathersvr;

const utility::string_t WeatherData::KEY_LATITUDE = U("latitude");
const utility::string_t WeatherData::KEY_LONGITUDE = U("longitude");
const utility::string_t WeatherData::KEY_TIMEZONE = U("timezone");
const utility::string_t WeatherData::KEY_ICON = U("icon");
const utility::string_t WeatherData::KEY_TEMPERATURE = U("temperature");
const utility::string_t WeatherData::KEY_APARENT_TEMPERATURE = U("apparentTemperature");
const utility::string_t WeatherData::KEY_HUMIDIY = U("humidity");
const utility::string_t WeatherData::KEY_PRESSURE = U("pressure");
const utility::string_t WeatherData::KEY_WINDSPEED = U("windSpeed");
const utility::string_t WeatherData::KEY_CLOUD_COVER = U("cloudCover");
const utility::string_t WeatherData::KEY_CURRENTLY = U("currently");

char WeatherData::BROWSE_LATITUDE[] = "Latitude";
char WeatherData::BROWSE_LONGITUDE[] = "Longitude";
char WeatherData::BROWSE_TIMEZONE[] = "Timezone";
char WeatherData::BROWSE_ICON[] = "Icon";
char WeatherData::BROWSE_TEMPERATURE[] = "Temperature";
char WeatherData::BROWSE_APPARENT_TEMPERATURE[] = "ApparentTemperature";
char WeatherData::BROWSE_HUMIDITY[] = "Humidity";
char WeatherData::BROWSE_PRESSURE[] = "Pressure";
char WeatherData::BROWSE_WIND_SPEED[] = "WindSpeed";
char WeatherData::BROWSE_CLOUD_COVER[] = "CloudCover";

weathersvr::WeatherData::WeatherData(double latitude, double longitude, std::string timezone, std::string icon, 
	double temperature, double apparentTemperature, double humidity, double pressure, double windSpeed, double cloudCover)
	: latitude {latitude}, longitude {longitude}, timezone {timezone}, icon {icon},
	temperature {temperature}, apparentTemperature {apparentTemperature}, pressure {pressure}, 
	humidity {humidity}, windSpeed {windSpeed}, cloudCover {cloudCover}
{}

weathersvr::WeatherData::WeatherData()
	: WeatherData {0, 0, "", "", 0, 0, 0, 0, 0, 0}
{}

WeatherData weathersvr::WeatherData::parseJson(web::json::value & json) {
	double latitude = json.at(KEY_LATITUDE).as_double();
	double longitude = json.at(KEY_LONGITUDE).as_double();
	std::string timezone = utility::conversions::to_utf8string(json.at(KEY_TIMEZONE).as_string()); // Converts from wstring to string

	auto currently = json.at(KEY_CURRENTLY);		

	std::string icon = utility::conversions::to_utf8string(currently.at(KEY_ICON).as_string());
	double temperature = currently.at(KEY_TEMPERATURE).as_double();
	double apparentTemperature = currently.at(KEY_APARENT_TEMPERATURE).as_double();
	double pressure = currently.at(KEY_PRESSURE).as_double();
	double humidity = currently.at(KEY_HUMIDIY).as_double();
	double windSpeed = currently.at(KEY_WINDSPEED).as_double();
	double cloudCover = currently.at(KEY_CLOUD_COVER).as_double();

	return WeatherData(latitude, longitude, timezone, icon, temperature, apparentTemperature, 
		pressure, humidity, windSpeed, cloudCover);
}
