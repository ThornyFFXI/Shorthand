#ifndef __ASHITA_ThornySettings_H_INCLUDED__
#define __ASHITA_ThornySettings_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "Ashita.h"
#include "Output.h"
#include "thirdparty/rapidxml.hpp"
#include <filesystem>
#include <fstream>
#include <WinBase.h>

using namespace rapidxml;
using namespace std;

#define ERROR_COLOR 123
#define HIGHLIGHT_COLOR Ashita::Chat::Colors::LawnGreen
#define MESSAGE_COLOR 127

class SettingsHelper
{
private:
    IAshitaCore* m_AshitaCore;
    OutputHelpers* pOutput;
    char PluginName[256];
    char Extension[256];

    char* FileBuffer;
    char CurrentFile[256];
    xml_document<>* Document;


public:
    SettingsHelper(IAshitaCore* core, OutputHelpers* output, const char* Plugin)
    {
        m_AshitaCore = core;
        pOutput      = output;
        strcpy_s(PluginName, 256, Plugin);
        strcpy_s(Extension, 256, ".xml");
        strcpy_s(CurrentFile, 256, "NO_FILE");
    }

    ~SettingsHelper()
    {
        UnloadSettings();
    }

    void SetExtension(const char* NewExtension)
    {
        strcpy_s(this->Extension, 256, NewExtension);
    }

    void CreateDirectories(const char* Path)
    {
        //Ensure directories exist, making them if not.
        string makeDirectory(Path);
        size_t nextDirectory = makeDirectory.find("\\");
        nextDirectory        = makeDirectory.find("\\", nextDirectory + 1);
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
    }

