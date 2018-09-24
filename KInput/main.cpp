/*
    Copyright (C) <2018>  <Kasi Reddy>

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
#include <iostream>
#include <MinHook.h>
#include "KInput.hpp"

KInput* Input = nullptr;

typedef HWND(__stdcall *ptr_CreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

ptr_CreateWindowExW CreateWindowExW_Original = nullptr;
LPVOID *CreateWindowExW_Address = nullptr;

HWND __stdcall CreateWindowExW_Hook(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    HWND Temp = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    std::wstring WStr(lpClassName);
    std::string ClassNameString(WStr.begin(), WStr.end());
    if (ClassNameString == "SunAwtCanvas" && Input) {
        Input->NotifyCanvasUpdate(Temp);
    }
    return Temp;
}

bool HookCreateWindow() {
    HMODULE kernel = GetModuleHandle("user32.dll");
    CreateWindowExW_Address = (LPVOID *) GetProcAddress(kernel, "CreateWindowExW");
    if (!CreateWindowExW_Address) {
        return false;
    }

    // Initialize MinHook.
    if (MH_Initialize() != MH_OK)
    {
        return false;
    }

    // Create a hook for MessageBoxW, in disabled state.
    if (MH_CreateHook(CreateWindowExW_Address, ((LPVOID *) &CreateWindowExW_Hook),
                      ((LPVOID *) &CreateWindowExW_Original)) != MH_OK)
    {
        return false;
    }

    // Enable the hook for CreateWindowExW.
    return MH_EnableHook(CreateWindowExW_Address) == MH_OK;
}

bool UnHookCreateWindow() {
    // Disable the hook.
    if (MH_DisableHook(CreateWindowExW_Address) != MH_OK)
    {
        return false;
    }

    // Uninitialize MinHook.
    return MH_Uninitialize() == MH_OK;
}

extern "C"
__declspec(dllexport)
bool KInput_FocusEvent(void* Data)
{
    if (!Input)
        return false;
    struct FocusEvent
    {
        std::int32_t ID;
    };
    FocusEvent* Event = (FocusEvent*)Data;
    return Input->FocusEvent(Event->ID);
}

extern "C"
__declspec(dllexport)
bool KInput_KeyEvent(void* Data)
{
    if (!Input)
        return false;
    struct KeyEvent
    {
        std::int32_t ID;
        std::int64_t When;
        std::int32_t Modifiers;
        std::int32_t KeyCode;
        std::uint16_t KeyChar;
        std::int32_t KeyLocation;
    };
    KeyEvent* Event = (KeyEvent*)Data;
    return Input->KeyEvent(Event->ID, Event->When, Event->Modifiers, Event->KeyCode,
                           Event->KeyChar, Event->KeyLocation);
}

extern "C"
__declspec(dllexport)
bool KInput_MouseEvent(void* Data)
{
    if (!Input)
        return false;
    struct MouseEvent
    {
        std::int32_t ID;
        std::int64_t When;
        std::int32_t Modifiers;
        std::int32_t X;
        std::int32_t Y;
        std::int32_t ClickCount;
        bool PopupTrigger;
        std::int32_t Button;
    };
    MouseEvent* Event = (MouseEvent*)Data;
    return Input->MouseEvent(Event->ID, Event->When, Event->Modifiers, Event->X,
                             Event->Y, Event->ClickCount, Event->PopupTrigger, Event->Button);
}

extern "C"
__declspec(dllexport)
bool KInput_MouseWheelEvent(void* Data)
{
    if (!Input)
        return false;
    struct MouseWheelEvent
    {
        std::int32_t ID;
        std::int64_t When;
        std::int32_t Modifiers;
        std::int32_t X;
        std::int32_t Y;
        std::int32_t ClickCount;
        bool PopupTrigger;
        std::int32_t ScrollType;
        std::int32_t ScrollAmount;
        std::int32_t WheelRotation;
    };
    MouseWheelEvent* Event = (MouseWheelEvent*)Data;
    return Input->MouseWheelEvent(Event->ID, Event->When, Event->Modifiers, Event->X,
                                  Event->Y, Event->ClickCount, Event->PopupTrigger, Event->ScrollType,
                                  Event->ScrollAmount, Event->WheelRotation);
}

bool __stdcall DllMain(HMODULE DLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            {
                DisableThreadLibraryCalls(DLL);
                HookCreateWindow();
                Input = new KInput();
            }
            break;
        case DLL_PROCESS_DETACH:
            {
                if (Input)
                {
                    UnHookCreateWindow();
                    delete Input;
                    Input = nullptr;
                }
            }
            break;
        default:
            {

            }
            break;
    }
    return TRUE;
}
