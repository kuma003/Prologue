// ------------------------------------------------
// WindModel.hppの実装
// ------------------------------------------------

#include "WindModel.hpp"

#include <fstream>
#include <stdexcept>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "math/Algorithm.hpp"
#include "misc/Constant.hpp"

// --------------------------------------------------------------
// 大気モデルは国際標準大気参考にしている
// JP Wikipedia: https://ja.wikipedia.org/wiki/%E5%9B%BD%E9%9A%9B%E6%A8%99%E6%BA%96%E5%A4%A7%E6%B0%97
// EN Wikipedia: https://en.wikipedia.org/wiki/International_Standard_Atmosphere
//
// 実際に実装したモデル: https://pigeon-poppo.com/standard-atmosphere/
// 上記サイトが参照しているNASAの論文: https://ntrs.nasa.gov/citations/19770009539
// --------------------------------------------------------------
namespace Atmospehre {
    struct Layer {
        double baseTemperature;  // [°C]
        double lapseRate;        // [°C/m]
        double basePressure;     // [Pa]
        double baseDensity;      // [kg/m^3]
    };

    constexpr size_t LayerCount = 3;

    // !! Geopotential altitude
    // 0 ~ 11000 [m]    : Troposphere
    // 11000 ~ 20000 [m]: Tropopause
    // 20000 ~ 32000 [m]: Stratosphere
    // 32000 ~ [m]      : Undefined and an error will occur if the altitude exceeds this
    constexpr double LayerThresholds[LayerCount + 1] = {0, 11000, 20000, 32000};

    // 各層におけるパラメータ
    const Layer Layers[LayerCount] = {
        {.baseTemperature = AppSetting::Atmosphere::baseTemperature,
         .lapseRate       = -6.5e-3,
         .basePressure    = AppSetting::Atmosphere::basePressure,
         .baseDensity     = 1.2985},
        {.baseTemperature = -56.5, .lapseRate = 0.0e-3, .basePressure = 22632.064, .baseDensity = 0.3639},
        {.baseTemperature = -76.5, .lapseRate = 1.0e-3, .basePressure = 5474.889, .baseDensity = 0.0880}};

    namespace Wind {
        constexpr double GeostrophicWind = 15;  // 地衡風 [m/s]

        constexpr double SurfaceLayerLimit = 300;  // 接地境界層 0 ~ 300 [m]

        // --------------------------------------------------------------
        // エクマン層
        // Wikipedia: https://ja.wikipedia.org/wiki/%E3%82%A8%E3%82%AF%E3%83%9E%E3%83%B3%E5%A2%83%E7%95%8C%E5%B1%A4
        // From: http://kishou.u-gakugei.ac.jp/graduate/local/doc04.pdf
        // --------------------------------------------------------------
        constexpr double EkmanLayerLimit = 1000;  // エクマン層 300 ~ 1000 [m]
    }
}

// 0未満または360以上の角度を0 ~ 360におさめる
double normalizeAngle(double angle) {
    if (0 <= angle && angle < 360) {
        return angle;
    } else if (-360 <= angle && angle < 0) {
        return 360 + angle;
    } else if (angle >= 360) {
        return angle - 360 * static_cast<int>(std::floor(angle / 360));
    } else if (angle < -360) {
        return angle - 360 * static_cast<int>(std::floor(angle / 360));
    } else {
        CommandLine::PrintInfo(PrintInfoType::Warning, "Unhandled angle " + std::to_string(angle));
        return angle;
    }
}

double applyPowerLow(double windSpeed, double height) {
    return windSpeed
           * pow(height / AppSetting::WindModel::powerLowBaseAltitude, 1.0 / AppSetting::WindModel::powerConstant);
}

Vector3D applyPowerLow(const Vector3D& wind, double height) {
    return wind * pow(height / AppSetting::WindModel::powerLowBaseAltitude, 1.0 / AppSetting::WindModel::powerConstant);
}

WindModel::WindModel(double groundWindSpeed, double groundWindDirection, double magneticDeclination) :
    m_groundWindSpeed(groundWindSpeed),
    m_groundWindDirection(normalizeAngle(groundWindDirection + magneticDeclination)) {
    m_directionInterval = 270 - m_groundWindDirection;
    if (m_directionInterval <= -45.0) {
        m_directionInterval = 270 - m_groundWindDirection + 360;
    }
}

WindModel::WindModel(double magneticDeclination) : m_groundWindSpeed(0.0), m_groundWindDirection(0.0) {
    std::fstream windfile("input/wind/" + AppSetting::WindModel::realdataFilename);

    char header[1024];
    windfile.getline(header, 1024);
    size_t i = 1;
    char c;
    std::string dummy;
    m_windData.push_back(WindData());
    while (!windfile.eof()) {
        m_windData.push_back(WindData());
        windfile >> m_windData[i].height >> c >> m_windData[i].speed >> c >> m_windData[i].direction;
        if (m_windData[i] == WindData()) {
            break;
        }
        i++;
    }
    if (m_windData[m_windData.size() - 1] == WindData()) {
        m_windData.pop_back();
    }

    windfile.close();

    for (auto& wind : m_windData) {
        wind.direction += magneticDeclination;
    }
}

