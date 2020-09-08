#ifndef __ASHITA_ThornyOutputHelpers_H_INCLUDED__
#define __ASHITA_ThornyOutputHelpers_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Program Files (x86)\Ashita 4\plugins\sdk\Ashita.h"
#include <iostream>
#include <string>

using namespace std;

#define ERROR_COLOR 123
#define HIGHLIGHT_COLOR Ashita::Chat::Colors::LawnGreen
#define MESSAGE_COLOR 127

class OutputHelpers
{
    char pluginName[256];

public:
    OutputHelpers(IAshitaCore* core, ILogManager* logger, const char* pluginNameConst)
    {
        m_AshitaCore = core;
        m_LogManager = logger;
        strcpy_s(pluginName, 256, pluginNameConst);
    }

    void error(string input)
    {
        string working = input;
        int index      = input.find("$H");
        while (index != string::npos)
        {
            working = working.replace(index, 2, HIGHLIGHT_COLOR);
            index   = working.find("$H");
        }

        index = input.find("$R");
        while (index != string::npos)
        {
            working = working.replace(index, 2, Ashita::Chat::Colors::Reset);
            index   = working.find("$R");
        }

        stringstream output;
        output << Ashita::Chat::Header(pluginName) << Ashita::Chat::Colors::Reset << working;
        m_AshitaCore->GetChatManager()->Write(ERROR_COLOR, false, output.str().c_str());

        stringstream error;
        for (int x = 0; x < working.length(); x++)
        {
            if ((working[x] == 0x1E) || (working[x] == 0x1F))
                x++;
            else
                error << working[x];
        }

        m_LogManager->Log((uint32_t)Ashita::LogLevel::Error, pluginName, error.str().c_str());
    }
    void error_f(const char* input, ...)
    {
        va_list args;
        va_start(args, input);

        char buffer[1024];
        vsprintf_s(buffer, 1024, input, args);
        va_end(args);

        error(buffer);
    }
    void message(string input)
    {
        string working = input;
        int index      = input.find("$H");
        while (index != string::npos)
        {
            working = working.replace(index, 2, HIGHLIGHT_COLOR);
            index   = working.find("$H");
        }

        index = input.find("$R");
        while (index != string::npos)
        {
            working = working.replace(index, 2, Ashita::Chat::Colors::Reset);
            index   = working.find("$R");
        }

        stringstream output;
        output << Ashita::Chat::Header(pluginName) << Ashita::Chat::Colors::Reset << working;

        m_AshitaCore->GetChatManager()->Write(MESSAGE_COLOR, false, output.str().c_str());
    }
    void message_f(const char* input, ...)
    {
        va_list args;
        va_start(args, input);

        char buffer[1024];
        vsprintf_s(buffer, 1024, input, args);
        va_end(args);

        message(buffer);
    }

    void debug(string input)
    {
        m_LogManager->Log((uint32_t)Ashita::LogLevel::Debug, pluginName, input.c_str());
    }

    void debug_f(const char* input, ...)
    {
        va_list args;
        va_start(args, input);
        char buffer[1024];
        vsprintf_s(buffer, 1024, input, args);
        va_end(args);
        debug(buffer);
    }

private:
    IAshitaCore* m_AshitaCore;
    ILogManager* m_LogManager;
};
#endif