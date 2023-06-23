#include <DFF/HitCounterManager.h>

#include <catch.hpp>

using namespace DFF;

TEST_CASE("HitCounterManager/GetCountForNullActor") {
    CHECK(!HitCounterManager::GetSingleton().GetHitCount(nullptr).has_value());
}
