#pragma once

#include <RE/Skyrim.h>
#include <DFF/Deal.h>
#include <DFF/Rule.h>

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

        int SelectDeal(int lastRejectedId);

        int ActivateRule(int id);

        int ActivateRule(std::string name);

        void RemoveDeal(std::string name);

        void ResetAllDeals();

        void Pause();

        void Resume();

        void ExtendDeal(std::string deal, double by);

        std::string GetRandomDeal();

        RE::TESGlobal* GetRuleGlobal(int id);
        
        int GetDealCost(std::string name);

        double GetExpensiveDebtCount();

        std::vector<std::string> GetDeals();

        std::vector<std::string> GetDealRules(std::string name);

        std::vector<std::string> GetGroupNames();

        std::vector<std::string> GetGroupRules(std::string groupName);

        std::vector<std::string> GetEnslavementRules();

        void ShowBuyoutMenu();

        [[nodiscard]] inline bool IsBuyoutSelected() { return menuChosen; }
        
        std::string GetBuyoutMenuResult();

        static void OnRevert(SKSE::SerializationInterface*);

        static void OnGameSaved(SKSE::SerializationInterface* serde);

        static void OnGameLoaded(SKSE::SerializationInterface* serde);

    private:
        DealManager() = default;
        
        std::string GetNextDealName();
        
        mutable std::mutex _lock;


        std::unordered_map<int, std::string> id_map;
        std::unordered_map<std::string, int> name_map;

        std::unordered_map<std::string, Rule> rules; // a list of all rules
        std::unordered_map<std::string, std::vector<Rule*>> ruleGroups; // what add on each rule is from

        std::unordered_map<std::string, Deal> deals;  // all active deals containing rules

        std::unordered_map<Rule*, std::unordered_set<Rule*>> conflicts; // every conflict between every rule generated on startup

        bool menuChosen;
        std::string chosenDeal;

        std::vector<std::string> allDealNames{
            "Skeever", 
            "Bear", 
            "Wolf", 
            "Slaughterfish", 
            "Deer", 
            "Troll", 
            "Mudcrab", 
            "Spider", 
            "Hopper", 
            "Cat",  
            "Dog",           
            "Bat",           
            "Clam",      
            "Rabbit", 
            "Fox",     
            "Oyster",
            "Hawk", 
            "Tern", 
            "Cow",  
            "Moth", 
            "Butterfly", 
            "Salmon", 
            "Chaurus", 
            "Dragon", 
            "Draugr",  
            "Automaton", 
            "Giant", 
            "Hagraven",
            "Wisp"
        };

        friend class Deal;
    };
#pragma warning(pop)
}
