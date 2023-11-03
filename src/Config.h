#pragma once

#include <articuno/articuno.h>
#include <articuno/types/auto.h>
#include <SKSE/SKSE.h>

namespace DFF {
    class Debug {
    public:
        [[nodiscard]] inline spdlog::level::level_enum GetLogLevel() const noexcept {
            return _logLevel;
        }

        [[nodiscard]] inline spdlog::level::level_enum GetFlushLevel() const noexcept {
            return _flushLevel;
        }

    private:
        articuno_serialize(ar) {
            auto logLevel = spdlog::level::to_string_view(_logLevel);
            auto flushLevel = spdlog::level::to_string_view(_flushLevel);
            ar <=> articuno::kv(logLevel, "logLevel");
            ar <=> articuno::kv(flushLevel, "flushLevel");
        }

        articuno_deserialize(ar) {
            *this = Debug();
            std::string logLevel;
            std::string flushLevel;
            if (ar <=> articuno::kv(logLevel, "logLevel")) {
                _logLevel = spdlog::level::from_str(logLevel);
            }
            if (ar <=> articuno::kv(flushLevel, "flushLevel")) {
                _flushLevel = spdlog::level::from_str(flushLevel);
            }
        }

        spdlog::level::level_enum _logLevel{spdlog::level::level_enum::info};
        spdlog::level::level_enum _flushLevel{spdlog::level::level_enum::trace};

        friend class articuno::access;
    };

    enum SeverityMode {
        Median,
        Mode,
        Max,
        None
    };

    class Config {
    public:
        [[nodiscard]] inline const Debug& GetDebug() const noexcept {
            return _debug;
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
        articuno_serde(ar) {
            std::string _targetSeverityMode;

            ar <=> articuno::kv(_debug, "debug");
            ar <=> articuno::kv(forcedDealIds, "forcedDeals");
            
            ar <=> articuno::kv(baseScore, "baseScore");
            
            ar <=> articuno::kv(belowThreshBoost, "belowThreshBoost");
            ar <=> articuno::kv(exactThreshBoost, "exactThreshBoost");
            
            ar <=> articuno::kv(pathBoost, "pathBoost");
            
            ar <=> articuno::kv(lowWilpowerBoost, "lowWilpowerBoost");
            ar <=> articuno::kv(highWillpowerBoost, "highWillpowerBoost");
           
            ar <=> articuno::kv(lowWillpowerThresh, "lowWillpowerThresh");
            ar <=> articuno::kv(highWillpowerThresh, "highWillpowerThresh");
            
            ar <=> articuno::kv(applyMultiplePathBoost, "applyMultiplePathBoost");

            ar <=> articuno::kv(_targetSeverityMode, "targetSeverityMode");

            std::unordered_map<std::string, SeverityMode> calcModeMapping = {
                {"median", SeverityMode::Median},
                {"mode", SeverityMode::Mode},
                {"max", SeverityMode::Max},
                {"none", SeverityMode::None},
            };

            targetSeverityMode = calcModeMapping.count(_targetSeverityMode) ? calcModeMapping[_targetSeverityMode]
                                                                            : SeverityMode::Median;

            for (auto& id : forcedDealIds) std::transform(id.begin(), id.end(), id.begin(), ::tolower);
        }

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

        Debug _debug;
        friend class articuno::access;
    };
}
