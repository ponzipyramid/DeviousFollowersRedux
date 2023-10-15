#pragma once

#include <DFF/Rule.h>
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>
#include <articuno/articuno.h>
#include <articuno/types/auto.h>

namespace DFF {
    class RulePath {
        articuno_deserialize(ar) { 
            ar <=> articuno::kv(ruleIds, "ruleIds"); 
        }

        std::vector<std::string> ruleIds;
    }
}  // namespace DFF