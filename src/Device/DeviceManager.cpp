#include <SKSE/SKSE.h>
#include <Device/DeviceManager.h>
#include <Config/Config.h>
#include <Deals/Deal.h>
#include <random>
#include <stdlib.h>
#include <ranges>
#include "Deals/DealManager.h"

using namespace DFF;
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
    std::string devicesDefaultFile = dir + "\\devices.default.json";
    std::string devicesFile = dir + "\\devices.json";

    if (!std::filesystem::exists(devicesDefaultFile)) {
        log::error("Error: devices default file doesn't exist");
        return;
    }

    if (!std::filesystem::exists(devicesFile) && !std::filesystem::copy_file(devicesDefaultFile, devicesFile)) {
        log::error("Error: failed to generate devices file");
        return;
    }

    std::ifstream inputFile(devicesFile);
    if (inputFile.good()) {
        auto data = json::parse(inputFile);
        this->lib = DeviceLibrary(data);

        log::info("Loaded in device library");
    } else
        log::error("Error: Failed to read device library");
}

RE::TESObjectARMO* DeviceManager::GetRandomDeviceByKeyword(RE::Actor* actor, RE::BGSKeyword* kwd) { 
    auto devices = lib.GetDevicesByCategory(kwd->GetFormEditorID());
    
    SKSE::log::info("GetRandomDeviceByKeyword: {} Devices Fetched: {}", kwd->GetFormEditorID(), devices.size());

    std::vector<RE::BGSKeyword*> badKwds;
    badKwds.push_back(kwd);

    RE::TESObjectREFR::InventoryItemMap itemMap = actor->GetInventory();
    for (auto& [item, value] : itemMap) {
        auto refr = item->As<RE::TESObjectARMO>();
        log::info("GetRandomDeviceByKeyword: Scanning {} {}", item->GetName(), refr != nullptr);

        if (!refr) continue;

        for (auto& itemKwd : refr->GetKeywords()) {
            if (itemKwd->GetFormID() == kwd->GetFormID()) return refr;

            std::string kwdName(itemKwd->GetFormEditorID());
            if (kwdName.starts_with("zad_Devious")) {
                SKSE::log::info("GetRandomDeviceByKeyword: exclude kwd {}", kwdName);
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

    SKSE::log::info("GetWornItemByKeyword: find kwd {} on {}", kwd->GetFormEditorID(),
                    actor->GetActorBase()->GetName());

    auto itemMap = actor->GetInventory();
    for (auto& [item, _] : itemMap) {
        log::info("GetWornItemByKeyword: processing {} {}", item->GetName(), item->GetFormID());
        auto refr = item->As<RE::TESObjectARMO>();
        if (auto device = lib.GetDeviceByArmor(refr)) {
            if (device->keywordNames.contains(kwd->GetFormEditorID())) {
                SKSE::log::info("Found Item: {}", refr->GetName());
                return refr;
            } else {
                log::info("GetWornItemByKeyword: {} is devious but not item {} w/o {}", item->GetFormEditorID(),
                          device->GetKeywordListString(), kwd->GetFormEditorID()); 
            }
        } else {
            log::info("GetWornItemByKeyword: {} is not an inventory device {}", item->GetName(), refr != nullptr);
        }
    }
    SKSE::log::info("GetWornItemByKeyword: could not find kwd {} on {}", kwd->GetFormEditorID(),
                    actor->GetActorBase()->GetName());
    return nullptr;
}
