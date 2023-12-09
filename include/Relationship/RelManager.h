#pragma once
#include "Relationship//RelStage.h"
#include "Deals/DealManager.h"

namespace DFF {
	#pragma warning(push)
	#pragma warning(disable : 4251)

	
	class __declspec(dllexport) RelManager {
	public:
		[[nodiscard]] static RelManager& GetSingleton() noexcept;

		void Init();

		inline int GetStage() {
			return currentStageIndex;
		}

		inline void ActivateRelationshipStage(int stage) {
			SKSE::log::info("ActivateRelationshipStage: setting stage to {}", stage);
			
			DealManager& dealManager = DFF::DealManager::GetSingleton();

			currentStageIndex = stage - 1;
			auto& currentStage = stages[currentStageIndex];

			for (int i = stage; i < max_stages; i++) {
				dealManager.RemoveDeal(stages[i].GetName());
			}

			SKSE::log::info("ActivateRelationshipStage: finished setting stage to {}", currentStage.GetName());
		}

		inline std::string GetRelationshipStageDeal() {
			return stages[currentStageIndex].GetName();
		}

		inline std::vector<std::string> GetLockInRules() {
			return stages[currentStageIndex].GetLockInRules();
		}

		template <typename T>
		inline T GetRelStageSetting(std::string setting) {
			return stages[currentStageIndex].GetSetting<T>(setting);
		}

		inline int GetRelationshipStageIndex() {
			return currentStageIndex + 1;
		}

		inline RelStage& GetRelationshipStage() {
			return stages[currentStageIndex];
		}

		inline void ActivateRelationshipStageDeak() {
			DealManager& dealManager = DFF::DealManager::GetSingleton();
			auto& currentStage = stages[currentStageIndex];

			for (auto ruleId : stages[currentStageIndex].GetLockInRules()) {
				dealManager.ActivateRule(ruleId, currentStage.GetName(), true);
			}

			dealManager.SetDealLockIn(currentStage.GetName(), true);
		}

		inline std::vector<std::string> GetRelationshipStageRules() {
			std::vector<std::string> validLockInRules;

			DealManager& dealManager = DFF::DealManager::GetSingleton();
			auto lockInRules = stages[currentStageIndex].GetLockInRules();
			for (auto& rule : lockInRules) {
				if (dealManager.GetRuleGlobal(rule)) {
					validLockInRules.push_back(rule);
				}
			}

			return validLockInRules;
		}

	private:
		std::vector<RelStage> stages;
		int currentStageIndex = 0;
		const int max_stages = 5;
		RE::TESQuest* flowQuest;
	};
}