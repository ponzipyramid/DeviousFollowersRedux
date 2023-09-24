#pragma once

#include "Rule.h"
#include <articuno/articuno.h>
#include <articuno/types/auto.h>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>


namespace DFF {
    class Deal {
    public:
        [[nodiscard]] Deal() = default;
        inline Deal(std::string name) { this->name = name; }
        inline std::string GetName() { return name; }
        std::vector<Rule*> rules;
        int GetDealCost();

    private:


        std::string name;
    };
}