void WindModel::update(double height) {
    m_height = height;

    m_geopotentialHeight = getGeopotentialHeight();
    m_gravity            = getGravity();
    m_temperature        = getTemperature();
    m_pressure           = getPressure();
    m_airDensity         = getAirDensity();

    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        m_wind = getWindFromData();
        break;

    case WindModelType::Original:
        m_wind = getWindOriginalModel();
        break;

    case WindModelType::OnlyPowerLow:
        m_wind = getWindOnlyPowerLow();
        break;
    case WindModelType::NoWind:
        m_wind = Vector3D(0, 0, 0);
        break;
    }
}

// Geopotential Height:
// https://ja.wikipedia.org/wiki/%E3%82%B8%E3%82%AA%E3%83%9D%E3%83%86%E3%83%B3%E3%82%B7%E3%83%A3%E3%83%AB
// Formula from: https://pigeon-poppo.com/standard-atmosphere/#i-2
double WindModel::getGeopotentialHeight() {
    return Constant::EarthRadius * m_height / (Constant::EarthRadius + m_height);
}

// Formula from: https://ja.wikipedia.org/wiki/%E5%9C%B0%E7%90%83%E3%81%AE%E9%87%8D%E5%8A%9B#%E9%AB%98%E5%BA%A6
double WindModel::getGravity() {
    const double k = (Constant::EarthRadius / (Constant::EarthRadius + m_height));
    return Constant::G * k * k;
}

// Formula from: https://pigeon-poppo.com/standard-atmosphere/#i-3
double WindModel::getTemperature() {
    for (size_t i = 0; i < Atmospehre::LayerCount; i++) {
        if (m_geopotentialHeight < Atmospehre::LayerThresholds[i + 1]) {
            const auto& layer = Atmospehre::Layers[i];
            return layer.baseTemperature + layer.lapseRate * m_geopotentialHeight;
        }
    }

    throw std::runtime_error{"Current height is " + std::to_string(m_height) + " m. "
                             + "Wind model is not defined above 32000 m."};
}

// Formula from: https://keisan.casio.jp/exec/system/1203469826
// はじめに記載した参考文献はおそらく間違っている
// https://pigeon-poppo.com/standard-atmosphere/#i-4
double WindModel::getPressure() {
    for (size_t i = 0; i < Atmospehre::LayerCount; i++) {
        if (m_geopotentialHeight <= Atmospehre::LayerThresholds[i + 1]) {
            const auto k = Atmospehre::Layers[i].lapseRate * m_height;
            return Atmospehre::Layers[i].basePressure
                   * pow(1 + k / (m_temperature - Constant::AbsoluteZero - k), 5.257);
        }
    }

    throw std::runtime_error{"Current height is " + std::to_string(m_height) + " m. "
                             + "Wind model is not defined above 32000 m."};
}

// Formula from: https://pigeon-poppo.com/standard-atmosphere/#i-5
double WindModel::getAirDensity() {
    return m_pressure / ((m_temperature - Constant::AbsoluteZero) * Constant::GasConstant);
}

Vector3D WindModel::getWindFromData() {
    size_t index = 0;
    for (size_t i = 0; i < m_windData.size(); i++) {
        if (m_height > m_windData[i].height) {
            index++;
        }
    }

    if (index == 0) {
        return Vector3D(0, 0, 0);
    }

    const auto& windData1 = m_windData[index - 1];
    const auto& windData2 = m_windData[index];

    const auto windSpeed =
        Algorithm::Lerp(m_height, windData1.height, windData2.height, windData1.speed, windData2.speed);
    const auto direction =
        Algorithm::Lerp(m_height, windData1.height, windData2.height, windData1.direction, windData2.direction);

    const double rad = direction * Constant::PI / 180;

    return -Vector3D(sin(rad), cos(rad), 0) * windSpeed;
}

Vector3D WindModel::getWindOriginalModel() {
    if (m_height <= 0) {
        const double rad          = m_groundWindDirection * Constant::PI / 180;
        const Vector3D groundWind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return groundWind;
    }

    if (m_height < Atmospehre::Wind::SurfaceLayerLimit) {  // Surface layer
        const double deltaDirection = m_height / Atmospehre::Wind::EkmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;
        const Vector3D wind         = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return applyPowerLow(wind, m_height);
    } else if (m_height < Atmospehre::Wind::EkmanLayerLimit) {  // Ekman layer
        const double deltaDirection = m_height / Atmospehre::Wind::EkmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;

        const double borderWindSpeed = applyPowerLow(m_groundWindSpeed, m_height);

        const double k =
            (m_height - Atmospehre::Wind::SurfaceLayerLimit) / (Atmospehre::Wind::SurfaceLayerLimit * sqrt(2));
        const double u = Atmospehre::Wind::GeostrophicWind * (1 - exp(-k) * cos(k));
        const double v = Atmospehre::Wind::GeostrophicWind * exp(-k) * sin(k);

        const double descentRate = ((Atmospehre::Wind::GeostrophicWind - u) / Atmospehre::Wind::GeostrophicWind);

        return -Vector3D(sin(rad), cos(rad), 0) * borderWindSpeed * descentRate
               - Vector3D(u * sin(rad), u * cos(rad), v);
    } else {  // Free atomosphere
        return Vector3D(Atmospehre::Wind::GeostrophicWind, 0, 0);
    }
}

Vector3D WindModel::getWindOnlyPowerLow() {
    if (m_height <= 0) {
        const double rad = m_groundWindDirection * Constant::PI / 180;
        return -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;
    } else {
        const double rad    = m_groundWindDirection * Constant::PI / 180;
        const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return applyPowerLow(wind, m_height);
    }
}
