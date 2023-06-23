#include <DFF/DealManager.h>

#include <catch.hpp>

using namespace DFF;

TEST_CASE("DealManager/GetCountForNullActor") { CHECK(!DealManager::GetSingleton().IsDealActive(10)); }