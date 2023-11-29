#pragma once

#include <SKSE/SKSE.h>
#include "Parse.hpp"

namespace DFF {
    enum SeverityMode {
        Median,
        Mode,
        Max,
        None
    };

    class Config {
    public:
        Config() = default;
        Config(YAML::Node a_node) {
            std::string _targetSeverityMode;
            this->baseScore = Parse::FetchIfPresent<int>(a_node["baseScore"], 0);

            this->belowThreshBoost = Parse::FetchIfPresent<int>(a_node["belowThreshBoost"], 0);
            this->exactThreshBoost = Parse::FetchIfPresent<int>(a_node["exactThreshBoost"], 0);
            
            this->pathBoost = Parse::FetchIfPresent<int>(a_node["pathBoost"], 0);
           
            this->lowWilpowerBoost = Parse::FetchIfPresent<int>(a_node["lowWilpowerBoost"], 0);
            this->highWillpowerBoost = Parse::FetchIfPresent<int>(a_node["highWillpowerBoost"], 0);

            this->lowWillpowerThresh = Parse::FetchIfPresent<int>(a_node["lowWillpowerThresh"], 0);
            this->highWillpowerThresh = Parse::FetchIfPresent<int>(a_node["highWillpowerThresh"], 0);
            
            this->applyMultiplePathBoost = Parse::FetchIfPresent<bool>(a_node["applyMultiplePathBoost"], false);
            
            this->forcedDealIds = Parse::FetchIfPresent<std::vector<std::string>>(a_node["forcedDealIds"], std::vector<std::string>());
            for (auto& id : forcedDealIds) std::transform(id.begin(), id.end(), id.begin(), ::tolower);

            _targetSeverityMode = Parse::FetchIfPresent<std::string>(a_node["targetSeverityMode"], "median");

            std::unordered_map<std::string, SeverityMode> calcModeMapping = {
                {"median", SeverityMode::Median},
                {"mode", SeverityMode::Mode},
                {"max", SeverityMode::Max},
                {"none", SeverityMode::None},
            };

            targetSeverityMode = calcModeMapping.count(_targetSeverityMode) ? calcModeMapping[_targetSeverityMode]
                : SeverityMode::Median;
        }
        
        [[nodiscard]] inline const int GetBaseScore() const noexcept { return baseScore; }
        
        [[nodiscard]] inline const int GetBelowThresholdBoost() const noexcept { return belowThreshBoost; }
        
        [[nodiscard]] inline const int GetExactThresholdBoost() const noexcept { return exactThreshBoost; }
        
        [[nodiscard]] inline const int GetPathBoost() const noexcept { return pathBoost; }
        
        [[nodiscard]] inline const int GetLowWillpowerThreshold() const noexcept { return lowWillpowerThresh; }
       
        [[nodiscard]] inline const int GetHighWillpowerThreshold() const noexcept { return highWillpowerThresh; }
        
        [[nodiscard]] inline const int GetLowWillpowerBoost() const noexcept { return lowWilpowerBoost; }
        
        [[nodiscard]] inline const int GetHighWillpowerBoost() const noexcept { return highWillpowerBoost; }
        
        [[nodiscard]] inline const bool GetApplyMultiplePathBoost() const noexcept { return applyMultiplePathBoost; }

        [[nodiscard]] inline const SeverityMode GetTargetSeverityMode() const noexcept { return targetSeverityMode; }

        [[nodiscard]] inline const std::vector<std::string> GetForcedDealIds() const noexcept { return forcedDealIds; }

        [[nodiscard]] static const Config& GetSingleton() noexcept;

    private:
        std::vector<std::string> forcedDealIds;
        int baseScore;
        int belowThreshBoost;
        int exactThreshBoost;
        int pathBoost;
        int lowWilpowerBoost;
        int highWillpowerBoost;
        int lowWillpowerThresh;
        int highWillpowerThresh;
        bool applyMultiplePathBoost;
        SeverityMode targetSeverityMode;
    };
}
