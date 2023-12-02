#pragma once

#include <Deals/Rule.h>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

namespace DFF {
    class RulePath {
    public:
        inline RulePath(std::string name, YAML::Node a_node) { 
            this->name = name;
            this->ruleIds = a_node["rules"].as<std::vector<std::string>>();
        }

        inline std::string GetName() { return name; }
        inline std::vector<std::string> GetRuleIds() { return ruleIds; }

    private:
        std::vector<std::string> ruleIds;
        std::string name;
    };

}  // namespace DFF