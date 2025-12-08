#include "Shorthand.h"
#pragma comment(lib, "psapi.lib")
#include <psapi.h>

__declspec(dllexport) IPlugin* __stdcall expCreatePlugin(const char* args)
{
    UNREFERENCED_PARAMETER(args);

    return (IPlugin*)(new Shorthand());
}

__declspec(dllexport) void __stdcall expDestroyPlugin(void* instance)
{
    delete static_cast<Shorthand*>(instance);
}

__declspec(dllexport) double __stdcall expGetInterfaceVersion(void)
{
    return ASHITA_INTERFACE_VERSION;
}

bool Shorthand::Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id)
{
	this->m_AshitaCore = core;
	this->m_LogManager = logger;
	this->m_PluginId   = id;
	pOutput = new OutputHelpers(core, logger, this->GetName());
    pSettings          = new SettingsHelper(core, pOutput, this->GetName());

    if (m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberIsActive(0))
    {
        mState.CharacterName = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberName(0);
    }
    else
    {
        mState.CharacterName = "NO_NAME";
    }

	LoadSettingsXml(false);
	InitializeSpells();

    MODULEINFO mod = {0};
    ::GetModuleInformation(::GetCurrentProcess(), ::GetModuleHandle("FFXiMain.dll"), &mod, sizeof(MODULEINFO));
    pWardrobe = Ashita::Memory::FindPattern((uintptr_t)mod.lpBaseOfDll, (uintptr_t)mod.SizeOfImage, "A1????????8B88B4000000C1E907F6C101E9", 1, 0);
    if (!pWardrobe)
        return false;

	return true;
}

void Shorthand::Release(void)
{
    delete pSettings;
	delete pOutput;
}

bool Shorthand::HandleCommand(int32_t mode, const char* command, bool injected)
{
	UNREFERENCED_PARAMETER(injected);

	//If debug mode enabled, log everything that goes through HandleCommand.
	if (mSettings.Debug)
	{		
		pOutput->debug_f("%s(Mode:%d) %s", injected ? "[Injected] " : "", mode, command);
	}

	//Don't process anything that's part of a <st> or menu mode.
	if ((mode == 0) || (mode > 2)) return false;

	//If we start with /raw, trim it and end.
	if (strncmp(command, "/raw ", 5) == 0)
	{
		int newLength = strlen(command) - 5;
		if (newLength < 1) return true;
		memcpy((void*)command, command + 5, newLength);
		memset((void*)(command + newLength), 0, 5);
		return false;
	}

	std::vector<string> args;
	int argcount = Ashita::Commands::GetCommandArgs(command, &args);
	if ((CheckArg(0, "/shh")) || (CheckArg(0, "/shorthand")))
	{
		if (CheckArg(1, "debug"))
		{
			if (CheckArg(2, "on"))
				mSettings.Debug = true;
			else if (CheckArg(2, "off"))
				mSettings.Debug = false;
			else
				mSettings.Debug = !mSettings.Debug;

			pOutput->message_f("Debug log $H%s$R.", mSettings.Debug ? "enabled" : "disabled");
		}

		else if (CheckArg(1, "impact"))
		{
			if (CheckArg(2, "on"))
				mSettings.UnlockImpact = true;
			else if (CheckArg(2, "off"))
				mSettings.UnlockImpact = false;
			else
				mSettings.UnlockImpact = !mSettings.UnlockImpact;

			pOutput->message_f("Impact override $H%s$R.", mSettings.UnlockImpact ? "enabled" : "disabled");
			if (!mState.KnowsImpact) SetSpellLearned(503, mSettings.UnlockImpact);
		}

		else if (CheckArg(1, "dispelga"))
		{
			if (CheckArg(2, "on"))
				mSettings.UnlockDispelga = true;
			else if (CheckArg(2, "off"))
				mSettings.UnlockDispelga = false;
			else
				mSettings.UnlockDispelga = !mSettings.UnlockDispelga;

			pOutput->message_f("Dispelga override $H%s$R.", mSettings.UnlockDispelga ? "enabled" : "disabled");
			if (!mState.KnowsDispelga) SetSpellLearned(360, mSettings.UnlockDispelga);
		}

		else if (CheckArg(1, "honor"))
		{
			if (CheckArg(2, "on"))
				mSettings.UnlockHonorMarch = true;
			else if (CheckArg(2, "off"))
				mSettings.UnlockHonorMarch = false;
			else
				mSettings.UnlockHonorMarch = !mSettings.UnlockHonorMarch;

			pOutput->message_f("Honor March override $H%s$R.", mSettings.UnlockHonorMarch ? "enabled" : "disabled");
			if (!mState.KnowsHonorMarch) SetSpellLearned(417, mSettings.UnlockHonorMarch);
		}

		else if (CheckArg(1, "aria"))
		{
			if (CheckArg(2, "on"))
				mSettings.UnlockAriaOfPassion = true;
			else if (CheckArg(2, "off"))
				mSettings.UnlockAriaOfPassion = false;
			else
				mSettings.UnlockAriaOfPassion = !mSettings.UnlockAriaOfPassion;

			pOutput->message_f("Aria of Passion override $H%s$R.", mSettings.UnlockAriaOfPassion ? "enabled" : "disabled");
			if (!mState.KnowsAriaOfPassion) SetSpellLearned(418, mSettings.UnlockAriaOfPassion);
		}

		else if (CheckArg(1, "export"))
		{
			CreateSettingsXml(false);
		}

		else if (CheckArg(1, "reload"))
		{
			LoadSettingsXml(true);
		}

		else if (CheckArg(1, "packetws"))
		{
			if (CheckArg(2, "on"))
				mSettings.PacketWs = true;
			else if (CheckArg(2, "off"))
				mSettings.PacketWs = false;
			else
				mSettings.PacketWs = !mSettings.PacketWs;

			pOutput->message_f("Packet based weaponskills $H%s$R.", mSettings.PacketWs ? "enabled" : "disabled");
		}

		return true;
	}

	return ProcessInput(command);
}

