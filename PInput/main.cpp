/*
    Copyright (C) <2018>  <Andre>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <map>
#include <iostream>
#include <chrono>
#include "KInputCtrl.hpp"
#include "PInput.hpp"

static HMODULE This = nullptr;
static std::map<DWORD, PInput*> Clients;

static const char* SimbaExports[] =
{
    "PInput_Create", "function PInput_Create(PID : Int32) : Boolean;",

    "PInput_PressKey", "function PInput_PressKey(PID : Int32; KeyCode : Int32) : Boolean;",
    "PInput_KeyUp", "function PInput_KeyUp(PID : Int32; KeyCode : Int32) : Boolean;",
    "PInput_KeyDown", "function PInput_KeyDown(PID : Int32; KeyCode : Int32) : Boolean;",
    "PInput_IsKeyDown", "function PInput_IsKeyDown(PID : Int32; KeyCode : Int32) : Boolean;",
    "PInput_SendKeys", "function PInput_SendKeys(PID: Int32; Text : string; KeyWait, KeyModWait: integer) : Boolean;",

    "PInput_GainFocus", "function PInput_GainFocus(PID: Int32) : Boolean",
    "PInput_LoseFocus", "function PInput_LoseFocus(PID: Int32; AltTab : Boolean) : Boolean",
    "PInput_IsFocused", "function PInput_IsFocused(PID: Int32) : Boolean",

    "PInput_GetMousePos", "function PInput_GetMousePos(PID: Int32; var X, Y: Int32) : Boolean",
    "PInput_IsMouseButtonDown", "function PInput_IsMouseButtonDown(PID: Int32; ClickType: TClickType) : Boolean",
    "PInput_MoveMouse", "function PInput_MoveMouse(PID: Int32; X, Y: Int32) : Boolean",
    "PInput_HoldMouse", "function PInput_HoldMouse(PID: Int32; X, Y: Int32; ClickType: TClickType) : Boolean",
    "PInput_ReleaseMouse", "function PInput_ReleaseMouse(PID: Int32; X, Y: Int32; ClickType: TClickType) : Boolean",
    "PInput_ClickMouse", "function PInput_ClickMouse(PID: Int32; X, Y: Int32; ClickType: TClickType) : Boolean",
    "PInput_ScrollMouse", "function PInput_ScrollMouse(PID: Int32; X, Y, Lines: Int32) : Boolean",

    "PInput_Delete", "function PInput_Delete(PID : Int32) : Boolean;"
};

static constexpr std::int32_t ExportSize = ((sizeof(SimbaExports) / 2) / sizeof(const char*));

extern "C"
__declspec(dllexport)
std::int32_t GetPluginABIVersion()
{
    return 2;
}

extern "C"
__declspec(dllexport)
std::int32_t GetFunctionCount()
{
    return ExportSize;
}

extern "C"
__declspec(dllexport)
std::int32_t GetFunctionInfo(std::int32_t Index, void* &Address, char* &Definition)
{
    if (Index < ExportSize)
    {
        Address = (void*)GetProcAddress(This, SimbaExports[Index * 2]);
        strcpy(Definition, SimbaExports[(Index * 2) + 1]);
        return Index;
    }
    return -1;
}

extern "C"
__declspec(dllexport)
bool PInput_Create(DWORD PID)
{
    if (!Clients.count(PID))
    {
        std::string FilePath(MAX_PATH, '\0');
        GetModuleFileName(This, FilePath.data(), FilePath.size());
        FilePath = FilePath.substr(0, FilePath.find_last_of("/\\"));
        std::cout << "Searching KInput.dll in " << FilePath << std::endl;
        Clients[PID] = new PInput(PID, FilePath);
        return true;
    }
    return false;
}

extern "C"
__declspec(dllexport)
bool PInput_PressKey(DWORD PID, std::int32_t KeyCode)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->PressKey(KeyCode);
}

extern "C"
__declspec(dllexport)
bool PInput_KeyDown(DWORD PID, std::int32_t KeyCode)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->KeyDown(KeyCode);
}

extern "C"
__declspec(dllexport)
bool PInput_KeyUp(DWORD PID, std::int32_t KeyCode)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->KeyUp(KeyCode);
}

extern "C"
__declspec(dllexport)
bool PInput_IsKeyDown(DWORD PID, std::int32_t KeyCode)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->IsKeyDown(KeyCode);
}

extern "C"
__declspec(dllexport)
bool PInput_SendKeys(DWORD PID, char* Text, std::int32_t KeyWait, std::int32_t KeyModWait)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->SendKeys(Text, KeyWait, KeyModWait);
}

extern "C"
__declspec(dllexport)
bool PInput_GainFocus(DWORD PID)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->GainFocus();
}

extern "C"
__declspec(dllexport)
bool PInput_LoseFocus(DWORD PID, bool AltTab)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->LoseFocus(AltTab);
}

extern "C"
__declspec(dllexport)
bool PInput_IsFocused(DWORD PID)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->IsFocused();
}

extern "C"
__declspec(dllexport)
bool PInput_GetMousePos(DWORD PID, std::int32_t *X, std::int32_t *Y)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->GetMousePos(X, Y);
}

extern "C"
__declspec(dllexport)
bool PInput_IsMouseButtonDown(DWORD PID, std::int32_t ClickType)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->IsMouseButtonDown(ClickType);
}

extern "C"
__declspec(dllexport)
bool PInput_MoveMouse(DWORD PID, std::int32_t X, std::int32_t Y)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->MoveMouse(X, Y);
}

extern "C"
__declspec(dllexport)
bool PInput_HoldMouse(DWORD PID, std::int32_t X, std::int32_t Y, std::int32_t ClickType)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->HoldMouse(X, Y, ClickType);
}

extern "C"
__declspec(dllexport)
bool PInput_ReleaseMouse(DWORD PID, std::int32_t X, std::int32_t Y, std::int32_t ClickType)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->ReleaseMouse(X, Y, ClickType);
}

extern "C"
__declspec(dllexport)
bool PInput_ClickMouse(DWORD PID, std::int32_t X, std::int32_t Y, std::int32_t ClickType)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->ClickMouse(X, Y, ClickType);
}

extern "C"
__declspec(dllexport)
bool PInput_ScrollMouse(DWORD PID, std::int32_t X, std::int32_t Y, std::int32_t Lines)
{
    if (!Clients.count(PID))
        return false;
    return Clients[PID]->ScrollMouse(X, Y, Lines);
}

extern "C"
__declspec(dllexport)
bool PInput_Delete(DWORD PID)
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

bool __stdcall DllMain(HMODULE DLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
                This = DLL;
                DisableThreadLibraryCalls(This);
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
