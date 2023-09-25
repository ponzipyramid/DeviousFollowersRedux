#include "DFF/Deal.h"
#include "DFF/Settings.h"
#include "DFF/DealManager.h"
#include "Serialization.hpp"

using namespace DFF;

void Deal::UpdateTimer() {
    timer =  + Settings::GetDealsBaseDays();
}

void Deal::Extend(double by) {
    if (by == 0.0f) {
        auto baseDays = Settings::GetDealsBaseDays();

        if (timer < baseDays) {
            timer = baseDays;
        }

        timer += baseDays;
    } else {
        timer += by;
    }
}

int Deal::GetCost() {
    int stage = rules.size();

    double basePrice = Settings::GetBasePrice();

    double price = basePrice * stage;

    double now = Settings::GetGameDaysPassed();

    if (timer > now) {
        double earlyOutPrice = Settings::GetMulti() * basePrice;
        price += (timer - now) * earlyOutPrice;
    }

    int multiplier = 1;
    for (auto& rule : rules) {
        if (rule->GetPath() == "devious followers continued/expensive") {
            price += multiplier * Settings::GetDailyDebt() * Settings::GetDeepDebtDifficulty();
            break;
        }
        multiplier *= 2;
    }

    return std::ceil(price);
}

void Deal::Serialize(SKSE::SerializationInterface* serde) {
    Serialization::Write<std::string>(serde, name);
    Serialization::Write<double>(serde, timer);

    Serialization::Write<std::size_t>(serde, rules.size());
    for (auto rule : rules) {
        Serialization::Write<std::string>(serde, rule->GetPath());
    }
}

Deal::Deal(SKSE::SerializationInterface* serde) {
    name = Serialization::Read<std::string>(serde);
    timer = Serialization::Read<double>(serde);

    int numRules = Serialization::Read<std::size_t>(serde);
    rules.reserve(numRules);

    auto allRules = DealManager::GetSingleton().rules;

    for (; numRules > 0; numRules--) {
        auto path = Serialization::Read<std::string>(serde);
        if (auto rule = DealManager::GetSingleton().GetRuleByPath(path)) {
            rules.push_back(rule); 
        } else {
            SKSE::log::info("Deal Constructor: Failed to load in rule {}", path);
        }
    }
}