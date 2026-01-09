// ------------------------------------------------
// AppSetting.hppの実装
// prologue.settings.jsonの初期化に関わる
// ------------------------------------------------

#include "AppSetting.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <thread>

#include "CommandLine.hpp"
#include "utils/JsonUtils.hpp"

namespace AppSetting {
    // ------------------------------------------------
    // 初期化に関する諸関数の定義
    // ------------------------------------------------

    namespace Internal {
        bool initialized = false;
        boost::property_tree::ptree pt;

        template <typename T>
        T InitValue(const std::string& key) {
            if (!initialized) {
                boost::property_tree::read_json("prologue.settings.json", pt);
                initialized = true;
            }

            return JsonUtils::GetValueExc<T>(pt, key);
        }

		template <typename T>
		T initValueWithDefault(const std::string& key, const T& defaultValue) {
			if (!initialized) {
				boost::property_tree::read_json("prologue.settings.json", pt);
				initialized = true;
			}
			return JsonUtils::GetValueWithDefault<T>(pt, key, defaultValue);
		}

        size_t InitThreadCount() {
            const int count = Internal::InitValue<int>("processing.multi_thread_count");
            if (count < 1) {
                CommandLine::PrintInfo(PrintInfoType::Warning,
                                       "Specified thread count is too low",
                                       "Thread count is automatically set to 1.");
                return 1;
            }

            const size_t threadCountLimit = std::thread::hardware_concurrency();
            if (threadCountLimit == 0) {
                CommandLine::PrintInfo(PrintInfoType::Error,
                                       "Could not get hardware concurrency",
                                       "Thread count is automatically set to 1.");
                return 1;
            }

            if (static_cast<size_t>(count) > threadCountLimit) {
                CommandLine::PrintInfo(
                    PrintInfoType::Warning,
                    "Specified thread count exceeds the number that the machine can run.",
                    "Thread count is automatically set to " + std::to_string(threadCountLimit) + ".");
                return threadCountLimit;
            }

            return static_cast<size_t>(count);
        }

        WindModelType InitWindModelType() {
            const std::string windmodeltype = InitValue<std::string>("wind_model.type");

            if (windmodeltype == "real") {
                return WindModelType::Real;
            } else if (windmodeltype == "original") {
                return WindModelType::Original;
            } else if (windmodeltype == "only_powerlow") {
                return WindModelType::OnlyPowerLow;
            } else if (windmodeltype == "no_wind") {
                return WindModelType::NoWind;
            } else {
                CommandLine::PrintInfo(PrintInfoType::Error,
                                       "In prologue.settings.json",
                                       "wind_model.type",
                                       "\"" + windmodeltype + "\" is invalid string.",
                                       "Set \"real\", \"original\", \"only_powerlow\" or \"no_wind\"");
                throw 0;
            }
        }

        int InitResultPrecision() {
            int precision = Internal::InitValue<int>("result.precision");
            if (0 <= precision) {
                return precision;
            } else {
                CommandLine::PrintInfo(PrintInfoType::Warning, "Result precision is set to the default value of 8.");
                return 8;
            }
        }

        int InitStepSaveInterval() {
            const int interval = Internal::InitValue<int>("result.step_save_interval");
            if (interval < 1) {
                CommandLine::PrintInfo(PrintInfoType::Warning, "Step save interval is set to the default value of 10.");
                return 10;
            } else {
                return interval;
            }
        }
    }

    // ------------------------------------------------
    // 初期化
    // ------------------------------------------------

    const bool Processing::multiThread   = Internal::InitValue<bool>("processing.multi_thread");
    const size_t Processing::threadCount = Internal::InitThreadCount();

    const double Simulation::dt                  = Internal::InitValue<double>("simulation.dt");
    const double Simulation::detectPeakThreshold = Internal::initValueWithDefault<double>("simulation.detect_peak_threshold", 0.0);
    const double Simulation::windSpeedMin        = Internal::InitValue<double>("simulation.scatter.wind_speed_min");
    const double Simulation::windSpeedMax        = Internal::InitValue<double>("simulation.scatter.wind_speed_max");
    const double Simulation::windDirInterval     = Internal::InitValue<double>("simulation.scatter.wind_dir_interval");

    const int Result::precision        = Internal::InitResultPrecision();
    const int Result::stepSaveInterval = Internal::InitStepSaveInterval();

    const double WindModel::powerConstant         = Internal::InitValue<double>("wind_model.power_constant");
    const double WindModel::powerLowBaseAltitude  = Internal::InitValue<double>("wind_model.power_low_base_alt");
    const WindModelType WindModel::type           = Internal::InitWindModelType();
    const std::string WindModel::realdataFilename = Internal::InitValue<std::string>("wind_model.realdata_filename");

    const double Atmosphere::basePressure    = Internal::InitValue<double>("atmosphere.base_pressure_pascal");
    const double Atmosphere::baseTemperature = Internal::InitValue<double>("atmosphere.base_temperature_celsius");
}
