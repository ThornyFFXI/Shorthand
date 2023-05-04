#include "Shorthand.h"

//Target macros we let the game resolve.  We cannot inject as a packet for these, because we don't have a numeric target.
std::list<string> gameResolve = {
	"<bt>",
	"<ht>",
	"<ft>",
	"<st>",
	"<stpc>",
	"<stnpc>",
	"<stpt>",
	"<stal>",
	"<lastst>",
	"<r>",
	"<scan>"
};

bool Shorthand::ProcessInput(const char* command)
{
	//Substitute in any autotranslated stuff.
	char buffer[1024];
	int32_t size = m_AshitaCore->GetChatManager()->ParseAutoTranslate(command, buffer, 1024, false);
	buffer[size] = 0x00;

	//Attempt to figure out what action the command is tied to.  If none found, don't block command.
	actioninfo_t action = GetAssociatedAction(buffer);
	if (action.Type == 0) return false;

	//Attempt to find the action's target.  If none found, don't block command.
	int32_t target = GetActionTarget(&action);
	if (target == 0) return false;

	//We found an action and a target, so process the action and block command.
	ProcessAction(action, target);
	return true;
}
void Shorthand::ProcessAction(actioninfo_t action, int32_t target)
{
	bool usePackets = ((action.Type == (uint32_t)ActionFlags::Weaponskill) && (mSettings.PacketWs));

	if (target > 0)
	{
		uint32_t targetId = m_AshitaCore->GetMemoryManager()->GetEntity()->GetServerId(target);
		if (usePackets)
		{
			pk_Action packet = { 0 };
			packet.TargetIndex = target;
			packet.TargetId = targetId;
			packet.AbilityId = action.Id;
			packet.Category = 0x07;
			m_AshitaCore->GetPacketManager()->AddOutgoingPacket(0x1A, sizeof(pk_Action), (uint8_t*)(&packet));
		}
		else
		{
			char buffer[1024];
			if (action.Type == (uint32_t)ActionFlags::Spell)
			{
				ISpell* spell = m_AshitaCore->GetResourceManager()->GetSpellById(action.Id);
				sprintf_s(buffer, 1024, "/ma \"%s\" %d", spell->Name[0], targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
			else if (action.Type == (uint32_t)ActionFlags::Weaponskill)
			{
				IAbility* ws = m_AshitaCore->GetResourceManager()->GetAbilityById(action.Id);
				sprintf_s(buffer, 1024, "/ws \"%s\" %d", ws->Name[0], targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
			else if (action.Type == (uint32_t)ActionFlags::Ability)
			{
				IAbility* ja = m_AshitaCore->GetResourceManager()->GetAbilityById(action.Id + 512);
				sprintf_s(buffer, 1024, "/ja \"%s\" %d", ja->Name[0], targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
			else if (action.Type == (uint32_t)ActionFlags::Item)
			{
				IItem* item = m_AshitaCore->GetResourceManager()->GetItemById(action.Id);
				sprintf_s(buffer, 1024, "/item \"%s\" %d", item->Name[0], targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
			else if (action.Type == (uint32_t)ActionFlags::Attack)
			{
				sprintf_s(buffer, 1024, "/attack %d", targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
			else if (action.Type == (uint32_t)ActionFlags::Ranged)
			{
				sprintf_s(buffer, 1024, "/ra %d", targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
			else if (action.Type == (uint32_t)ActionFlags::Target)
			{
				sprintf_s(buffer, 1024, "/target %d", targetId);
				m_AshitaCore->GetChatManager()->QueueCommand(0, buffer);
			}
		}
	}

	else
	{
		if ((usePackets) && (target == -1))
			pOutput->error_f("Cannot use packet based action with target of '%s'.  Defaulting to command.", action.Target.c_str());

		char buffer[1024];
		if (action.Type == (uint32_t)ActionFlags::Spell)
		{
			ISpell* spell = m_AshitaCore->GetResourceManager()->GetSpellById(action.Id);
			sprintf_s(buffer, 1024, "/raw /ma \"%s\" %s", spell->Name[0], action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}
		else if (action.Type == (uint32_t)ActionFlags::Weaponskill)
		{
			IAbility* ws = m_AshitaCore->GetResourceManager()->GetAbilityById(action.Id);
			sprintf_s(buffer, 1024, "/raw /ws \"%s\" %s", ws->Name[0], action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}
		else if (action.Type == (uint32_t)ActionFlags::Ability)
		{
			IAbility* ja = m_AshitaCore->GetResourceManager()->GetAbilityById(action.Id + 512);
			sprintf_s(buffer, 1024, "/raw /ja \"%s\" %s", ja->Name[0], action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}
		else if (action.Type == (uint32_t)ActionFlags::Item)
		{
			IItem* item = m_AshitaCore->GetResourceManager()->GetItemById(action.Id);
			sprintf_s(buffer, 1024, "/raw /item \"%s\" %s", item->Name[0], action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}
		else if (action.Type == (uint32_t)ActionFlags::Attack)
		{
			sprintf_s(buffer, 1024, "/raw /attack %s", action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}
		else if (action.Type == (uint32_t)ActionFlags::Ranged)
		{
			sprintf_s(buffer, 1024, "/raw /ra %s", action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}
		else if (action.Type == (uint32_t)ActionFlags::Target)
		{
			sprintf_s(buffer, 1024, "/raw /target %s", action.Target.c_str());
			m_AshitaCore->GetChatManager()->QueueCommand(1, buffer);
		}		
	}
}

actioninfo_t Shorthand::GetAssociatedAction(const char* command)
{
	//Vector for arguments, since we'll need it whether or not we find an alias.
	std::vector<string> args;

	//Check alias map first.
	int longestAlias = 0;
	actioninfo_t* bestAlias = NULL;
	for (std::map<std::string, actioninfo_t>::iterator it = mSettings.AliasMap.begin(); it != mSettings.AliasMap.end(); it++)
	{
		//Skip aliases that are shorter than current match or longer than the command, we want the longest matching one.
		if ((it->first.length() <= strlen(command)) && (it->first.length() > longestAlias))
		{
			//Make sure the text actually matches.
			if (_strnicmp(command, it->first.c_str(), it->first.length()) == 0)
			{
				//Set a pointer to the action and record how long the alias is.
				longestAlias = it->first.length();
				bestAlias = &(it->second);
			}
		}
	}

	//If we found an action, the pointer will be set.
	if (bestAlias)
	{
		if (Ashita::Commands::GetCommandArgs(command + longestAlias, &args) > 0)
		{
			return actioninfo_t(bestAlias->Type, bestAlias->Id, args[0]);
		}
		else
		{
			return actioninfo_t(bestAlias->Type, bestAlias->Id, "");
		}
	}

	//We didn't find an alias, so split command into arguments.
	int argcount = Ashita::Commands::GetCommandArgs(command, &args);

	//If we don't have any args, we obviously can't determine an action.
	if (argcount < 1) return actioninfo_t(0, 0, "");

	//If we only have one arg, we need a placeholder for target in case it can be autofilled later.
	if (argcount == 1)
	{
		args.push_back("");
		argcount++;
	}

	//Check generic commands.
	if (CheckArg(0, "/ra"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Ranged, 0, args[1]);
	}

	if (CheckArg(0, "/range"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Ranged, 0, args[1]);
	}

	if (CheckArg(0, "/shoot"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Ranged, 0, args[1]);
	}

	if (CheckArg(0, "/throw"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Ranged, 0, args[1]);
	}

	if (CheckArg(0, "/a"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Attack, 0, args[1]);
	}

	if (CheckArg(0, "/attack"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Attack, 0, args[1]);
	}

	if (CheckArg(0, "/ta"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Target, 0, args[1]);
	}

	if (CheckArg(0, "/target"))
	{
		return actioninfo_t((uint32_t)ActionFlags::Target, 0, args[1]);
	}

	if (CheckArg(0, "/ma"))
	{
		return GetActionId((uint32_t)ActionFlags::Spell, args, argcount);
	}

	if (CheckArg(0, "/magic"))
	{
		return GetActionId((uint32_t)ActionFlags::Spell, args, argcount);
	}

	if (CheckArg(0, "/ja"))
	{
		return GetActionId((uint32_t)ActionFlags::Ability, args, argcount);
	}

	if (CheckArg(0, "/jobability"))
	{
		return GetActionId((uint32_t)ActionFlags::Ability, args, argcount);
	}

	if (CheckArg(0, "/pet"))
	{
		return GetActionId((uint32_t)ActionFlags::Ability, args, argcount);
	}

	if (CheckArg(0, "/ws"))
	{
		return GetActionId((uint32_t)ActionFlags::Weaponskill, args, argcount);
	}

	if (CheckArg(0, "/weaponskill"))
	{
		return GetActionId((uint32_t)ActionFlags::Weaponskill, args, argcount);
	}

	if (CheckArg(0, "/i"))
	{
		return GetActionId((uint32_t)ActionFlags::Item, args, argcount);
	}

	if (CheckArg(0, "/item"))
	{
		return GetActionId((uint32_t)ActionFlags::Item, args, argcount);
	}

	if (strncmp(command, "//", 2) == 0)
	{
		if (args[0].length() > 2)
		{
			//Treat the // as it's own argument and push the others back to standardize.
			//We only have to do this if the command is smushed into the //.. something like '// blizzard seiryu' would be fine as is.
			args.push_back(args[argcount - 1]);
			for (int x = argcount; x >= 0; x--)
			{
				if (x == 1) args[1] = args[0].substr(2);
				else if (x == 0) args[0] = "//";
				else args[x] = args[x - 1];
			}
			argcount++;
		}

		return GetActionId((uint32_t)ActionFlags::Unknown, args, argcount);
	}

	return actioninfo_t(0, 0, "");
}
actioninfo_t Shorthand::GetActionId(uint32_t flags, std::vector<string> args, int argcount)
{
	actioninfo_t pending(0, 0, "");

	//If we don't have a valid second argument to get our action from, we can't determine which action is needed.
	if ((argcount < 2) || (args[1].length() < 1)) return pending;

	//If we only have 2 args, we need a placeholder for target in case it can be autofilled later.
	if (argcount == 2) args.push_back("");

	//Grab our level info.  This is used to prioritize spells and abilities we actually know in the event of ambiguity.
	int mainJob = m_AshitaCore->GetMemoryManager()->GetPlayer()->GetMainJob();
	int mainJobLevel = m_AshitaCore->GetMemoryManager()->GetPlayer()->GetMainJobLevel();
	int subJob = m_AshitaCore->GetMemoryManager()->GetPlayer()->GetSubJob();
	int subJobLevel = m_AshitaCore->GetMemoryManager()->GetPlayer()->GetSubJobLevel();


	if (flags & (uint32_t)ActionFlags::Spell)
	{
		for (std::map<std::string, uint16_t>::iterator iter = mSettings.SpellMap.begin(); iter != mSettings.SpellMap.end(); iter++)
		{
			if (RelaxedMatch(args[1], iter->first))
			{
				if (m_AshitaCore->GetMemoryManager()->GetPlayer()->HasSpell(iter->second))
				{
					ISpell* spellInfo = m_AshitaCore->GetResourceManager()->GetSpellById(iter->second);
					bool hasLevel = false;
					if (spellInfo == NULL)
					{
						pOutput->error_f("Spell resource could not be resolved.  Index:%d", iter->second);
					}
					else
					{
						if (spellInfo->LevelRequired[(int)pow(mainJob, 2)] < mainJobLevel) hasLevel = true;
						if (spellInfo->LevelRequired[(int)pow(subJob, 2)] < subJobLevel) hasLevel = true;
					}
					if (hasLevel)
					{
						return actioninfo_t((uint32_t)ActionFlags::Spell, iter->second, args[2]);
					}
				}
				pending = actioninfo_t((uint32_t)ActionFlags::Spell, iter->second, args[2]);
			}
		}
	}

	if (flags & (uint32_t)ActionFlags::Ability)
	{
		for (std::map<std::string, uint16_t>::iterator iter = mSettings.AbilityMap.begin(); iter != mSettings.AbilityMap.end(); iter++)
		{
			if (RelaxedMatch(args[1], iter->first))
			{
				if (m_AshitaCore->GetMemoryManager()->GetPlayer()->HasAbility(iter->second + 512))
				{
					return actioninfo_t((uint32_t)ActionFlags::Ability, iter->second, args[2]);
				}
				else if (pending.Id == 0)
				{
					pending = actioninfo_t((uint32_t)ActionFlags::Ability, iter->second, args[2]);
				}
			}
		}
	}

	if (flags & (uint32_t)ActionFlags::Weaponskill)
	{
		for (std::map<std::string, uint16_t>::iterator iter = mSettings.WeaponskillMap.begin(); iter != mSettings.WeaponskillMap.end(); iter++)
		{
			if (RelaxedMatch(args[1], iter->first))
			{
				if (m_AshitaCore->GetMemoryManager()->GetPlayer()->HasWeaponSkill(iter->second))
				{
					return actioninfo_t((uint32_t)ActionFlags::Weaponskill, iter->second, args[2]);
				}
				else if (pending.Id == 0)
				{
					pending = actioninfo_t((uint32_t)ActionFlags::Weaponskill, iter->second, args[2]);
				}
			}
		}
	}

	if (flags & (uint32_t)ActionFlags::Item)
	{
		for (std::map<std::string, uint16_t>::iterator iter = mSettings.ItemMap.begin(); iter != mSettings.ItemMap.end(); iter++)
		{
			if (RelaxedMatch(args[1], iter->first))
			{
				if (CheckForUsableItem(iter->second))
				{
					return actioninfo_t((uint32_t)ActionFlags::Item, iter->second, args[2]);
				}
				else if (pending.Id == 0)
				{
					pending = actioninfo_t((uint32_t)ActionFlags::Item, iter->second, args[2]);
				}
			}
		}
	}

	return pending;
}
int32_t Shorthand::GetActionTarget(actioninfo_t* action)
{
	const char* target = action->Target.c_str();
	
	//Resolve any in-game macros we can.
	if (_stricmp(target, "<me>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
	if (_stricmp(target, "<p0>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
	if (_stricmp(target, "<p1>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(1);
	if (_stricmp(target, "<p2>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(2);
	if (_stricmp(target, "<p3>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(3);
	if (_stricmp(target, "<p4>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(4);
	if (_stricmp(target, "<p5>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(5);
	if (_stricmp(target, "<a10>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(6);
	if (_stricmp(target, "<a11>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(7);
	if (_stricmp(target, "<a12>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(8);
	if (_stricmp(target, "<a13>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(9);
	if (_stricmp(target, "<a14>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(10);
	if (_stricmp(target, "<a15>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(11);
	if (_stricmp(target, "<a20>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(12);
	if (_stricmp(target, "<a21>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(13);
	if (_stricmp(target, "<a22>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(14);
	if (_stricmp(target, "<a23>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(15);
	if (_stricmp(target, "<a24>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(16);
	if (_stricmp(target, "<a25>") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(17);
	if (_stricmp(target, "<t>") == 0)
	{
		uint8_t target = m_AshitaCore->GetMemoryManager()->GetTarget()->GetIsSubTargetActive();
		return m_AshitaCore->GetMemoryManager()->GetTarget()->GetTargetIndex(target);
	}
	if (_stricmp(target, "<pet>") == 0)
	{
		uint16_t myIndex = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
		return m_AshitaCore->GetMemoryManager()->GetEntity()->GetPetTargetIndex(myIndex);
	}

	//Return -1 for any in-game macros we aren't resolving.  This will clean the string and send it through again.
	//Note that this will not give us a numeric value, so we cannot inject packets for packetws.
	for (std::list<string>::iterator iter = gameResolve.begin(); iter != gameResolve.end(); iter++)
	{
		if (_stricmp(target, iter->c_str()) == 0)
		{
			action->Target = *iter;
			return -1;
		}
	}

	//Resolve custom macros
	if (_stricmp(target, "me") == 0) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);

	//Command is not using a valid macro, so parse entities to find a matching target.
	return FindBestTarget(action);
}
int32_t Shorthand::FindBestTarget(actioninfo_t* action)
{
	//Figure out what our action is capable of targeting.
	uint16_t validFlags = GetValidFlags(action);

	//If we don't have any text in the target field, see if it's a self-only spell or our current target will work.
	if (action->Target.length() < 1)
	{
		if (validFlags == 1) return m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);

		int targetStatus = m_AshitaCore->GetMemoryManager()->GetTarget()->GetIsSubTargetActive();
		int targetIndex = m_AshitaCore->GetMemoryManager()->GetTarget()->GetTargetIndex(targetStatus);
		if (IsValidTarget(validFlags, targetIndex)) return targetIndex;

		//Not self-only, and our current target isn't valid.  We have no text to work with, so we can't resolve a target accurately.
		return 0;
	}

	if (IsPositiveInteger(action->Target.c_str()))
	{
		//If we have an integer in range of valid entity indices, check index first.
		int32_t target = atoi(action->Target.c_str());
        if ((target > 0) && (target < 2304))
        {
            if (IsValidTarget(validFlags, target))
                return target;
        }

		//If index doesn't work, check for ID.
		for (int x = 0; x < 0x900; x++)
		{
			if ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetServerId(x) == target)
				&& (IsValidTarget(validFlags, x))) return x;
		}
	}

	//Variables to store our best match and it's qualifying statistics so we can compare potential matches against it.
	float bestDistance = FLT_MAX;
	int bestStringComp = 30;
	int bestTarget = 0;

	for (int x = 1; x < 0x900; x++)
	{
		if (IsValidTarget(validFlags, x))
		{
			const char* name = m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x);
			const char* check = strcasestr(name, action->Target.c_str());
			if (!check) continue;

			//A match earlier in the name is preferred over one in the middle of name, as less likely to be accidental.
			//Start with a comparison value greater than max length of name.
			int stringValue = 29; 
			if (_stricmp(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x), action->Target.c_str()) == 0)
			{
				//If the name is an exact match, this is preferred over a name that includes the typed command always.
				//Set comparison value to -1 so it cannot be matched except by another exact match.
				stringValue = -1;
			}
			else
			{
				//Set comparison value to how far into the mob's name the match was found.
				stringValue = check - name;
			}

			//We always prefer a better name match to a better distance match.  So, if our name match is worse we're done.
			if (stringValue > bestStringComp) continue;

			float distance = m_AshitaCore->GetMemoryManager()->GetEntity()->GetDistance(x);
			
			//We already checked if our name match is worse, so if it's not better it's equal and we can fall back on distance.
			if ((stringValue < bestStringComp) || (distance < bestDistance))
			{
				bestDistance = distance;
				bestStringComp = stringValue;
				bestTarget = x;
			}
		}
	}

	//Now we return target.  If we didn't find one, it'll still be 0, and the command will be aborted.
	return bestTarget;
}