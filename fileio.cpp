#include "Shorthand.h"
#include "rapidxml.hpp"
#include <fstream>

using namespace rapidxml;

void Shorthand::CreateSettingsXml(bool basic)
{
    std::string Path = basic ? pSettings->GetDefaultSettingsPath() : pSettings->GetInputWritePath("empty.xml");
    pSettings->CreateDirectories(Path.c_str());

	ofstream outstream(Path.c_str());
	if (!outstream.is_open())
	{
		pOutput->error_f("Failed to write file.  [%s]", Path.c_str());
		return;
	}

	outstream << "<shorthand>\n";
	outstream << "\n\t<settings>\n";
	outstream << "\t\t<packetws>";
	if (mSettings.PacketWs) outstream << "true";
	else outstream << "false";
	outstream << "</packetws>\n";
	outstream << "\t\t<gearcheck>";
	if (mSettings.CheckInventoryForGear) outstream << "true";
	else outstream << "false";
	outstream << "</gearcheck>\n";
	outstream << "\t\t<impact>";
	if (mSettings.UnlockImpact) outstream << "true";
	else outstream << "false";
	outstream << "</impact>\n";
	outstream << "\t\t<honor>";
	if (mSettings.UnlockHonorMarch) outstream << "true";
	else outstream << "false";
	outstream << "</honor>\n";
	outstream << "\t\t<dispelga>";
	if (mSettings.UnlockDispelga) outstream << "true";
	else outstream << "false";
	outstream << "</dispelga>\n";
	outstream << "\t\t<debug>";
	if (mSettings.Debug) outstream << "true";
	else outstream << "false";
	outstream << "</dispelga>\n";
	outstream << "\t</settings>\n\n";

	if (!basic)
	{
		for (unsigned short X = 512; X < 1512; X++)
		{
			IAbility* Ability = m_AshitaCore->GetResourceManager()->GetAbilityById(X);
			if ((Ability)
				&& (strlen(Ability->Name[0]) > 1))
			{
				outstream << "\t<ability id=\"" << (X - 512) << "\"> <!--" << Ability->Name[0] << "-->\n";
				outstream << "\t</ability>\n";
			}
		}
		outstream << "\n";
		for (unsigned short X = 0; X < 2048; X++)
		{
			ISpell* Spell = m_AshitaCore->GetResourceManager()->GetSpellById(X);
			if ((Spell)
				&& (strlen(Spell->Name[0]) > 1))
			{
				outstream << "\t<spell id=\"" << X << "\"> <!--" << Spell->Name[0] << "-->\n";
				outstream << "\t</spell>\n";
			}
		}
		outstream << "\n";
		for (unsigned short X = 0; X < 512; X++)
		{
			IAbility* Ability = m_AshitaCore->GetResourceManager()->GetAbilityById(X);
			if ((Ability)
				&& (strlen(Ability->Name[0]) > 1))
			{
				outstream << "\t<weaponskill id=\"" << X << "\"> <!--" << Ability->Name[0] << "-->\n";
				outstream << "\t</weaponskill>\n";
			}
		}
		outstream << "\n";
	}

	outstream << "</shorthand>";
	outstream.close();
	pOutput->message_f("Wrote settings XML. [$H%s$R]", Path.c_str());
}

