#pragma once

#include <DFF/Rule.h>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace DFF {
    class RulePath {
    public:
        inline RulePath(std::string name) { this->name = name; }

        inline std::string GetName() { return name; }
        inline std::vector<std::string> GetRuleIds() { return ruleIds; }

    private:
        std::vector<std::string> ruleIds;
        std::string name;
    };

}  // namespace DFF