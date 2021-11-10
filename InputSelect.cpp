#include <iostream>
#include <Windows.h>
#include <WinUser.h>
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

#define HDMI 0x11
#define DISPLAY_PORT 0x0F

int nRet = 0;
DWORD dwMode;
bool bFound = false;

static BOOL CALLBACK MyInfoEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    DWORD dwPhysMonNum;
    LPPHYSICAL_MONITOR pPhysicalMonitors = nullptr;
    DWORD dwCapStrLen;

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

    std::wcout << L"Found " << pPhysicalMonitors[0].szPhysicalMonitorDescription << L" monitor\n";

    if (memcmp(pPhysicalMonitors[0].szPhysicalMonitorDescription, L"ASUS", 8) == 0)
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

        if (!SetVCPFeature(pPhysicalMonitors[0].hPhysicalMonitor, 0x60, dwMode))
        {
            std::cout << "SetVCPFeature error!\n";
            nRet = 1;
        }
        bFound = true;
    }

    DestroyPhysicalMonitors(dwPhysMonNum, pPhysicalMonitors);
    free(pPhysicalMonitors);

    return !bFound; //false to stop enumeration
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Either \"hdmi\" or \"dp\" is required!\n";
        return 1;
    }

    if (strcmp(argv[1], "hdmi") == 0)
        dwMode = HDMI;
    else if (strcmp(argv[1], "dp") == 0)
        dwMode = DISPLAY_PORT;
    else
    {
        std::cout << "Unknown input type \"" << argv[1] << "\"!\nMust be either \"hdmi\" or \"dp\"\n";
        return 1;
    }

    EnumDisplayMonitors(nullptr, nullptr, &MyInfoEnumProc, 0);

    if (!bFound)
        std::cout << "ASUS monitor not found!\n";

    if (nRet == 0 && bFound)
        std::cout << argv[1] << " input selected\n";

    return nRet;
}
