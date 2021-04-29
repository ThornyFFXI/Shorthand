#include "Shorthand.h"

void Shorthand::InitializeSpells()
{
	//If we're not ingame yet, we don't want to check the spell memory because it may not be initialized or accurate.
	uint16_t myIndex = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
	if (myIndex == 0) return;
	if (((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(myIndex) & 0x200) == 0)
		|| ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(myIndex) & 0x4000))) return;

	//We know we're ingame, so we can fill in what we know.
	mState.KnowsDispelga = m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(360);
	mState.KnowsHonorMarch = m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(417);
	mState.KnowsImpact = m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(503);

	//Once we fill in what we know, we can apply our overrides if applicable.
	if ((!mState.KnowsDispelga) && (mSettings.UnlockDispelga)) SetSpellLearned(360, true);
	if ((!mState.KnowsHonorMarch) && (mSettings.UnlockHonorMarch)) SetSpellLearned(417, true);
	if ((!mState.KnowsImpact) && (mSettings.UnlockImpact)) SetSpellLearned(503, true);
}
void Shorthand::SetSpellLearned(uint32_t id, bool learned)
{	
	const auto o = m_AshitaCore->GetOffsetManager();
	const auto pointer = m_AshitaCore->GetPointerManager()->Get(u8"player.hasspell");
	const auto offset1 = o->Get(u8"player.hasspell", u8"offset1");
	const auto offset2 = o->Get(u8"player.hasspell", u8"offset2");

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
		if (!isdigit(input[0])) return false;
	}
	return true;
}
const char* Shorthand::strcasestr(const char* _Str, const char* _SubStr)
{
	const char* a, * b;

	for (; *_Str; _Str++) {

		a = _Str;
		b = _SubStr;

		while ((*a++ | 32) == (*b++ | 32))
			if (!*b)
				return (_Str);

	}
	return(NULL);
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
		if (buffs[x] == id) return true;
	}
	return false;
}
bool Shorthand::CheckForEquippableItem(uint16_t Id)
{
	for (std::list<int>::iterator iter = mSettings.EquipBags.begin(); iter != mSettings.EquipBags.end(); iter++)
	{
		for (int x = 1; x < m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerCountMax(*iter); x++)
		{
			Ashita::FFXI::item_t* pItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(*iter, x);
			if ((pItem->Id == Id) && (pItem->Count > 0)) return true;
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
			int index = Read8(&equipData->Index, 0);
			int container = Read8(&equipData->Index, 1);
			if (index == 0) continue;
			Ashita::FFXI::item_t* pItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(container, index);
			if ((pItem != NULL) && (pItem->Id == Id) && (pItem->Count > 0) && (pItem->Extra[3] & 0x40)) return true;
		}
	}
	else
	{
		for (std::list<int>::iterator iter = mSettings.UsableBags.begin(); iter != mSettings.UsableBags.end(); iter++)
		{
			for (int x = 1; x < m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerCountMax(*iter); x++)
			{
				Ashita::FFXI::item_t* pItem = m_AshitaCore->GetMemoryManager()->GetInventory()->GetContainerItem(*iter, x);
				if ((pItem != NULL) && (pItem->Id == Id) && (pItem->Count > 0)) return true;
			}
		}
	}
	return false;
}

bool Shorthand::IsValidTarget(uint16_t validFlags, int index)
{
	if (m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(index) == NULL) return false;
	if ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(index) & 0x200) == 0) return false;
	if ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(index) & 0x4000) != 0) return false;

	if (validFlags == UINT16_MAX) return true; //Custom flags for target.

	if ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetHPPercent(index) == 0) != (validFlags == 157)) return false; //157 = raise/tractor rules
	if (validFlags == 1) return (m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0) == index); //1 = self
	unsigned int FullFlags = m_AshitaCore->GetMemoryManager()->GetEntity()->GetSpawnFlags(index);
	unsigned char Flags = FullFlags & 0xFF;
	if (validFlags == 5) return ((Flags == 0x0D) || (FullFlags == 4366)); //5 = party
	else if (validFlags == 29) return ((Flags == 0x01) || (Flags == 0x09) || (Flags == 0x0D) || (FullFlags == 4366)); //29 = player
	else if (validFlags == 32) return (Flags == 0x10); //32 = enemy
	else if (validFlags == 63) return ((Flags == 0x01) || (Flags == 0x09) || (Flags == 0x10) || (Flags == 0x0D) || (FullFlags == 4366)); //63 = player or mob
	else if (validFlags == 157) return ((Flags == 0x01) || (Flags == 0x09) || (Flags == 0x0D)); //157 = dead player
	return false;
}
bool Shorthand::RelaxedMatch(std::string input, std::string compare)
{
	const char* lhs = input.c_str();
	const char* rhs = compare.c_str();

	while (lhs[0] != 0x00)
	{
		//Advance left side as long as it's not something that needs to be matched.
		while ((!isalnum(lhs[0]))
			&& (lhs[0] != 0x00)) lhs++;

		//Advance right hand side as long as it's not something that needs to be matched
		while ((!isalnum(rhs[0]))
			&& (rhs[0] != 0x00)) rhs++;

		//Break if either is finished.
		if ((lhs[0] == 0x00)
			|| (rhs[0] == 0x00))
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
		if (!CheckRoman(&lhs, &rhs)) return false;
	}
	while ((!isalnum(rhs[0]))
		&& (rhs[0] != 0x00)) rhs++;
	if ((rhs[0] == 0x00) && (lhs[0] == '1')) lhs++;
	return ((lhs[0] == 0x00) && (rhs[0] == 0x00));
}
bool Shorthand::CheckRoman(const char** lhs, const char** rhs)
{
	if ((*lhs)[0] == '2')
	{
		if (strncmp(*rhs, "II", 2) == 0)
		{
			(*lhs)++;
			(*rhs) += 2;
			return true;
		}
	}

	else if ((*lhs)[0] == '3')
	{
		if (strncmp(*rhs, "III", 3) == 0)
		{
			(*lhs)++;
			(*rhs) += 3;
			return true;
		}
	}

	else if ((*lhs)[0] == '4')
	{
		if (strncmp(*rhs, "IV", 2) == 0)
		{
			(*lhs)++;
			(*rhs) += 2;
			return true;
		}
	}

	else if ((*lhs)[0] == '5')
	{
		if ((*rhs)[0] == 'V')
		{
			(*lhs)++;
			(*rhs)++;
			return true;
		}
	}

	else if ((*lhs)[0] == '6')
	{
		if (strncmp(*rhs, "VI", 2) == 0)
		{
			(*lhs)++;
			(*rhs) += 2;
			return true;
		}
	}

	else if ((*lhs)[0] == '7')
	{
		if (strncmp(*rhs, "VII", 3) == 0)
		{
			(*lhs)++;
			(*rhs) += 3;
			return true;
		}
	}


	else if ((*lhs)[0] == '8')
	{
		if (strncmp(*rhs, "VIII", 4) == 0)
		{
			(*lhs)++;
			(*rhs) += 4;
			return true;
		}
	}


	else if ((*lhs)[0] == '9')
	{
		if (strncmp(*rhs, "IX", 2) == 0)
		{
			(*lhs)++;
			(*rhs) += 2;
			return true;
		}
	}

	return false;
}