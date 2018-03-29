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

#include "Injector.hpp"
#include <iostream>

Injector::Injector(DWORD PID) : PID(PID), ProcessHandle(nullptr)
{
    this->ProcessHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, PID);
}

void* Injector::GetRemoteProcAddress(void* DLL, std::string ProcName)
{
    void* Result = nullptr;
    if ((!DLL) || (!this->ProcessHandle))
        return Result;
    void* GetProcAddr = (void*)GetProcAddress(GetModuleHandle("Kernel32.dll"), "GetProcAddress");
    void* RemoteParam = VirtualAllocEx(this->ProcessHandle, nullptr, sizeof(char) * (ProcName.size() + 1), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    void* RemoteResult = VirtualAllocEx(this->ProcessHandle, nullptr, sizeof(unsigned long), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (RemoteParam && RemoteResult)
    {
        if (WriteProcessMemory(this->ProcessHandle, RemoteParam, ProcName.c_str(), sizeof(char) * (ProcName.size() + 1), nullptr))
        {
            unsigned char RemoteThreadBuffer[27] =
            {
                0x68, 0x00, 0x00, 0x00, 0x00,
                0x68, 0x00, 0x00, 0x00, 0x00,
                0xB8, 0x00, 0x00, 0x00, 0x00,
                0xFF, 0xD0,
                0xA3, 0x00, 0x00, 0x00, 0x00,
                0x33, 0xC0,
                0xC2, 0x08, 0x00
            };
            *(unsigned long*)(RemoteThreadBuffer + 0x01) = (unsigned long)RemoteParam;
            *(unsigned long*)(RemoteThreadBuffer + 0x06) = (unsigned long)DLL;
            *(unsigned long*)(RemoteThreadBuffer + 0x0B) = (unsigned long)GetProcAddr;
            *(unsigned long*)(RemoteThreadBuffer + 0x12) = (unsigned long)RemoteResult;
            void* RemoteBuffer = VirtualAllocEx(this->ProcessHandle, nullptr, sizeof(RemoteThreadBuffer), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (RemoteBuffer)
            {
                if (WriteProcessMemory(this->ProcessHandle, RemoteBuffer, RemoteThreadBuffer, sizeof(RemoteThreadBuffer), nullptr))
                {
                    HANDLE RemoteThread = CreateRemoteThread(this->ProcessHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)RemoteBuffer, 0, 0, nullptr);
                    if (RemoteThread)
                    {
                        WaitForSingleObject(RemoteThread, INFINITE);
                        if (GetExitCodeThread(RemoteThread, (LPDWORD)&Result) != STILL_ACTIVE)
                            ReadProcessMemory(this->ProcessHandle, RemoteResult, &Result, sizeof(std::uint32_t), nullptr);
                        CloseHandle(RemoteThread);
                    }
                }
                VirtualFreeEx(this->ProcessHandle, RemoteBuffer, 0, MEM_RELEASE);
            }
        }
    }
    if (RemoteParam)
        VirtualFreeEx(this->ProcessHandle, RemoteParam, 0, MEM_RELEASE);
    if (RemoteResult)
        VirtualFreeEx(this->ProcessHandle, RemoteResult, 0, MEM_RELEASE);
    return Result;
}

bool Injector::Load(std::string DLLPath)
{
    bool Result = false;
    if (!this->ProcessHandle)
        return Result;
    if (Modules.count(DLLPath))
        return true;
    void* LoadLib = (void*)GetProcAddress(GetModuleHandle("Kernel32.dll"), "LoadLibraryA");
    if (LoadLib)
    {
        void* DLLEntry = VirtualAllocEx(this->ProcessHandle, nullptr, sizeof(char) * (DLLPath.size() + 1), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (DLLEntry)
        {
            if (WriteProcessMemory(this->ProcessHandle, DLLEntry, DLLPath.c_str(), sizeof(char) * (DLLPath.size() + 1), nullptr))
            {
                HANDLE RemoteThread = CreateRemoteThread(this->ProcessHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLib, DLLEntry, 0, nullptr);
                if (RemoteThread)
                {
                    WaitForSingleObject(RemoteThread, INFINITE);
                    void* DLL = nullptr;
                    if (GetExitCodeThread(RemoteThread, (LPDWORD)&DLL))
                    {
                        if (DLL)
                        {
                            Modules[DLLPath] = DLL;
                            Result = true;
                        }
                    }
                    CloseHandle(RemoteThread);
                }
            }
            VirtualFreeEx(this->ProcessHandle, DLLEntry, 0, MEM_RELEASE);
        }
    }
    return Result;
}

bool Injector::CallExport(std::string DLLPath, std::string ProcName, void* Data, std::uint32_t Size)
{
    bool Result = false;
    if (!this->ProcessHandle)
        return Result;
    if (!Modules.count(DLLPath))
        return true;
    void* DLL = Modules[DLLPath];
    void* Func = GetRemoteProcAddress(DLL, ProcName);
    if (Func)
    {
        void* DataEntry = VirtualAllocEx(this->ProcessHandle, nullptr, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (DataEntry)
        {
            if (WriteProcessMemory(this->ProcessHandle, DataEntry, Data, Size, nullptr))
            {
                HANDLE RemoteThread = CreateRemoteThread(this->ProcessHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)Func, DataEntry, 0, nullptr);
                if (RemoteThread)
                {
                    WaitForSingleObject(RemoteThread, INFINITE);
                    GetExitCodeThread(RemoteThread, (LPDWORD)&Result);
                    CloseHandle(RemoteThread);
                }
            }
            VirtualFreeEx(this->ProcessHandle, DataEntry, 0, MEM_RELEASE);
        }
    }
    return Result;
}

bool Injector::Free(std::string DLLPath)
{
    bool Result = false;
    if (!this->ProcessHandle)
        return Result;
    if (!Modules.count(DLLPath))
        return true;
    void* DLL = Modules[DLLPath];
    void* FreeLib = (void*)GetProcAddress(GetModuleHandle("Kernel32.dll"), "FreeLibrary");
    if (FreeLib)
    {
        HANDLE RemoteThread = CreateRemoteThread(this->ProcessHandle, nullptr, 0, (LPTHREAD_START_ROUTINE)FreeLib, DLL, 0, nullptr);
        if (RemoteThread)
        {
            WaitForSingleObject(RemoteThread, INFINITE);
            BOOL Freed = FALSE;
            if (GetExitCodeThread(RemoteThread, (LPDWORD)&Freed))
            {
                if (Freed)
                {
                    this->Modules.erase(DLLPath);
                    Result = true;
                }
            }
            CloseHandle(RemoteThread);
        }
    }

    return Result;
}

Injector::~Injector()
{
    if (this->ProcessHandle)
    {
        CloseHandle(this->ProcessHandle);
        this->ProcessHandle = nullptr;
    }
}
