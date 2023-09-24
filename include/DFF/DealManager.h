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

        void InitQuestData();

        int SelectDeal(int lastRejectedId);

        void ActivateRule(int id);

        void RemoveDeal(std::string name);

        std::vector<std::string> GetActiveDeals();

        std::vector<std::string> GetDealRules(std::string name);

        std::vector<std::string> GetGroupNames();

        std::vector<std::string> GetGroupRules(std::string groupName);

        void ShowBuyoutMenu();

        [[nodiscard]] inline bool IsBuyoutSelected() { return menuChosen; }
        
        int GetBuyoutMenuResult();

        static void OnRevert(SKSE::SerializationInterface*);

        static void OnGameSaved(SKSE::SerializationInterface* serde);

        static void OnGameLoaded(SKSE::SerializationInterface* serde);

    private:
        DealManager() = default;
        
        std::string GetNextDealName();
        
        mutable std::mutex _lock;

        std::unordered_map<int, std::string> id_map;

        std::unordered_map<std::string, Rule> rules; // a list of all rules
        std::unordered_map<std::string, std::vector<Rule*>> ruleGroups; // what add on each rule is from

        std::unordered_map<std::string, Deal> activeDeals;  // all active deals containing rules

        std::unordered_map<Rule*, std::unordered_set<Rule*>> conflicts; // every conflict between every rule generated on startup

        bool menuChosen;
        int chosenCost;

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
    };
#pragma warning(pop)
}
