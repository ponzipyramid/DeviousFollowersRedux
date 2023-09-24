#include <SKSE/SKSE.h>
#include <Device/DeviceManager.h>
#include <Config.h>
#include <DFF/Deal.h>
#include <articuno/archives/ryml/ryml.h>
#include <random>
#include <stdlib.h>
#include <ranges>
#include "DFF/DealManager.h"

using namespace DFF;
using namespace articuno::ryml;
using namespace SKSE;

namespace {
    int PickRandom(int max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, max - 1);
        return distr(gen);
    }
}

DeviceManager& DeviceManager::GetSingleton() noexcept {
    static DeviceManager instance;
    return instance;
}

void DeviceManager::Init() {     
    std::string dir("Data\\SKSE\\Plugins\\Devious Followers Redux\\Config");
    std::string devicesFile = dir + "\\devices.json";

    std::ifstream inputFile(devicesFile);
    if (inputFile.good()) {
        yaml_source ar(inputFile);

        ar >> lib;
       
        log::info("Loaded in device library");
    } else
        log::error("Error: Failed to read device library");
}

RE::TESObjectARMO* DeviceManager::GetRandomDeviceByKeyword(RE::Actor* actor, RE::BGSKeyword* kwd) { 
    auto devices = lib.GetDevicesByCategory(kwd->GetFormEditorID());
    
    SKSE::log::info("{} Devices Fetched: {}", kwd->GetFormEditorID(), devices.size());

    std::vector<RE::BGSKeyword*> badKwds;
    badKwds.push_back(kwd);

    RE::TESObjectREFR::InventoryItemMap itemMap = actor->GetInventory();
    for (auto& [item, value] : itemMap) {
        auto refr = item->As<RE::TESObjectARMO>();

        if (!refr) continue;

        for (auto& itemKwd : refr->GetKeywords()) {
            if (itemKwd->GetFormID() == kwd->GetFormID()) return nullptr;

            std::string kwdName(itemKwd->GetFormEditorID());
            if (kwdName.starts_with("zad_Devious")) {
                badKwds.push_back(itemKwd);
            }
        }
    }

    std::vector<Device> valid;

    std::copy_if(devices.begin(), devices.end(), std::back_inserter(valid), [=](Device& device) {
            auto item = device.GetArmor();
            return !item->HasKeywordInArray(badKwds, false); 
    });
    
    SKSE::log::info("{} Valid Devices Fetched: {}", kwd->GetFormEditorID(), valid.size());

    if (valid.empty()) return nullptr;

    std::vector<Device> filtered;
    std::copy_if(valid.begin(), valid.end(), std::back_inserter(filtered),
                 [=](Device& device) { return lib.IsValid(&device); });

    SKSE::log::info("{} Filtered Devices Fetched: {}", kwd->GetFormEditorID(), filtered.size());


    Device* chosen;
    if (filtered.empty()) {
        int rand = PickRandom(devices.size());
        chosen = &devices[rand];
    } else {
        int rand = PickRandom(valid.size());
        chosen = &valid[rand];
    }

    return chosen->GetArmor();
}

RE::TESObjectARMO* DeviceManager::GetWornItemByKeyword(RE::Actor* actor, RE::BGSKeyword* kwd) {

    SKSE::log::info("Attempting to find kwd {} on {}", kwd->GetFormEditorID(), actor->GetFormEditorID());

    RE::TESObjectREFR::InventoryItemMap itemMap = actor->GetInventory();
    for (auto& [item, value] : itemMap) {
        auto refr = item->As<RE::TESObjectARMO>();
        if (refr && refr->HasKeyword(kwd)) return refr;
    }

    return nullptr;
}