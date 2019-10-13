#include "WeatherData.h"

namespace weatherserver {

  const utility::string_t WeatherData::KEY_LATITUDE = U("latitude");
  const utility::string_t WeatherData::KEY_LONGITUDE = U("longitude");
  const utility::string_t WeatherData::KEY_TIMEZONE = U("timezone");
  const utility::string_t WeatherData::KEY_ICON = U("icon");
  const utility::string_t WeatherData::KEY_TEMPERATURE = U("temperature");
  const utility::string_t WeatherData::KEY_APARENT_TEMPERATURE = U("apparentTemperature");
  const utility::string_t WeatherData::KEY_HUMIDIY = U("humidity");
  const utility::string_t WeatherData::KEY_PRESSURE = U("pressure");
  const utility::string_t WeatherData::KEY_WINDSPEED = U("windSpeed");
  const utility::string_t WeatherData::KEY_WINDBEARING = U("windBearing");
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
  char WeatherData::BROWSE_WIND_BEARING[] = "WindBearing";
  char WeatherData::BROWSE_CLOUD_COVER[] = "CloudCover";

  WeatherData::WeatherData(double latitude, double longitude, std::string timezone, std::string icon, double temperature,
    double apparentTemperature, double humidity, double pressure, double windSpeed, double windBearing, double cloudCover)
    : latitude { latitude },
      longitude { longitude },
      timezone { timezone },
      icon { icon },
      temperature { temperature },
      apparentTemperature { apparentTemperature },
      pressure { pressure },
      humidity { humidity },
      windSpeed { windSpeed },
      windBearing { windBearing },
      cloudCover { cloudCover } {}

  WeatherData::WeatherData()
    : WeatherData{ 0, 0, "", "", 0, 0, 0, 0, 0, 0, 0 } {}

  WeatherData WeatherData::parseJson(web::json::value& json) {

    double latitude = 999;
    double longitude = 999;
    std::string timezone;
    std::string icon;
    double temperature = 0;
    double apparentTemperature = 0;
    double pressure = 0;
    double humidity = 0;
    double windSpeed = 0;
    double windBearing = 0;
    double cloudCover = 0;

    try {
      latitude = json.at(KEY_LATITUDE).as_double();
      longitude = json.at(KEY_LONGITUDE).as_double();
      timezone = utility::conversions::to_utf8string(json.at(KEY_TIMEZONE).as_string()); // Converts from wstring to string

      auto currently = json.at(KEY_CURRENTLY);

      icon = utility::conversions::to_utf8string(currently.at(KEY_ICON).as_string());
      temperature = currently.at(KEY_TEMPERATURE).as_double();
      apparentTemperature = currently.at(KEY_APARENT_TEMPERATURE).as_double();
      pressure = currently.at(KEY_PRESSURE).as_double();
      humidity = currently.at(KEY_HUMIDIY).as_double();
      windSpeed = currently.at(KEY_WINDSPEED).as_double();

      // windBearing value is not returned if wind speed is 0.
      if (windSpeed > 0.001)
        windBearing = currently.at(KEY_WINDBEARING).as_double();
      else
        windBearing = 0;

      cloudCover = currently.at(KEY_CLOUD_COVER).as_double();
    }
    catch (const web::json::json_exception& ex) {

      std::cout << "Exception caught while parsing JSON object with weather data: " << ex.what() << std::endl
        << "latitude = " << latitude << std::endl
        << "longitude = " << longitude << std::endl
        << "timezone = " << timezone << std::endl
        << "icon = " << icon << std::endl
        << "temperature = " << temperature << std::endl
        << "apparent temperature = " << apparentTemperature << std::endl
        << "humidity = " << humidity << std::endl
        << "pressure = " << pressure << std::endl
        << "wind speed = " << windSpeed << std::endl
        << "wind bearing = " << windBearing << std::endl
        << "cloud cover = " << cloudCover << std::endl;
    }

    return WeatherData(latitude, longitude, timezone, icon, temperature, apparentTemperature,
      humidity, pressure, windSpeed, windBearing, cloudCover);
  }
}
