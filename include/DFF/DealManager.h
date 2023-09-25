#pragma once

#include <RE/Skyrim.h>
#include <DFF/Deal.h>
#include <DFF/Rule.h>

namespace DFF {
#pragma warning(push)
#pragma warning(disable : 4251)
    class __declspec(dllexport) DealManager {
    public:
        [[nodiscard]] static DealManager& GetSingleton() noexcept;

        void InitDeals();

        void InitQuests();

        std::string SelectRule(std::string lastRejected);

        bool CanEnableRule(std::string rule);
        
        bool CanDisableRule(std::string rule);

        int ActivateRule(std::string name);

        void RemoveDeal(std::string name);

        void ResetAllDeals();

        void Pause();

        void Resume();

        void ExtendDeal(std::string deal, double by);

        std::string GetRandomDeal();

        RE::TESGlobal* GetRuleGlobal(std::string path);
        
        std::string GetRuleName(std::string path);

        std::string GetRuleDesc(std::string path);

        std::string GetRuleHint(std::string path);
        
        int GetDealCost(std::string name);

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

        Rule* GetRuleByPath(std::string path);
        Deal* GetDealByName(std::string path);
        std::string GetNextDealName();
        
        mutable std::mutex _lock;

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