void Shorthand::LoadSettingsXml(bool forceReload)
{
	//Reset settings.
	mSettings = settings_t(m_AshitaCore);

	//Get path to settings XML.
    std::string Path = pSettings->GetCharacterSettingsPath(mState.CharacterName.c_str());

	if ((Path == pSettings->GetLoadedXmlPath()) && (!forceReload)) return;

	if (Path == "FILE_NOT_FOUND")
    {
        Path = pSettings->GetDefaultSettingsPath();
        CreateSettingsXml(true);
	}

	xml_document<>* XMLReader = pSettings->LoadSettingsXml(Path);
    if (XMLReader == NULL)
        return;

	xml_node<>* Node = XMLReader->first_node("shorthand");
	if (Node) Node = Node->first_node();
	while (Node)
	{
		if (_stricmp(Node->name(), "settings") == 0)
		{
			xml_node<>* SubNode = Node->first_node();
			while (SubNode)
			{
				if (_stricmp(SubNode->name(), "packetws") == 0)
				{
					if (_stricmp(SubNode->value(), "true") == 0) mSettings.PacketWs = true;
				}
				if (_stricmp(SubNode->name(), "gearcheck") == 0)
				{
					if (_stricmp(SubNode->value(), "false") == 0) mSettings.CheckInventoryForGear = false;
				}
				if (_stricmp(SubNode->name(), "impact") == 0)
				{
					if (_stricmp(SubNode->value(), "true") == 0) mSettings.UnlockImpact = true;
				}
				if (_stricmp(SubNode->name(), "dispelga") == 0)
				{
					if (_stricmp(SubNode->value(), "true") == 0) mSettings.UnlockDispelga = true;
				}
				if (_stricmp(SubNode->name(), "honor") == 0)
				{
					if (_stricmp(SubNode->value(), "true") == 0) mSettings.UnlockHonorMarch = true;
				}
				if (_stricmp(SubNode->name(), "debug") == 0)
				{
					if (_stricmp(SubNode->value(), "true") == 0) mSettings.Debug = true;
				}

				SubNode = SubNode->next_sibling();
			}

		}

		else if (_stricmp(Node->name(), "ability") == 0)
		{
			xml_attribute<>* Attr = Node->first_attribute("id");
			if ((Attr)
				&& (IsPositiveInteger(Attr->value())))
			{
				int ID = atoi(Attr->value());
				IAbility* pAbility = m_AshitaCore->GetResourceManager()->GetAbilityById(ID + 512);
				if (pAbility)
				{
					xml_node<>* SubNode = Node->first_node();
					while (SubNode)
					{
						if (_stricmp(SubNode->name(), "command") == 0)
						{
							actioninfo_t builder((uint32_t)ActionFlags::Ability, ID, "");
							mSettings.AliasMap.insert(std::make_pair(SubNode->value(), builder));
						}
						else if (_stricmp(SubNode->name(), "name") == 0)
						{
							mSettings.AbilityMap.insert(std::make_pair(SubNode->value(), ID));
						}

						SubNode = SubNode->next_sibling();
					}
				}
				else
				{
					pOutput->error_f("Invalid ability in settings file.  [ID:$H%d$R]", ID);
				}
			}
		}

		else if (_stricmp(Node->name(), "item") == 0)
		{
			xml_attribute<>* Attr = Node->first_attribute("id");
			if ((Attr)
				&& (IsPositiveInteger(Attr->value())))
			{
				int ID = atoi(Attr->value());
				IItem* pItem = m_AshitaCore->GetResourceManager()->GetItemById(ID);
				if ((pItem)
					&& (strlen(pItem->Name[0]) > 1)
					&& ((pItem->Flags & 0x0200) || (pItem->Flags & 0x0400))) //Only usable or enchanted items.
				{
					xml_node<>* SubNode = Node->first_node();
					while (SubNode)
					{
						if (_stricmp(SubNode->name(), "command") == 0)
						{
							actioninfo_t builder((uint32_t)ActionFlags::Item, ID, "");
							mSettings.AliasMap.insert(std::make_pair(SubNode->value(), builder));
						}
						else if (_stricmp(SubNode->name(), "name") == 0)
						{
							mSettings.ItemMap.insert(std::make_pair(SubNode->value(), ID));
						}

						SubNode = SubNode->next_sibling();
					}
				}
				else
				{
					pOutput->error_f("Invalid item in settings file.  [ID:$H%d$R]", ID);
				}
			}
		}

		else if (_stricmp(Node->name(), "spell") == 0)
		{
			xml_attribute<>* Attr = Node->first_attribute("id");
			if ((Attr)
				&& (IsPositiveInteger(Attr->value())))
			{
				int ID = atoi(Attr->value());
				ISpell* pSpell = m_AshitaCore->GetResourceManager()->GetSpellById(ID);
				if (pSpell)
				{
					xml_node<>* SubNode = Node->first_node();
					while (SubNode)
					{
						if (_stricmp(SubNode->name(), "command") == 0)
						{
							actioninfo_t builder((uint32_t)ActionFlags::Spell, pSpell->Index, "");
							mSettings.AliasMap.insert(std::make_pair(SubNode->value(), builder));
						}
						else if (_stricmp(SubNode->name(), "name") == 0)
						{
							mSettings.SpellMap.insert(std::make_pair(SubNode->value(), pSpell->Index));
						}

						SubNode = SubNode->next_sibling();
					}
				}
				else
				{
					pOutput->error_f("Invalid spell in settings file.  [ID:$H%d$R]", ID);
				}
			}
		}

		else if (_stricmp(Node->name(), "weaponskill") == 0)
		{
			xml_attribute<>* Attr = Node->first_attribute("id");
			if ((Attr)
				&& (IsPositiveInteger(Attr->value())))
			{
				int ID = atoi(Attr->value());
				IAbility* pAbility = m_AshitaCore->GetResourceManager()->GetAbilityById(ID);
				if (pAbility)
				{
					xml_node<>* SubNode = Node->first_node();
					while (SubNode)
					{
						if (_stricmp(SubNode->name(), "command") == 0)
						{
							actioninfo_t builder((uint32_t)ActionFlags::Weaponskill, ID, "");
							mSettings.AliasMap.insert(std::make_pair(SubNode->value(), builder));
						}
						else if (_stricmp(SubNode->name(), "name") == 0)
						{
							mSettings.WeaponskillMap.insert(std::make_pair(SubNode->value(), ID));
						}

						SubNode = SubNode->next_sibling();
					}
				}
				else
				{
					pOutput->error_f("Invalid ability in settings file.  [ID:$H%d$R]", ID);
				}
			}
		}

		Node = Node->next_sibling();
	}
}