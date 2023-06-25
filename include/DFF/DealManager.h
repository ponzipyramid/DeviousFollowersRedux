#pragma once

#include <RE/Skyrim.h>
#include <DFF/Deal.h>

namespace DFF {
#pragma warning(push)
#pragma warning(disable : 4251)
    /**
     * The class which tracks hit count information.
     *
     * <p>
     * All aspects of this are done in a singleton since SKSE cannot create instances of new Papyrus types such as a
     * hit counter.
     * </p>
     *
     * <p>
     * Hit count information must be tracked in the SKSE cosave when the game is save and loaded. Therefore the main
     * <code>SKSEQuery_Load</code> call must be sure to initialize the serialization handlers such that they will call
     * this class when it needs to save or load data.
     * </p>
     */
    class __declspec(dllexport) DealManager {
    public:
        /**
         * Get the singleton instance of the <code>DealManager</code>.
         */
        [[nodiscard]] static DealManager& GetSingleton() noexcept;

        void InitDeals();

        void InitQuests();

        void InitQuestData();

        int SelectDeal(int track, int maxModDeals, float prefersModular, int lastRejectedId);


        /**
         * Inform DFF that a deal has been activated.
         *
         * @param The id of the deal that has been selected. This should be the deal chosen by SelectDeal in most cases.\
         */
        void ActivateDeal(int id);

        /**
         * Inform DFF that a deal has been removed.
         *
         * @param The id of the deal that has been removed.
         */
        void RemoveDeal(RE::TESQuest* quest);

        /**
         * Progress deal and return the stage index.
         *
         * @param The id of the deal.
         */
        int GetStage(int id);

        /**
         * Allows for checking if a particular deal is active. Generally used for displaying buy out deal option.
         *
         * @param The id of the deal.
         * 
         * @return true if deal is active, false if not
         */
        bool IsDealActive(int id);

        /**
         * Allows for checking if a particular deal has been selected. 
         *
         * @param The id of the deal.
         *
         * @return true if deal is active, false if not
         */
        bool IsDealSelected(std::string name);


        /**
         * Get quest for deal.
         *
         * @param The id of the deal.
         */
        RE::TESQuest* GetDealQuest(int id);

        void ActivateRule(RE::TESQuest* quest, int id);

        void LoadActiveModularRules(std::vector<RE::TESQuest*> quests, std::vector<int> ruleIds);

        Deal* GetDealById(int id, bool modular = false);

        void LoadRuleMaxStage(RE::TESQuest*, int maxStage);

        std::vector<RE::TESQuest*> GetActiveDeals(bool classic);
        std::string GetDealName(RE::TESQuest* quest);
        std::vector<std::string> GetDealRules(RE::TESQuest* quest);

        void ToggleRule(std::string group, int id, bool enabled);
        void ToggleStageVariation(std::string name, int stageIndex, int varIndex, bool enabled);

        int GetDealNextQuestStage(int id);

        /**
         * The serialization handler for reverting game state.
         *
         * <p>
         * This is called as the handler for revert. Revert is called by SKSE on a plugin that is registered for
         * serialization handling on a new game or before a save game is loaded. It should be used to revert the state
         * of the plugin back to its default.
         * </p>
         */
        static void OnRevert(SKSE::SerializationInterface*);

        /**
         * The serialization handler for saving data to the cosave.
         *
         * @param serde The serialization interface used to write data.
         */
        static void OnGameSaved(SKSE::SerializationInterface* serde);

        /**
         * The serialization handler for loading data from a cosave.
         *
         * @param serde  The serialization interface used to read data.
         */
        static void OnGameLoaded(SKSE::SerializationInterface* serde);


    private:
        void RegeneratePotentialDeals(int maxModDeals, int lastRejectedId);
        bool DoesActiveExclude(Conflictor* entity);
        Rule* GetRuleById(int id);

        DealManager() = default;
        mutable std::mutex _lock;

        std::unordered_map<std::string, Deal> deals;
        std::unordered_map<int, std::string> id_name_map;
        std::unordered_map<std::string, int> name_id_map;

        std::vector<std::string> candidateClassicDeals;
        std::vector<std::string> candidateModularDeals;

        std::vector<Deal*> builtInDeals;
        std::vector<Deal*> modularDeals;

        std::unordered_set<std::string> activeDeals;
        std::unordered_map<std::string, std::vector<std::string>> dealGroups;
        std::unordered_map<RE::FormID, Deal*> formMap;
        std::string selectedDeal = "";

        std::unordered_map<std::string, Rule> rules;
        std::unordered_map<std::string, std::vector<Rule*>> ruleGroups;
        std::unordered_map<Deal*, std::vector<Rule*>> activeRules;

        std::vector<Rule*> candidateRules;

        std::unordered_map<Conflictor*, std::unordered_set<Conflictor*>> conflicts;
    };
#pragma warning(pop)
}
