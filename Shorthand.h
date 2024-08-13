#ifndef __ASHITA_Shorthand_H_INCLUDED__
#define __ASHITA_Shorthand_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Ashita 4\plugins\sdk\Ashita.h"
#include "..\common\Output.h"
#include "..\common\Settings.h"
#include "Structs.h"
using namespace std;

class Shorthand : IPlugin
{
private:
    IAshitaCore* m_AshitaCore;
    ILogManager* m_LogManager;
    uint32_t m_PluginId;
    charstate_t         mState;
    settings_t          mSettings;
    OutputHelpers*      pOutput;
    SettingsHelper* pSettings;
    DWORD pWardrobe;

public:
    const char* GetName(void) const override
    {
        return "Shorthand";
    }
    const char* GetAuthor(void) const override
    {
        return "Thorny";
    }
    const char* GetDescription(void) const override
    {
        return "Insert description here.";
    }
    const char* GetLink(void) const override
    {
        return "Insert link here.";
    }
    double GetVersion(void) const override
    {
        return 1.21f;
    }
    int32_t GetPriority(void) const override
    {
        return -5;
    }
    uint32_t GetFlags(void) const override
    {
        return (uint32_t)Ashita::PluginFlags::Legacy;
    }
	
    //main.cpp
    bool Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id) override;
    void Release(void) override;
    bool HandleCommand(int32_t mode, const char* command, bool injected) override;
    bool HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked) override;	
    bool HandleOutgoingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked) override;

private:
    //fileio.cpp
    void CreateSettingsXml(bool basic);
    void LoadSettingsXml(bool forceReload);

    //processinput.cpp
    bool ProcessInput(const char* command);
    void ProcessAction(actioninfo_t action, int32_t target);
    actioninfo_t GetAssociatedAction(const char* command);
    actioninfo_t GetActionId(uint32_t flags, std::vector<string> args, int argcount);
    int32_t GetActionTarget(actioninfo_t* action);
    int32_t FindBestTarget(actioninfo_t* action);

    //helpers.cpp
    void InitializeSpells();
    void SetSpellLearned(uint32_t id, bool learned);
    bool IsPositiveInteger(const char* input);
    const char* strcasestr(const char* _Str, const char* _SubStr);
    uint16_t GetValidFlags(actioninfo_t* action);
    bool HasBuff(int16_t id);
    bool CheckBagEnabled(int index);
    bool CheckForUsableItem(uint16_t Id);
    bool CheckForEquippableItem(uint16_t Id);
    bool IsValidTarget(uint16_t validFlags, int index);
    bool RelaxedMatch(std::string input, std::string compare);
    bool CheckRoman(const char** lhs, const char** rhs);
};
#endif