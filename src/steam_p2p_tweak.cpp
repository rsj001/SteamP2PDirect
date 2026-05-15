#pragma comment(linker, "/export:GetFileVersionInfoA=C:\\Windows\\System32\\version.GetFileVersionInfoA,@1")
#pragma comment(linker, "/export:GetFileVersionInfoByHandle=C:\\Windows\\System32\\version.GetFileVersionInfoByHandle,@2")
#pragma comment(linker, "/export:GetFileVersionInfoExA=C:\\Windows\\System32\\version.GetFileVersionInfoExA,@3")
#pragma comment(linker, "/export:GetFileVersionInfoExW=C:\\Windows\\System32\\version.GetFileVersionInfoExW,@4")
#pragma comment(linker, "/export:GetFileVersionInfoSizeA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeA,@5")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExA,@6")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExW,@7")
#pragma comment(linker, "/export:GetFileVersionInfoSizeW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeW,@8")
#pragma comment(linker, "/export:GetFileVersionInfoW=C:\\Windows\\System32\\version.GetFileVersionInfoW,@9")
#pragma comment(linker, "/export:VerFindFileA=C:\\Windows\\System32\\version.VerFindFileA,@10")
#pragma comment(linker, "/export:VerFindFileW=C:\\Windows\\System32\\version.VerFindFileW,@11")
#pragma comment(linker, "/export:VerInstallFileA=C:\\Windows\\System32\\version.VerInstallFileA,@12")
#pragma comment(linker, "/export:VerInstallFileW=C:\\Windows\\System32\\version.VerInstallFileW,@13")
#pragma comment(linker, "/export:VerLanguageNameA=C:\\Windows\\System32\\version.VerLanguageNameA,@14")
#pragma comment(linker, "/export:VerLanguageNameW=C:\\Windows\\System32\\version.VerLanguageNameW,@15")
#pragma comment(linker, "/export:VerQueryValueA=C:\\Windows\\System32\\version.VerQueryValueA,@16")
#pragma comment(linker, "/export:VerQueryValueW=C:\\Windows\\System32\\version.VerQueryValueW,@17")

#include <Windows.h>
#include <fstream>
#include "MinHook.h"

typedef bool(__cdecl *SteamAPI_RunCallbacks_t)();
typedef void(__cdecl *SetDebugOutput_t)(void *pUtils, int nLogLevel, void(__cdecl *pfnFunc)(int nLogLevel, const char *pMsg));
SteamAPI_RunCallbacks_t fpSteamAPI_RunCallbacks = nullptr;
SetDebugOutput_t fpSetDebugOutput = nullptr;
typedef void *(__cdecl *GetUtils_t)();
typedef bool(__cdecl *SetConfig_t)(void *pUtils, int eConfigValue, int nValue);

#define LOG_DEBUG
void __stdcall MySteamNetDebug(int nType, const char *pszMsg)
{
    static FILE *g_LogFile = nullptr;
    static LONG g_Init = 0;
    if (InterlockedCompareExchange(&g_Init, 1, 0) == 0)
    {
        fopen_s(&g_LogFile, "force_ice_steam_dbg.log", "a");
    }
    if (!g_LogFile || !pszMsg)
        return;
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(g_LogFile,
            "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\n",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
            pszMsg);
}

