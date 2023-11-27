#pragma once

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

        spdlog::level::level_enum _logLevel{spdlog::level::level_enum::info};
        spdlog::level::level_enum _flushLevel{spdlog::level::level_enum::trace};
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
    };
}
