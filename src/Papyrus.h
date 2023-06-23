#pragma once

#include <RE/Skyrim.h>

namespace DFF {
    bool RegisterHitCounter(RE::BSScript::IVirtualMachine* vm);
    bool RegisterDealManager(RE::BSScript::IVirtualMachine* vm);
    void InitializeHook(SKSE::Trampoline& trampoline);
}