void ApplySDRPenalty()
{
    static HMODULE hSteamApi = GetModuleHandleA("steam_api64.dll");
    if (!hSteamApi)
    {
        MessageBoxA(NULL, "Failed to get steam_api64.dll in Detour", "Failure", MB_ICONWARNING);
    }
    auto SteamAPI_SteamNetworkingUtils_vxxx = (GetUtils_t)GetProcAddress(hSteamApi, "SteamAPI_SteamNetworkingUtils_SteamAPI_v003");
    if (!SteamAPI_SteamNetworkingUtils_vxxx)
    {
        SteamAPI_SteamNetworkingUtils_vxxx = (GetUtils_t)GetProcAddress(hSteamApi, "SteamAPI_SteamNetworkingUtils_SteamAPI_v004");
    }
    if (!SteamAPI_SteamNetworkingUtils_vxxx)
    {
        MessageBoxA(NULL, "Failed to get SteamNetworkingUtils! Check flattened symbol name.", "Failure", MB_ICONWARNING);
    }
    auto Flat_SetGlobalConfigValueInt32 = (SetConfig_t)GetProcAddress(hSteamApi, "SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32");
    if (SteamAPI_SteamNetworkingUtils_vxxx && Flat_SetGlobalConfigValueInt32)
    {
        // utils = SteamNetworkingUtils()
        void *utilsPtr = SteamAPI_SteamNetworkingUtils_vxxx();
        if (utilsPtr)
        {
            // 106 = k_ESteamNetworkingConfig_P2P_Transport_SDR_Penalty
            Flat_SetGlobalConfigValueInt32(utilsPtr, 106, 200);
            // 105 = k_ESteamNetworkingConfig_P2P_Transport_ICE_Penalty
            Flat_SetGlobalConfigValueInt32(utilsPtr, 105, 0);
            // Allow ALL ICE routes
            // 104 = k_ESteamNetworkingConfig_P2P_Transport_ICE_Enable
            // 0x7fffffff = k_nSteamNetworkingConfig_P2P_Transport_ICE_Enable_All
            Flat_SetGlobalConfigValueInt32(utilsPtr, 104, 0x7fffffff);
#ifdef LOG_DEBUG
            auto SetDebugFunc = (SetDebugOutput_t)GetProcAddress(hSteamApi, "SteamAPI_ISteamNetworkingUtils_SetDebugOutputFunction");
            if (SetDebugFunc)
            {
                SetDebugFunc(utilsPtr, 4, MySteamNetDebug);
            }
            else
            {
                MessageBoxA(NULL, "Failed to get SetDebugFunc!", "Logging Failure", MB_ICONWARNING);
            }
/*
	k_ESteamNetworkingConfig_LogLevel_AckRTT = 13, // [connection int32] RTT calculations for inline pings and replies
	k_ESteamNetworkingConfig_LogLevel_PacketDecode = 14, // [connection int32] log SNP packets send/recv
	k_ESteamNetworkingConfig_LogLevel_Message = 15, // [connection int32] log each message send/recv
	k_ESteamNetworkingConfig_LogLevel_PacketGaps = 16, // [connection int32] dropped packets
	k_ESteamNetworkingConfig_LogLevel_P2PRendezvous = 17, // [connection int32] P2P rendezvous messages
	k_ESteamNetworkingConfig_LogLevel_SDRRelayPings = 18, // [global int32] Ping relays
*/
            // 6 = k_ESteamNetworkingSocketsDebugOutputType_Verbose
            Flat_SetGlobalConfigValueInt32(utilsPtr, 13, 6);
            Flat_SetGlobalConfigValueInt32(utilsPtr, 14, 6);
            // Flat_SetGlobalConfigValueInt32(utilsPtr, 15, 6);
            Flat_SetGlobalConfigValueInt32(utilsPtr, 16, 6);
            Flat_SetGlobalConfigValueInt32(utilsPtr, 17, 6);
            Flat_SetGlobalConfigValueInt32(utilsPtr, 18, 6);
#endif
        }
    }
}

void __cdecl DetourSteamAPI_RunCallbacks()
{
    static int whatever_countdown = 512;
    fpSteamAPI_RunCallbacks();
    if (whatever_countdown--)
    {
        ApplySDRPenalty();
    }
}

void StartHook()
{
    if (MH_Initialize() != MH_OK)
        return;
    HMODULE hSteamApi = GetModuleHandleA("steam_api64.dll");
    int retryCount = 0;
    const int totalRetryCount = 25;
    while (!hSteamApi && retryCount < totalRetryCount)
    {
        Sleep(100);
        retryCount++;
        hSteamApi = GetModuleHandleA("steam_api64.dll");
    }

    if (!hSteamApi)
    {
        MessageBoxA(NULL, "Failed to get steam_api64.dll!", "Failure", MB_ICONWARNING);
        MySteamNetDebug(0, "Failed to get steam_api64.dll!");
        return;
    }
    else
    {
        MySteamNetDebug(0, "--------------------------------------------------------------");
        MySteamNetDebug(0, "Get steam_api64.dll successfully.");
    }
    LPVOID targetAddr = GetProcAddress(hSteamApi, "SteamAPI_RunCallbacks");
    if (targetAddr)
    {
        MH_CreateHook(targetAddr, &DetourSteamAPI_RunCallbacks, reinterpret_cast<LPVOID *>(&fpSteamAPI_RunCallbacks));
        MH_EnableHook(targetAddr);
    }
    else
    {
        MessageBoxA(NULL, "Failed to get SteamAPI_RunCallbacks!", "Failure", MB_ICONWARNING);
    }
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartHook, NULL, 0, NULL);
    }
    return TRUE;
}
