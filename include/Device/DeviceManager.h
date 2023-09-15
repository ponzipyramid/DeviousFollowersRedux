#pragma once

#include "Device/DeviceLibrary.h"

namespace DFF {
    class DeviceManager {
    public:
        [[nodiscard]] static DeviceManager& GetSingleton() noexcept;

        void Init();
        RE::TESObjectARMO* GetRandomDeviceByKeyword(RE::Actor* actor, RE::BGSKeyword* kwd);

        RE::TESObjectARMO* GetWornItemByKeyword(RE::Actor* actor, RE::BGSKeyword* kwd);

    private:
        DeviceLibrary lib;
    };

}  // namespace DFF