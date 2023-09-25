#pragma once

namespace DFF {
    namespace Settings {
        double GetGameDaysPassed() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x39, "Skyrim.esm")->value;
        }

        double GetDealsBaseDays() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x3CDFB, "DeviousFollowers.esp")->value;
        }

        double GetBasePrice() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x3C88F, "DeviousFollowers.esp")->value;
        }

        double GetDailyDebt() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0xC547, "DeviousFollowers.esp")->value;
        }

        double GetDeepDebtDifficulty() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x27FFE3, "DeviousFollowers.esp")->value;
        }

        double GetMulti() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x03CDFC, "DeviousFollowers.esp")->value;
        }
    }
}