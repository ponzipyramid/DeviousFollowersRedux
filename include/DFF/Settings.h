#pragma once

namespace DFF {
    namespace Settings {
        inline double GetGameDaysPassed() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x39, "Skyrim.esm")->value;
        }

        inline double GetDealsBaseDays() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x3CDFB, "DeviousFollowers.esp")->value;
        }

        inline double GetBasePrice() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x3C88F, "DeviousFollowers.esp")->value;
        }

        inline double GetDailyDebt() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0xC547, "DeviousFollowers.esp")->value;
        }

        inline double GetDeepDebtDifficulty() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x27FFE3, "DeviousFollowers.esp")->value;
        }

        inline double GetMulti() {
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESGlobal>(0x03CDFC, "DeviousFollowers.esp")->value;
        }

        inline int GetWillpower() {
            return RE::TESDataHandler::GetSingleton()
                ->LookupForm<RE::TESGlobal>(0x1A2A7, "Update.esm")
                ->value;
        }
    }
}