#pragma once

#include "Rule.h"
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>


namespace DFF {
    class Deal {
    public:
        [[nodiscard]] Deal() = default;
        inline Deal(std::string name) { this->name = name; }
        Deal(SKSE::SerializationInterface* intfc);

        inline std::string GetName() { return name; }
        inline bool IsOpen() { return rules.size() < 3; }
        inline void SetLockIn(bool a_lockIn) { this->lockIn = a_lockIn; }
        inline bool GetLockIn() { return lockIn; }
        
        std::vector<Rule*> rules;
        int GetCost();
        void UpdateTimer();
        void Extend(double by);

        void Serialize(SKSE::SerializationInterface* intfc);
    private:
        std::string name;
        double timer = 0.0f;
        bool lockIn = false;
    };
}

