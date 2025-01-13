#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#define RBUFP(p,pos) (((uint8_t*)(p)) + (pos))
#define Read8(p,pos) (*(uint8_t*)RBUFP((p),(pos)))
#define Read16(p,pos) (*(uint16_t*)RBUFP((p),(pos)))
#define Read32(p,pos) (*(uint32_t*)RBUFP((p),(pos)))
#define Read64(p,pos) (*(uint64_t*)RBUFP((p),(pos)))
#define ReadFloat(p,pos) (*(float_t*)RBUFP((p),(pos)))

#define WBUFP(p,pos) (((uint8_t*)(p)) + (pos))
#define Write8(p,pos) (*(uint8_t*)WBUFP((p),(pos)))
#define Write16(p,pos) (*(uint16_t*)WBUFP((p),(pos)))
#define Write32(p,pos) (*(uint32_t*)WBUFP((p),(pos)))
#define Write64(p,pos) (*(uint64_t*)WBUFP((p),(pos)))
#define WriteFloat(p,pos) (*(float_t*)WBUFP((p),(pos)))

#define CheckArg(a,b) (argcount > a) && (_stricmp(args[a].c_str(), b) == 0)


static std::string FormatName(std::string name)
{
    std::string newName;
    for (int x = 0; x < name.length(); x++)
    {
        if (x == 0)
            newName += toupper(name[x]);
        else
            newName += tolower(name[x]);
    }
    return newName;
}

#endif _UTILITIES_H_