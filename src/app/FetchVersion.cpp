// ------------------------------------------------
// FetchVersion.hppの実装
// ------------------------------------------------

#include "FetchVersion.hpp"

#include <iostream>
#include <memory>
#include <array>
#include <string>
#include <stdexcept>

#include "app/CommandLine.hpp"
#include "misc/Platform.hpp"


const std::string PROLOGUE_LATEST_URL = "https://github.com/FROM-THE-EARTH/Prologue/releases/latest";
const int MAX_REDIRECTS = 5;

namespace FetchVersion {
	std::string GetLatestVersionString(){
		std::string cmd;
		// redirect device is different between Windows and Mac/Linux systems
		if (PLATFORM_WINDOWS)
			cmd = "curl -sL -o nul -w \"%{url_effective}\" \"" + PROLOGUE_LATEST_URL + "\"";
		else
			cmd = "curl -sL -o /dev/null -w \"%{url_effective}\" \"" + PROLOGUE_LATEST_URL + "\"";

		std::array<char, 4096> buf{};
		std::string result;

		FILE* pipe = POPEN(cmd.c_str(), "r");
		if (!pipe) {
			CommandLine::PrintInfo(PrintInfoType::Warning, "Failed to check latest version.");
			return "N/A";
		}
		while (fgets(buf.data(), static_cast<int>(buf.size()), pipe)) {
			result += buf.data();
		}

		int rc = PCLOSE(pipe);
		if (rc != 0 || result.empty()) {
			CommandLine::PrintInfo(PrintInfoType::Warning, "Failed to check latest version.");
			return "N/A";
		}

		// trim newline characters
		while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) result.pop_back();

		// Extract version from URL
		size_t pos = result.find_last_of('/');
		if (pos != std::string::npos && pos + 1 < result.size())
			result = result.substr(pos + 2);
		else
			result = "N/A";


		return result;
	}
}
