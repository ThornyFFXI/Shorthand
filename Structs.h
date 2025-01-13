#ifndef __ASHITA_Shorthand_Structs_H_INCLUDED__
#define __ASHITA_Shorthand_Structs_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <chrono>
#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include "common\Utilities.h"

enum class ActionFlags : uint32_t
{
    Spell = 1,
    Weaponskill = 2,
    Ability = 4,
    Item = 8,
    Attack = 16,
    Ranged = 32,
    Target = 64,

    Unknown = Spell | Weaponskill | Ability | Item
};
struct actioninfo_t
{
    uint32_t Type;
    uint16_t Id;
    std::string Target;

    actioninfo_t(uint32_t Type, uint16_t Id, std::string Target) : Type(Type), Id(Id), Target(Target) {};
};
struct charstate_t
{
    std::string CharacterName;
    std::chrono::time_point<std::chrono::steady_clock> OverrideEntrust;
    std::chrono::time_point<std::chrono::steady_clock> OverridePianissimo;
    bool                KnowsDispelga;
    bool                KnowsImpact;
    bool                KnowsHonorMarch;
    charstate_t() :
        OverrideEntrust(std::chrono::steady_clock::now() - std::chrono::seconds(1)),
        OverridePianissimo(std::chrono::steady_clock::now() - std::chrono::seconds(1)),
        KnowsDispelga(false),
        KnowsImpact(false),
        KnowsHonorMarch(false),
    CharacterName(std::string("NO_NAME")) {}
};
struct settings_t
{
    std::map<std::string, uint16_t> AbilityMap;
    std::map<std::string, uint16_t> ItemMap;
    std::map<std::string, uint16_t> SpellMap;
    std::map<std::string, uint16_t> WeaponskillMap;
    std::map<std::string, actioninfo_t> AliasMap;

    std::list<int>      EquipBags;
    std::list<int>      UsableBags;
    bool                PacketWs;
    bool                CheckInventoryForGear;
    bool				UnlockDispelga;
    bool				UnlockImpact;
    bool				UnlockHonorMarch;
    bool                Debug;

    settings_t() {}
    settings_t(IAshitaCore* m_AshitaCore) :
        EquipBags(std::list<int>{0, 8, 10, 11, 12, 13, 14, 15, 16}),
        UsableBags(std::list<int> {0, 3}),
        PacketWs(false),
        CheckInventoryForGear(true),
        UnlockDispelga(false),
        UnlockImpact(false),
        UnlockHonorMarch(false),
        Debug(false) {
            AbilityMap = std::map<std::string, uint16_t>();
            for (uint16_t x = 512; x < 1512; x++)
            {
                IAbility* Ability = m_AshitaCore->GetResourceManager()->GetAbilityById(x);
                if ((Ability)
                    && (strlen(Ability->Name[0]) > 1))
                {
                    AbilityMap.insert(std::make_pair(Ability->Name[0], x - 512));
                }
            }
            ItemMap = std::map<std::string, uint16_t>();
            for (uint16_t x = 0; x < 65535; x++)
            {
                IItem* item = m_AshitaCore->GetResourceManager()->GetItemById(x);
                if ((item)
                    && (item->Name[0])
                    && (strlen(item->Name[0]) > 1)
                    && ((item->Flags & 0x0200) || (item->Flags & 0x0400))) //Only usable or enchanted items.
                {
                    ItemMap.insert(std::make_pair(item->Name[0], x));
                }
            }
            SpellMap = std::map<std::string, uint16_t>();
            for (uint16_t x = 0; x < 2048; x++)
            {
                ISpell* Spell = m_AshitaCore->GetResourceManager()->GetSpellById(x);
                if ((Spell)
                    && (strlen(Spell->Name[0]) > 1))
                {
                    SpellMap.insert(std::make_pair(Spell->Name[0], x));
                }
            }
            WeaponskillMap = std::map<std::string, uint16_t>();
            for (uint16_t x = 0; x < 512; x++)
            {
                IAbility* Ability = m_AshitaCore->GetResourceManager()->GetAbilityById(x);
                if ((Ability)
                    && (strlen(Ability->Name[0]) > 1))
                {
                    WeaponskillMap.insert(std::make_pair(Ability->Name[0], x));
                }
            }
            AliasMap = std::map<std::string, actioninfo_t>();
    };
};
struct pk_Action
{
    uint32_t Header;
    uint32_t TargetId;
    uint16_t TargetIndex;
    uint16_t Category;
    uint16_t AbilityId;
    uint16_t Padding;
    float_t PosX;
    float_t PosY;
    float_t PosZ;
};
#endif