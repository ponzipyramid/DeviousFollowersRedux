#include "DFF/Pack.h"

using namespace DFF;

bool Pack::Init(RE::TESDataHandler* handler) {
    quest = handler->LookupForm<RE::TESQuest>(formId, modName);
    return quest != nullptr;
}