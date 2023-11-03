#pragma once

#include <articuno/articuno.h>
#include <articuno/types/auto.h>

namespace DFF {
    class Device {
    public:
        inline RE::TESObjectARMO* GetArmor() { 
            return RE::TESDataHandler::GetSingleton()->LookupForm<RE::TESObjectARMO>(formId, espName);
        }
        inline std::string GetName() { 
            return name;
        }
        std::unordered_set<std::string> keywordNames;
        inline std::string GetKeywordListString() { 
            std::string listString;
            for (auto& keywordName : keywordNames) {
                listString += keywordName + ',';
            }

            return listString;
        }
    private:
        articuno_deserialize(ar) { 
            ar <=> articuno::kv(name, "name");
            ar <=> articuno::kv(formId, "formId");
            ar <=> articuno::kv(espName, "espName");
        }

        std::string name;
        int formId;
        std::string espName;

        friend class articuno::access;
        friend class DeviceLibrary;
    };

    class DeviceLibrary {
    public:
        inline std::vector<Device>& GetDevicesByCategory(std::string cat) {
            return deviceCategories[cat]; 
        }
        inline Device* GetDeviceByArmor(RE::TESBoundObject* obj) { return obj ? devices[obj->GetFormID()] : nullptr; }

        inline bool IsValid(const Device* device) {
            return validDevices.contains(device->name);
        }
    private:
        articuno_deserialize(ar) {
            ar <=> articuno::kv(filters, "filters");

            deviceCategories = {
                {"zad_DeviousCollar", zad_DeviousCollar}, 
                {"zad_DeviousPonyGear", zad_DeviousPonyGear}, 
                {"zad_DeviousBelt", zad_DeviousBelt}, 
                {"zad_DeviousPlug", zad_DeviousPlug}, 
                {"zad_DeviousPlugVaginal", zad_DeviousPlugVaginal}, 
                {"zad_DeviousPlugAnal", zad_DeviousPlugAnal}, 
                {"zad_DeviousBra", zad_DeviousBra}, 
                {"zad_DeviousArmCuffs", zad_DeviousArmCuffs}, 
                {"zad_DeviousLegCuffs", zad_DeviousLegCuffs}, 
                {"zad_DeviousArmbinder", zad_DeviousArmbinder}, 
                {"zad_DeviousGag", zad_DeviousGag}, 
                {"zad_DeviousGagPanel", zad_DeviousGagPanel},
                {"zad_DeviousGagRing", zad_DeviousGagRing}, 
                {"zad_DeviousBlindfold", zad_DeviousBlindfold}, 
                {"zad_DeviousHarness", zad_DeviousHarness}, 
                {"zad_DeviousPiercingsNipple", zad_DeviousPiercingsNipple}, 
                {"zad_DeviousPiercingsVaginal", zad_DeviousPiercingsVaginal}, 
                {"zad_DeviousCorset", zad_DeviousCorset}, 
                {"zad_DeviousHeavyBondage", zad_DeviousHeavyBondage}, 
                {"zad_DeviousSuit", zad_DeviousSuit}, 
                {"zad_DeviousHobbleSkirt", zad_DeviousHobbleSkirt}, 
                {"zad_DeviousHobbleSkirtRelaxed", zad_DeviousHobbleSkirtRelaxed}, 
                {"zad_DeviousStraitJacket", zad_DeviousStraitJacket}, 
                {"zad_DeviousHood", zad_DeviousHood}, 
                {"zad_DeviousGagLarge", zad_DeviousGagLarge}, 
                {"zad_DeviousYoke", zad_DeviousYoke}, 
                {"zad_DeviousBoots", zad_DeviousBoots}, 
                {"zad_DeviousGloves", zad_DeviousGloves}, 
                {"zad_DeviousBondageMittens", zad_DeviousBondageMittens}, 
                {"zad_DeviousAnkleShackles", zad_DeviousAnkleShackles}, 
                {"zad_DeviousCuffsFront", zad_DeviousCuffsFront}, 
                {"zad_DeviousArmbinderElbow", zad_DeviousArmbinderElbow}, 
                {"zad_DeviousYokeBB", zad_DeviousYokeBB}, 
                {"zad_DeviousPetSuit", zad_DeviousPetSuit}, 
                {"zad_DeviousElbowTie", zad_DeviousElbowTie}
            };

            auto handler = RE::TESDataHandler::GetSingleton();
            std::unordered_set<RE::FormID> seen;

            for (auto& [key, list] : deviceCategories) {
                ar <=> articuno::kv(list, key);

                deviceCategories[key] = list;

                auto& listDevices = deviceCategories[key];

                for (auto& lDevice : listDevices) {
                    auto id = lDevice.GetArmor()->GetFormID();
                    auto device = GetDeviceByArmor(lDevice.GetArmor());

                    if (!device) {
                        devices[id] = &lDevice;  
                        device = GetDeviceByArmor(lDevice.GetArmor());
                    }

                    device->keywordNames.insert(key);

                    SKSE::log::info("DeviceLibary::Load: adding {} to {} has {} kwds", key, device->GetName(), device->keywordNames.size());
                    

                    for (auto& filter : filters) {
                        if (device->name.contains(filter)) {
                            validDevices.insert(device->name);
                        }
                    }

                    seen.insert(id);
                }
            }

            SKSE::log::info("{} devices registered", devices.size());
            SKSE::log::info("{} devices are valid", validDevices.size());
        }
        std::vector<Device> zad_DeviousCollar;
        std::vector<Device> zad_DeviousPonyGear;
        std::vector<Device> zad_DeviousBelt;
        std::vector<Device> zad_DeviousPlug;
        std::vector<Device> zad_DeviousPlugVaginal;
        std::vector<Device> zad_DeviousPlugAnal;
        std::vector<Device> zad_DeviousBra;
        std::vector<Device> zad_DeviousArmCuffs;
        std::vector<Device> zad_DeviousLegCuffs;
        std::vector<Device> zad_DeviousArmbinder;
        std::vector<Device> zad_DeviousGag;
        std::vector<Device> zad_DeviousGagPanel;
        std::vector<Device> zad_DeviousGagRing;
        std::vector<Device> zad_DeviousBlindfold;
        std::vector<Device> zad_DeviousHarness;
        std::vector<Device> zad_DeviousPiercingsNipple;
        std::vector<Device> zad_DeviousPiercingsVaginal;
        std::vector<Device> zad_DeviousCorset;
        std::vector<Device> zad_DeviousHeavyBondage;
        std::vector<Device> zad_DeviousSuit;
        std::vector<Device> zad_DeviousHobbleSkirt;
        std::vector<Device> zad_DeviousHobbleSkirtRelaxed;
        std::vector<Device> zad_DeviousStraitJacket;
        std::vector<Device> zad_DeviousHood;
        std::vector<Device> zad_DeviousGagLarge;
        std::vector<Device> zad_DeviousYoke;
        std::vector<Device> zad_DeviousBoots;
        std::vector<Device> zad_DeviousGloves;
        std::vector<Device> zad_DeviousBondageMittens;
        std::vector<Device> zad_DeviousAnkleShackles;
        std::vector<Device> zad_DeviousCuffsFront;
        std::vector<Device> zad_DeviousArmbinderElbow;
        std::vector<Device> zad_DeviousYokeBB;
        std::vector<Device> zad_DeviousPetSuit;
        std::vector<Device> zad_DeviousElbowTie;

        std::vector<std::string> filters;
        std::unordered_map<RE::FormID, Device*> devices;
        std::unordered_map<std::string, std::vector<Device>> deviceCategories;
        std::unordered_set<std::string> validDevices;

        friend class articuno::access;
    };
}