bool Shorthand::HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked)
{
	UNREFERENCED_PARAMETER(size);
	UNREFERENCED_PARAMETER(sizeChunk);
	UNREFERENCED_PARAMETER(dataChunk);
	UNREFERENCED_PARAMETER(injected);
	UNREFERENCED_PARAMETER(blocked);

    if (id == 0x00A)
    {
        string currentName((const char*)data + 0x84);

        if (currentName != mState.CharacterName)
        {
            mState.CharacterName = currentName;
            LoadSettingsXml(false);
        }
    }

	//Use incoming spell packet to check if we have items.
	if (id == 0xAA)
	{
		//Set these.  If the game says we know it, we don't need to later check if we have the gear.
		mState.KnowsAriaOfPassion = (Read8(data, 0x038) & 0x04);
		mState.KnowsHonorMarch = (Read8(data, 0x38) & 0x02);
		mState.KnowsDispelga = (Read8(data, 0x31) & 0x01);
		mState.KnowsImpact = (Read8(data, 0x42) & 0x80);

		//If we have overrides in settings, adjust the packet so the game will show that we know it.
		if (mSettings.UnlockImpact) Write8(modified, 0x42) |= 0x80;
		if (mSettings.UnlockDispelga) Write8(modified, 0x31) |= 0x01;
		if (mSettings.UnlockHonorMarch) Write8(modified, 0x38) |= 0x02;
		if (mSettings.UnlockAriaOfPassion) Write8(modified, 0x38) |= 0x04;
	}

	return false;
}

bool Shorthand::HandleOutgoingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked)
{
	if (id == 0x1A)
	{
		//Set overrides so we allow these spells to be targeted to others as soon as the abilities are sent out, as buff takes time to appear.
		if (Read16(data, 10) == 0x09) //JA
		{
			if (Read16(data, 12) == 386) //Entrust
			{
				mState.OverrideEntrust = std::chrono::steady_clock::now() + std::chrono::seconds(3);
			}
			if (Read16(data, 12) == 229) //Pianissimo
			{
				mState.OverridePianissimo = std::chrono::steady_clock::now() + std::chrono::seconds(3);
			}
		}

		//If we have safety checks enabled and this is a spell, confirm we have the gear that unlocks the spell.
		if ((Read16(data, 10) == 3) && (mSettings.CheckInventoryForGear))
		{
			if ((Read16(data, 12) == 503) && (!mState.KnowsImpact))
			{
				if ((!CheckForEquippableItem(11363)) && (!CheckForEquippableItem(23799)))
				{
					pOutput->error("No twilight or crepuscular cloak in equippable bags.  Impact packet blocked.");
					return true;
				}
			}

			if ((Read16(data, 12) == 360) && (!mState.KnowsDispelga))
			{
				if (!CheckForEquippableItem(22040))
				{
					pOutput->error("No daybreak in equippable bags.  Dispelga packet blocked.");
					return true;
				}
			}

			if ((Read16(data, 12) == 417) && (!mState.KnowsHonorMarch))
			{
				if (!CheckForEquippableItem(21398))
				{
					pOutput->error("No marsyas in equippable bags.  Honor March packet blocked.");
					return true;
				}
			}

			if ((Read16(data, 12) == 418) && (!mState.KnowsAriaOfPassion))
			{
				uint16_t zoneId = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberZone(0);
				if (!(CheckForEquippableItem(22305) && (zoneId == 275 || zoneId == 133 || zoneId == 189)) || CheckForEquippableItem(22306) || CheckForEquippableItem(22307))
				{
					pOutput->error("No Loughnashade in equippable bags.  Aria of Passion packet blocked.");
					return true;
				}
			}
		}
	}

	return false;
}