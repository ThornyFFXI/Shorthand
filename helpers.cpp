#include "Shorthand.h"

void Shorthand::InitializeSpells()
{
    //If we're not ingame yet, we don't want to check the spell memory because it may not be initialized or accurate.
    uint16_t myIndex = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
    if (myIndex == 0)
        return;
    if (((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(myIndex) & 0x200) == 0) || ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(myIndex) & 0x4000)))
        return;

    //We know we're ingame, so we can fill in what we know.
    mState.KnowsDispelga   = m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(360);
    mState.KnowsHonorMarch = m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(417);
    mState.KnowsImpact     = m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(503);

    //Once we fill in what we know, we can apply our overrides if applicable.
    if ((!mState.KnowsDispelga) && (mSettings.UnlockDispelga))
        SetSpellLearned(360, true);
    if ((!mState.KnowsHonorMarch) && (mSettings.UnlockHonorMarch))
        SetSpellLearned(417, true);
    if ((!mState.KnowsImpact) && (mSettings.UnlockImpact))
        SetSpellLearned(503, true);
}
void Shorthand::SetSpellLearned(uint32_t id, bool learned)
{
    //If we're not ingame yet, we don't want to edit the spell memory.
    uint16_t myIndex = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
    if (myIndex == 0)
        return;
    if (((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(myIndex) & 0x200) == 0) || ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(myIndex) & 0x4000)))
        return;

    const auto o       = m_AshitaCore->GetOffsetManager();
    const auto pointer = m_AshitaCore->GetPointerManager()->Get("player.hasspell");
    const auto offset1 = o->Get("player.hasspell", "offset1");
    const auto offset2 = o->Get("player.hasspell", "offset3");

    if (pointer == 0)
        return;

    const auto ptr = *(uintptr_t*)(pointer + offset1);
    const auto off = *(uint32_t*)(pointer + offset2);

    if (ptr == 0 || off == 0)
        return;

    const auto buffer = (uint8_t*)(*(uintptr_t*)ptr + off);

    const auto v1 = 1 << id % 8;

    if (learned)
        buffer[id / 8] |= v1;
    else
        buffer[id / 8] &= ~v1;
}
bool Shorthand::IsPositiveInteger(const char* input)
{
    for (; input[0]; input++)
    {
        if (!isdigit(input[0]))
            return false;
    }
    return true;
}
const char* Shorthand::strcasestr(const char* _Str, const char* _SubStr)
{
    const char *a, *b;

    for (; *_Str; _Str++)
    {

        a = _Str;
        b = _SubStr;

        while ((*a++ | 32) == (*b++ | 32))
            if (!*b)
                return (_Str);
    }
    return (NULL);
}
uint16_t Shorthand::GetValidFlags(actioninfo_t* action)
{
    if (action->Type == (uint32_t)ActionFlags::Ability)
        return m_AshitaCore->GetResourceManager()->GetAbilityById(action->Id + 512)->Targets;

    else if (action->Type == (uint32_t)ActionFlags::Spell)
    {
        ISpell* spell = m_AshitaCore->GetResourceManager()->GetSpellById(action->Id);

        //Adjust valid targets for buff songs if we have pianissimo active.
        if ((spell->Type == 0x05) && (spell->Targets == 1))
        {
            if ((HasBuff(409)) || (std::chrono::steady_clock::now() < mState.OverridePianissimo))
                return 5;
        }

        //Adjust valid targets for indi spells if we have entrust active.
        if (strncmp(spell->Name[0], "Indi-", 5) == 0)
        {
            if ((HasBuff(584)) || (std::chrono::steady_clock::now() < mState.OverrideEntrust))
                return 5;
        }

        return spell->Targets;
    }

    else if (action->Type == (uint32_t)ActionFlags::Weaponskill)
        return m_AshitaCore->GetResourceManager()->GetAbilityById(action->Id)->Targets;

    else if (action->Type == (uint32_t)ActionFlags::Item)
        return m_AshitaCore->GetResourceManager()->GetItemById(action->Id)->Targets;

    else if ((action->Type == (uint32_t)ActionFlags::Attack) || (action->Type == (uint32_t)ActionFlags::Ranged))
        return 32;

    else if (action->Type == (uint32_t)ActionFlags::Target)
        return UINT16_MAX;

    return 0;
}
bool Shorthand::HasBuff(int16_t id)
{
    int16_t* buffs = m_AshitaCore->GetMemoryManager()->GetPlayer()->GetBuffs();
    for (int x = 0; x < 32; x++)
    {
        if (buffs[x] == id)
            return true;
    }
    return false;
}
bool Shorthand::CheckBagEnabled(int index)
{
    if (index > 10)
    {
        DWORD Memloc = Read32(pWardrobe, 0);
        Memloc       = Read32(Memloc, 0);
        uint8_t flag = Read8(Memloc, 0xB4);
        return ((flag & (uint8_t)pow(2, index - 9)) != 0);
    }
    return true;
}
bool Shorthand::CheckForEquippableItem(uint16_t Id)
{
    for (std::list<int>::iterator iter = mSettings.EquipBags.begin(); iter != mSettings.EquipBags.end(); iter++)
    {
        if (CheckBagEnabled(*iter))
        {
            for (int x = 1; x < m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerCountMax(*iter); x++)
            {
                Ashita::FFXI::item_t* pItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(*iter, x);
                if ((pItem->Id == Id) && (pItem->Count > 0))
                    return true;
            }
        }
    }
    return false;
}
bool Shorthand::CheckForUsableItem(uint16_t Id)
{
    IItem* pResource = m_AshitaCore->GetResourceManager()->GetItemById(Id);
    if (pResource->Flags & 0x0800)
    {
        for (int x = 0; x < 16; x++)
        {
            Ashita::FFXI::equipmententry_t* equipData = m_AshitaCore->GetMemoryManager()->GetInventory()->GetEquippedItem(x);
            int index                                 = Read8(&equipData->Index, 0);
            int container                             = Read8(&equipData->Index, 1);
            if (index == 0)
                continue;
            Ashita::FFXI::item_t* pItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(container, index);
            if ((pItem != NULL) && (pItem->Id == Id) && (pItem->Count > 0) && (pItem->Extra[3] & 0x40))
                return true;
        }
    }
    else
    {
        for (std::list<int>::iterator iter = mSettings.UsableBags.begin(); iter != mSettings.UsableBags.end(); iter++)
        {
            for (int x = 1; x < min(81, m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerCountMax(*iter)); x++)
            {
                Ashita::FFXI::item_t* pItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(*iter, x);
                if ((pItem != NULL) && (pItem->Id == Id) && (pItem->Count > 0))
                    return true;
            }
        }
    }
    return false;
}

