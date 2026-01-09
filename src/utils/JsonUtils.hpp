// ------------------------------------------------
// JSON読み取りに関する関数定義
// ほぼtemplateでcppで定義できないのでhppで定義
// ------------------------------------------------

#pragma once

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "app/CommandLine.hpp"

namespace JsonUtils {
    bool Exist(const boost::property_tree::ptree& pt, const std::string& key);

    template <typename T>
    bool HasValue(const boost::property_tree::ptree& pt, const std::string& key) {
        if (boost::optional<T> value = pt.get_optional<T>(key))
            return true;
        else
            return false;
    }

    template <typename T>
    T GetValue(const boost::property_tree::ptree& pt, const std::string& key) {
        if (const boost::optional<T> value = pt.get_optional<T>(key)) {
            return value.get();
        }
        return T();
    }

    template <typename T>
    std::optional<T> GetOptional(const boost::property_tree::ptree& pt, const std::string& key) {
        if (const auto result = pt.get_optional<T>(key); result.has_value()) {
            return result.value();
        } else {
            return std::nullopt;
        }
    }

    template <typename T>
    std::optional<T> GetValueOpt(const boost::property_tree::ptree& pt, const std::string& key) {
        if (JsonUtils::HasValue<T>(pt, key)) {
            return JsonUtils::GetValue<T>(pt, key);
        } else {
            return std::nullopt;
        }
    }

    template <typename T>
    T GetValueWithDefault(const boost::property_tree::ptree& pt, const std::string& key, T defaultValue) {
        if (JsonUtils::HasValue<T>(pt, key)) {
            return JsonUtils::GetValue<T>(pt, key);
        } else {
            return defaultValue;
        }
    }

    template <typename T>
    T GetValueExc(const boost::property_tree::ptree& pt, const std::string& key) {
        if (!JsonUtils::HasValue<T>(pt, key)) {
            CommandLine::PrintInfo(PrintInfoType::Error, "The key of " + key + " has no value.");
            throw 0;
        }

        return JsonUtils::GetValue<T>(pt, key);
    }
}