    std::string GetDefaultSettingsPath()
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%sconfig\\%s\\default%s", m_AshitaCore->GetInstallPath(), PluginName, Extension);
        return std::string(buffer);
    }
    std::string GetDefaultSettingsPath(bool* exists)
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%sconfig\\%s\\default%s", m_AshitaCore->GetInstallPath(), PluginName, Extension);
        *exists = std::filesystem::exists(buffer);
        return std::string(buffer);
    }

    std::string GetCharacterSettingsPath(const char* PlayerName)
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, PlayerName, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\default%s", m_AshitaCore->GetInstallPath(), PluginName, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        return "FILE_NOT_FOUND";
    }

    std::string GetInputSettingsPath(const char* Input)
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%s", Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%s%s", Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

            sprintf_s(buffer, 1024, "%sconfig\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        return "FILE_NOT_FOUND";
    }
    std::string GetInputSettingsPath(const char* PlayerName, const char* Input)
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%s", Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%s%s", Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, PlayerName, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, PlayerName, Input, Extension);
        if (std::filesystem::exists(buffer))

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        return "FILE_NOT_FOUND";
    }
    std::string GetInputSettingsPath(uint16_t CurrentZone, const char* Input)
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%s", Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%s%s", Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%u\\%s", m_AshitaCore->GetInstallPath(), PluginName, CurrentZone, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%u\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, CurrentZone, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        return "FILE_NOT_FOUND";
    }
    std::string GetInputSettingsPath(const char* PlayerName, uint16_t CurrentZone, const char* Input)
    {
        char buffer[1024];
        sprintf_s(buffer, 1024, "%s", Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%s%s", Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, PlayerName, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, PlayerName, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%u\\%s", m_AshitaCore->GetInstallPath(), PluginName, CurrentZone, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%u\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, CurrentZone, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, Input);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        sprintf_s(buffer, 1024, "%sconfig\\%s\\%s%s", m_AshitaCore->GetInstallPath(), PluginName, Input, Extension);
        if (std::filesystem::exists(buffer))
            return std::string(buffer);

        return "FILE_NOT_FOUND";
    }

    std::string GetInputWritePath(const char* Input)
    {
        char buffer[1024];
        if (strstr(Input, ".") == Input + (strlen(Input) - 4))
            sprintf_s(buffer, 1024, "%s\\config\\%s\\%s", m_AshitaCore->GetInstallPath(), PluginName, Input);
        else
            sprintf_s(buffer, 1024, "%s\\config\\%s\\%s.xml", m_AshitaCore->GetInstallPath(), PluginName, Input);
        CreateDirectories(buffer);
        return std::string(buffer);
    }

    //Load a file, letting user manage the pointer.
    char* LoadFile(std::string Path)
    {
        std::ifstream inputStream = ifstream(Path.c_str(), ios::in | ios::binary);
        if (!inputStream.is_open())
            return NULL;

        long Size  = inputStream.tellg();
        char* OutBuffer = new char[Size + 1];
        inputStream.seekg(0, ios::beg);
        inputStream.read(OutBuffer, Size);
        OutBuffer[Size] = '\0';

        return OutBuffer;
    }

    //Load a settings XML, letting SettingsHelper manage the memory.  Lifespan is only until new XML is loaded or UnloadSettings is called.
    xml_document<>* LoadSettingsXml(std::string Path)
    {
        //Unload existing settings so we don't memory leak when we reassign pointers.
        UnloadSettings();

        //Attempt to create a stream of the specified file.
        std::ifstream inputStream = ifstream(Path.c_str(), ios::in | ios::binary | ios::ate);
        if (!inputStream.is_open())
        {
            pOutput->error_f("Failed to parse settings XML. [$H%s$R]", Path.c_str());
            return NULL;
        }

        long Size  = inputStream.tellg();
        FileBuffer = new char[Size + 1];
        inputStream.seekg(0, ios::beg);
        inputStream.read(FileBuffer, Size);
        FileBuffer[Size] = '\0';

        Document = new xml_document<>();
        try
        {
            Document->parse<0>(FileBuffer);
        }
        catch (const rapidxml::parse_error& e)
        {
            int line = static_cast<long>(std::count(FileBuffer, e.where<char>(), '\n') + 1);
            pOutput->error_f("Parse error in settings XML [$H%s$R] at line $H%d$R. [$H%s$R]", e.what(), line, Path.c_str());
            delete Document;
            delete[] FileBuffer;
            return NULL;
        }
        catch (...)
        {
            pOutput->error_f("Failed to parse settings XML.  No error specified.  [$H%s$R]", Path.c_str());
            delete Document;
            delete[] FileBuffer;
            return NULL;
        }

        strcpy_s(CurrentFile, 256, Path.c_str());
        return Document;
    }
 
    //Get path to last loaded XML.
    std::string GetLoadedXmlPath()
    {
        return std::string(CurrentFile);
    }

    //Load an XML, using the specified char pointer to store the file contents.
    xml_document<>* LoadXml(std::string Path, char* Buffer)
    {
        //Unload existing settings so we don't memory leak when we reassign pointers.
        UnloadSettings();

        //Attempt to create a stream of the specified file.
        std::ifstream inputStream = ifstream(Path.c_str(), ios::in | ios::binary | ios::ate);
        if (!inputStream.is_open())
        {
            pOutput->error_f("Failed to open file. [$H%s$R]", Path.c_str());
            return NULL;
        }

        long Size  = inputStream.tellg();
        Buffer = new char[Size + 1];
        inputStream.seekg(0, ios::beg);
        inputStream.read(Buffer, Size);
        Buffer[Size] = '\0';

        xml_document<>* DocBuffer = new xml_document<>();
        try
        {
            DocBuffer->parse<0>(Buffer);
        }
        catch (const rapidxml::parse_error& e)
        {
            int line = static_cast<long>(std::count(FileBuffer, e.where<char>(), '\n') + 1);
            pOutput->error_f("Parse error in XML [$H%s$R] at line $H%d$R. [$H%s$R]", e.what(), line, Path.c_str());
            delete DocBuffer;
            delete[] Buffer;
            return NULL;
        }
        catch (...)
        {
            pOutput->error_f("Failed to parse XML.  No error specified.  [$H%s$R]", Path.c_str());
            delete DocBuffer;
            delete[] Buffer;
            return NULL;
        }

        return DocBuffer;
    }

    //Unload current XML.
    void UnloadSettings()
    {
        if (Document != NULL)
        {
            delete Document;
            delete[] FileBuffer;
            Document = NULL;
            strcpy_s(CurrentFile, 256, "NO_FILE");
        }
    }
};
#endif