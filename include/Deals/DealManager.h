#pragma once

#include <RE/Skyrim.h>
#include <Deals/Deal.h>
#include <Deals/Rule.h>

namespace DFF {
#pragma warning(push)
#pragma warning(disable : 4251)
    class __declspec(dllexport) DealManager {
    public:
        [[nodiscard]] static DealManager& GetSingleton() noexcept;

        void Init();

        std::string SelectRule(std::string lastRejected);

        bool CanEnableRule(std::string rule);
        
        bool CanDisableRule(std::string rule);

        int ActivateRule(std::string name, std::string a_dealName = "", bool a_create = false);
        
        void RemoveDeal(std::string name);

        void SetDealLockIn(std::string a_name, bool a_lockIn);

        void ResetAllDeals();

        void Pause();

        void Resume();

        void ExtendDeal(std::string deal, double by);

        std::string GetRandomDeal();

        RE::TESGlobal* GetRuleGlobal(std::string path);
        
        std::string GetRuleName(std::string path);

        std::string GetRuleHint(std::string path);
        
        std::string GetRuleInfo(std::string path);
        
        std::string GetRulePack(std::string path);
        
        int GetDealCost(std::string name);

        std::vector<std::string> GetDeals();

        std::vector<std::string> GetDealRules(std::string name);

        std::vector<std::string> GetPackNames();

        std::vector<std::string> GetPackRules(std::string PackName);

        std::vector<std::string> GetEnslavementRules();

        void ShowBuyoutMenu();

        [[nodiscard]] inline bool IsBuyoutSelected() { return menuChosen; }
        
        std::string GetBuyoutMenuResult();

        void SetRuleValid(std::string path, bool valid);

        RE::TESQuest* GetPackQuest(std::string path);

        static void OnRevert(SKSE::SerializationInterface*);

        static void OnGameSaved(SKSE::SerializationInterface* serde);

        static void OnGameLoaded(SKSE::SerializationInterface* serde);

    private:
        DealManager() = default;

        Deal* CreateDeal(std::string name);
        Rule* GetRuleByPath(std::string path);
        Deal* GetDealByName(std::string path);
        Pack* GetPackByName(std::string path);
        std::string GetNextDealName();
        int CalculateRuleWeight(Rule* rule, int targetSeverity);
        int CalculateTargetSeverity();

        mutable std::mutex _lock;

        std::unordered_map<std::string, Rule> rules; // a list of all rules
        std::unordered_map<std::string, Pack> packs; // what add on each rule is from

        std::unordered_map<std::string, Deal> deals;  // all active deals containing rules

        std::unordered_map<Rule*, std::unordered_set<Rule*>> conflicts; // every conflict between every rule
        std::unordered_map<Rule*, std::unordered_set<Rule*>> predecessors; // every rule's neighbors used for rule pathing

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
