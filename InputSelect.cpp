#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

#define APP_NAME "InputSelect"
#define PARAM_SELECT "select"
#define PARAM_LIST "list"
#define PARAM_DP "dp"
#define PARAM_HDMI "hdmi"

#define HDMI 0x11
#define DISPLAY_PORT 0x0F

#define CMD_UNKNOWN 0
#define CMD_LIST 1
#define CMD_SELECT 2

int nRet = 0;
DWORD dwMode = 0;
char* szMonitorName;
DWORD dwCommand = CMD_UNKNOWN;
bool bFound = false;

static BOOL CALLBACK MyInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    // called for each physical monitor

    DWORD dwPhysMonNum;
    LPPHYSICAL_MONITOR pPhysicalMonitors = nullptr;

    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &dwPhysMonNum))
    {
        std::cout << "GetNumberOfPhysicalMonitorsFromHMONITOR error!\n";
        nRet = 1;
        return false;
    }

    pPhysicalMonitors = static_cast<LPPHYSICAL_MONITOR>(calloc(dwPhysMonNum, sizeof(PHYSICAL_MONITOR)));
    if (pPhysicalMonitors == nullptr)
    {
        std::cout << "Memory allocation error!\n";
        nRet = 1;
        return false;
    }

    if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, dwPhysMonNum, pPhysicalMonitors))
    {
        std::cout << "GetPhysicalMonitorsFromHMONITOR error!\n";
        nRet = 1;
        free(pPhysicalMonitors);
        return false;
    }

    std::wcout << L"Monitor found: " << pPhysicalMonitors->szPhysicalMonitorDescription << L"\n";

    bool match = true;
    if (dwCommand != CMD_LIST)
    {
        // Freaking unicode. Monitor name is like m0o0n0i0t0o0r0
        for(int i = 0; i < strlen(szMonitorName); i++)
        {
            if (((BYTE)pPhysicalMonitors->szPhysicalMonitorDescription[i]) != szMonitorName[i])
            {
                match = false;
                break;
            }
        }

        //if (memcmp(pPhysicalMonitors->szPhysicalMonitorDescription, szMonitorName, strlen(szMonitorName)) == 0)
        if (match)
        {
            //GetCapabilitiesStringLength(pPhysicalMonitors[0].hPhysicalMonitor, &dwCapStrLen);
            //const auto pCap = static_cast<LPSTR>(malloc(dwCapStrLen));
            //CapabilitiesRequestAndCapabilitiesReply(pPhysicalMonitors[0].hPhysicalMonitor, pCap, dwCapStrLen);
            //free(pCap);
            /*
            Example return:
            (
            prot(monitor)
            type(LCD)
            model(PA278QV)
            cmds(01 02 03 07 0C E3 F3)
            vcp(
                02 04 05 08 10 12 
                14(04 05 08 0B) 16 18 1A 52 
                60(03 11 0F 10) 62 AC AE B6 C0 C6 C8 C9 
                CC(01 02 03 04 05 06 07 08 09 0A 0C 0D 11 12 14 1A 1E 1F 23) 
                D6(01 04 05) 
                DC(00 0B 0D 0E 0F 17 18 21 22)
                DF
                )
            mswhql(1)
            asset_eep(40)
            mccs_ver(2.1)
            )
                */

            if (!SetVCPFeature(pPhysicalMonitors->hPhysicalMonitor, 0x60, dwMode))
            {
                
                std::cout << "SetVCPFeature error " << GetLastError() << "\n";
                nRet = 1;
            }
            bFound = true;
        }
    }

    DestroyPhysicalMonitors(dwPhysMonNum, pPhysicalMonitors);
    free(pPhysicalMonitors);

    return !bFound; //false to stop enumeration
}

void strToLower(char *str)
{
    int i=0;
    while (str[i])
    {
        str[i] = tolower(str[i]);
        i++;
    }
}

void help()
{
    std::cout << "Usage: " << APP_NAME << " <" << PARAM_SELECT << " | " << PARAM_LIST << "> [(partial) monitor name] [" << PARAM_DP << " | " << PARAM_HDMI << "]\nExample:\n\t" << APP_NAME << " " << PARAM_SELECT << " ASUS " << PARAM_DP << "\n\t" << APP_NAME << " " << PARAM_LIST << "\n";
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        help();
        return 1;
    }

    strToLower(argv[1]);

    if (strcmp(argv[1], PARAM_SELECT) == 0)
        dwCommand = CMD_SELECT;
    else if (strcmp(argv[1], PARAM_LIST) == 0)
        dwCommand = CMD_LIST;

    if (dwCommand == CMD_UNKNOWN)
    {
        std::cout << "Unknown command \"" << argv[1] << "\"!\nMust be either \"" << PARAM_SELECT << "\" or \"" << PARAM_LIST << "\"\n";
        help();
        return 1;
    }
    
    if (dwCommand == CMD_SELECT)
    {
        if (argc < 3)
        {
            std::cout << "Monitor name and input type required!\n";
            help();
            return 1;
        }

        if (argc < 4)
        {
            std::cout << "Input type required!\n";
            help();
            return 1;
        }

        szMonitorName = argv[2];

        strToLower(argv[3]);

        if (strcmp(argv[3], PARAM_HDMI) == 0)
            dwMode = HDMI;
        else if (strcmp(argv[3], PARAM_DP) == 0)
            dwMode = DISPLAY_PORT;
        else
        {
            std::cout << "Unknown input type \"" << argv[3] << "\"!\nMust be either \"" << PARAM_DP << "\" or \"" << PARAM_HDMI << "\"\n";
            help();
            return 1;
        }
    }

    EnumDisplayMonitors(nullptr, nullptr, &MyInfoEnumProc, 0);

    if (dwCommand == CMD_SELECT && !bFound)
        std::cout << "Monitor " << szMonitorName << " not found!\n";

    if (nRet == 0 && dwCommand == CMD_SELECT && bFound)
        std::cout << szMonitorName << " " << argv[3] << " input selected\n";

    return nRet;
}