bool Shorthand::IsValidTarget(uint16_t validFlags, int index)
{
    auto entity = m_AshitaCore->GetMemoryManager()->GetEntity();

    // Disregard anything that's not rendered and targetable.
    if (entity->GetRawEntity(index) == NULL)
        return false;
    if ((entity->GetRenderFlags0(index) & 0x200) == 0)
        return false;
    if ((entity->GetRenderFlags0(index) & 0x4000) != 0)
        return false;

    auto myIndex      = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
    auto myAllegiance = entity->GetBallistaFlags(myIndex);
    auto flags        = entity->GetSpawnFlags(index);
    auto allegiance   = entity->GetBallistaFlags(index);
    auto health       = entity->GetHPPercent(index);

    // Custom validFlags for /target
    if (validFlags == UINT16_MAX)
    {
        // Can't target anything that's dead and on team monster.
        if ((health == 0) && (allegiance == 0))
            return false;

        return true;
    }

    // Don't target living entities for raise/tractor or dead entities for anything else..
    if (((validFlags & 0x80) != 0) != (health == 0))
        return false;

    // Self target..
    if ((validFlags & 0x01) && (flags & 0x200))
        return true;

    // Pet target..
    if ((validFlags & 0x02) && (flags & 0x0100))
    {
        // This only applies to your own pets.
        if (index == entity->GetPetTargetIndex(myIndex))
            return true;
    }

    // Party target..
    if ((validFlags & 0x04) && (flags & 0x04))
    {
        // Can't do this if person is charmed..
        if (allegiance == myAllegiance)
            return true;
    }

    // Alliance target..
    if ((validFlags & 0x08) && (flags & 0x08))
    {
        // Can't do this if person is charmed..
        if (allegiance == myAllegiance)
            return true;
    }

    // Friendly target..
    if ((validFlags & 0x10) && (allegiance == myAllegiance))
    {
        // Can cast on players and NPCs that are allied with self..
        if (flags & 0x03)
            return true;

        // Can cast on allied battle entities that aren't pets.. (helper npcs)
        if ((flags & 0x10) && ((flags & 0x100) == 0))
            return true;
    }

    // Enemy target..
    if ((validFlags & 0x20) && (allegiance != myAllegiance))
    {
        // Can cast on players that are charmed or on a different ballista team..
        if (flags & 0x01)
            return true;

        // Can cast on monsters that aren't currently pets..
        if ((flags & 0x10) && ((flags & 0x100) == 0))
            return true;
    }

    // NPC tradable..
    if ((validFlags & 0x40) && (flags & 0x02))
        return true;

    return false;
}
bool Shorthand::RelaxedMatch(std::string input, std::string compare)
{
    const char* lhs = input.c_str();
    const char* rhs = compare.c_str();

    while (lhs[0] != 0x00)
    {
        //Advance left side as long as it's not something that needs to be matched.
        while ((!isalnum(lhs[0])) && (lhs[0] != 0x00))
            lhs++;

        //Advance right hand side as long as it's not something that needs to be matched
        while ((!isalnum(rhs[0])) && (rhs[0] != 0x00))
            rhs++;

        //Break if either is finished.
        if ((lhs[0] == 0x00) || (rhs[0] == 0x00))
        {
            break;
        }

        //Advance both if they match.
        if ((lhs[0] | 32) == (rhs[0] | 32))
        {
            lhs++;
            rhs++;
            continue;
        }

        //Roman Numerals
        if (!CheckRoman(&lhs, &rhs))
            return false;
    }

    //Advance right hand side as long as it's not something that needs to be matched
    while ((!isalnum(rhs[0])) && (rhs[0] != 0x00))
        rhs++;

    // Special case for trailing 1 on input  [cure1 for "Cure", thunder1 for "Thunder", etc..]
    if ((rhs[0] == 0x00) && (lhs[0] == '1') && (lhs[1] == 0x00))
        return true;

    return ((lhs[0] == 0x00) && (rhs[0] == 0x00));
}

std::map<char, const char*> romanNumerals = {
    {'2', "II"},
    {'3', "III"},
    {'4', "IV"},
    {'5', "V"},
    {'6', "VI"},
    {'7', "VII"},
    {'8', "VIII"},
    {'9', "IX"}};
bool Shorthand::CheckRoman(const char** lhs, const char** rhs)
{
    for (auto iter = romanNumerals.begin(); iter != romanNumerals.end(); iter++)
    {
        if ((*lhs)[0] == iter->first)
        {
            auto len = strlen(iter->second);
            if (strncmp(*rhs, iter->second, len) == 0)
            {
                (*lhs)++;
                (*rhs) += len;
                return true;
            }
        }
    }
    return false;
}