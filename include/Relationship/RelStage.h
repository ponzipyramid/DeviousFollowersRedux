#pragma once
#include "Util/Parse.hpp"

namespace DFF {
	class RelStage {
	public:
		inline std::string GetName() { return name; }
		inline std::vector<std::string> GetLockInRules() { return lockInRules; }

		template <typename T>
		inline T GetSetting(std::string a_name) {
			SKSE::log::info("Fetching {}", a_name);

			try {
				return std::any_cast<T>(settings[a_name]);
			}
			catch (std::exception e) {
				SKSE::log::info("Failed to fetch setting {} due to {}", a_name, e.what());
				T val = T(); 
				return val;
			}
		}
	private:
		std::string name;
		std::vector<std::string> lockInRules;

		std::unordered_map<std::string, std::any> settings;

		friend struct YAML::convert<RelStage>;
	};
}

namespace YAML {
	template<>
	struct convert<DFF::RelStage> {
		static bool decode(const Node& node, DFF::RelStage& rhs) {
			rhs.name = node["name"].as<std::string>();

			rhs.lockInRules = Parse::FetchIfPresent<std::vector<std::string>>(node["lockInRules"], std::vector<std::string>());

			rhs.settings["allowDealRejection"] = Parse::FetchIfPresent<bool>(node["allowDealRejection"], true);
			rhs.settings["autoGoldControlMode"] = Parse::FetchIfPresent<bool>(node["autoGoldControlMode"], true);
			rhs.settings["allowBuyoutChance"] = Parse::FetchIfPresent<float>(node["autoGoldControlMode"], 100.0f);

			// buyout favor requirement
			// reject favor requirement

			rhs.settings["forcedDealsBoredomThreshold"] = Parse::FetchIfPresent<float>(node["allowBoredForcedDeals"], 0.0f);
			rhs.settings["forcedPunishmentAnnoyanceThreshold"] = Parse::FetchIfPresent<float>(node["autoGoldControlMode"], -80.0f);

			return true;
		}
	};
}