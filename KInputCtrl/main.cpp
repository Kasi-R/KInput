#include <windows.h>
#include <map>
#include <iostream>
#include "KInputCtrl.hpp"

std::map<DWORD, KInputCtrl*> Clients;

extern "C"
__declspec(dllexport)
bool KInput_Create(DWORD PID)
{
    if (!Clients.count(PID))
    {
        Clients[PID] = new KInputCtrl(PID);
        return true;
    }
    return false;
}

extern "C"
__declspec(dllexport)
bool KInput_Delete(DWORD PID)
{
    if (Clients.count(PID))
    {
        delete Clients[PID];
        Clients[PID] = nullptr;
        Clients.erase(PID);
        return true;
    }
    return false;
}

extern "C"
__declspec(dllexport)
bool KInput_FocusEvent(DWORD PID, std::int32_t ID)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->FocusEvent(ID);
}

extern "C"
__declspec(dllexport)
bool KInput_KeyEvent(DWORD PID, std::int32_t ID, std::int64_t When, std::int32_t Modifiers,
                     std::int32_t KeyCode, std::uint16_t KeyChar, std::int32_t KeyLocation)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->KeyEvent(ID, When, Modifiers, KeyCode, KeyChar, KeyLocation);
}

extern "C"
__declspec(dllexport)
bool KInput_MouseEvent(DWORD PID, std::int32_t ID, std::int64_t When, std::int32_t Modifiers,
                       std::int32_t X, std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger,
                       std::int32_t Button)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->MouseEvent(ID, When, Modifiers, X, Y, ClickCount, PopupTrigger, Button);
}

extern "C"
__declspec(dllexport)
bool KInput_MouseWheelEvent(DWORD PID, std::int32_t ID, std::int64_t When, std::int32_t Modifiers,
                            std::int32_t X, std::int32_t Y, std::int32_t ClickCount, bool PopupTrigger,
                            std::int32_t ScrollType, std::int32_t ScrollAmount, std::int32_t WheelRotation)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->MouseWheelEvent(ID, When, Modifiers, X, Y, ClickCount, PopupTrigger, ScrollType, ScrollAmount, WheelRotation);
}

BOOL __stdcall DllMain(HMODULE DLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
                DisableThreadLibraryCalls(DLL);
            }
            break;
        case DLL_PROCESS_DETACH:
            {

            }
            break;
        default:
            {

            }
            break;
    }
    return TRUE;
}
