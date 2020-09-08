#include "Shorthand.h"
#include "rapidxml.hpp"
#include <fstream>

using namespace rapidxml;

void Shorthand::CreateSettingsXml(bool basic)
{
	char buffer[1024];
	if (basic)
	{
		sprintf_s(buffer, 1024, "%s\\config\\shorthand\\settings.xml", m_AshitaCore->GetInstallPath());
	}
	else
	{
		sprintf_s(buffer, 1024, "%s\\config\\shorthand\\settings-empty.xml", m_AshitaCore->GetInstallPath());
	}

	ofstream outstream(buffer);
	if (!outstream.is_open())
	{
		pOutput->error_f("Failed to write file.  [%s]", buffer);
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
	pOutput->message_f("Wrote settings XML. [$H%s$R]", buffer);
}

void Shorthand::LoadSettingsXml()
{
	//Reset settings.
	mSettings = settings_t(m_AshitaCore);

	//Create path to settings XML.
	char buffer[1024];
	sprintf_s(buffer, 1024, "%s\\config\\shorthand\\settings.xml", m_AshitaCore->GetInstallPath());

	//Ensure directories exist, making them if not.
	string makeDirectory(buffer);
	size_t nextDirectory = makeDirectory.find("\\");
	nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
	while (nextDirectory != string::npos)
	{
		string currentDirectory = makeDirectory.substr(0, nextDirectory + 1);
		if ((!CreateDirectory(currentDirectory.c_str(), NULL)) && (ERROR_ALREADY_EXISTS != GetLastError()))
		{
			pOutput->error_f("Could not find or create folder. [$H%s$R]", currentDirectory.c_str());
			return;
		}
		nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
	}

	//If settings XML doesn't exist, write default settings to a blank file for next time.
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(buffer))
	{
		CreateSettingsXml(true);
		return;
	}

	std::ifstream Reader(buffer, ios::in | ios::binary | ios::ate);
	if (!Reader.is_open())
	{
		pOutput->error_f("Failed to read file.  [$H%s$R]", buffer);
		return;
	}

	Reader.seekg(0, ios::end);
	long Size = Reader.tellg();
	char* File = new char[Size + 1];
	Reader.seekg(0, ios::beg);
	Reader.read(File, Size);
	Reader.close();
	File[Size] = '\0';

	xml_document<>* XMLReader = new xml_document<>();
	try
	{
		XMLReader->parse<0>(File);
	}
	catch (const rapidxml::parse_error& e)
	{
		int line = static_cast<long>(std::count(File, e.where<char>(), '\n') + 1);
		stringstream error;
		error << "Parse error[$H" << e.what() << "$R] at line $H" << line << "$R.";
		pOutput->error(error.str().c_str());
		delete XMLReader;
		delete[] File;
		return;
	}
	catch (...)
	{
		pOutput->error("Failed to parse configuration XML.");
		delete XMLReader;
		delete[] File;
		return;
	}

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

	delete[] File;
	delete XMLReader;
}