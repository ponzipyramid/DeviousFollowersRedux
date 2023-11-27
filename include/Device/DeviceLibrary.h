#pragma once

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

        std::string name;
        int formId;
        std::string espName;

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
    };
}