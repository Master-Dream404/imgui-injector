#include "injector.h"
#include <iostream>
#include <filesystem>

#include <Windows.h>
#include <TlHelp32.h>

injector& injector::get_instance()
{
	static injector instance;
	return instance;
}
process& process::get_instance()
{
    static process instance;
    return instance;
}

DWORD injector::find_game(const std::string& game)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snap, &pe))
    {
        do
        {
            if (std::string(pe.szExeFile).find(game) != std::string::npos)
            {
                CloseHandle(snap);
                process::get_instance().game = pe.szExeFile;
                process::get_instance().pid = pe.th32ProcessID;
                process::get_instance().locked = true;
                HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
                if (handle)
                {
                    DWORD buffSize = 1024;
                    CHAR buffer[1024];
                    if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
                    {
                        process::get_instance().file_path = buffer;
                       
                    }
                }
                return pe.th32ProcessID;
            }
        } while (Process32Next(snap, &pe));
    }

    CloseHandle(snap);
    return 0;
}

std::uint64_t process::GetModuleBaseAddress(const char* modname)
{
    HWND hwnd = FindWindowA(NULL, modname);
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process::get_instance().pid);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry)) {
            do
            {
                if (!strcmp(modEntry.szModule, modname))
                {
                    CloseHandle(hSnap);
                    return (std::uint64_t)modEntry.modBaseAddr;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
}

void process::WriteProcess()
{
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process::get_instance().pid);
    ReadProcessMemory(handle, (void*)process::get_instance().address, &process::get_instance().ptraddress, sizeof(DWORD), 0);
}

std::string process::get_address(std::string game)
{
    process::get_instance().address = process::get_instance().GetModuleBaseAddress(game.c_str());
    process::get_instance().base_address = process::get_instance().GetModuleBaseAddress(game.c_str());
    return std::to_string(process::get_instance().base_address);
}

void injector::inject(DWORD pid, const std::string& path)
{
    system("cls");
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (!process)
    {
        std::cout << "Failed to open process.\n";
        return;
    }

    void* buffer = VirtualAllocEx(process, nullptr, path.size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!buffer)
    {
        std::cout << "Failed to create DLL path buffer in target process.\n";
        return;
    }

    if (!WriteProcessMemory(process, buffer, path.c_str(), path.size(), nullptr))
    {
        VirtualFreeEx(process, buffer, 0, MEM_RELEASE);
        CloseHandle(process);

        std::cout << "Failed to write DLL path to the buffer in target process.\n";
        return;
    }

    HANDLE thread = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, buffer, 0, nullptr);
    WaitForSingleObject(thread, INFINITE);

    DWORD threadResult = 0;
    GetExitCodeThread(thread, &threadResult);

    if (threadResult != 0)
        std::cout << "Successfully injected module into the target process.\n";
    else
        std::cout << "Failed to inject into the target process.\n";

    VirtualFreeEx(process, buffer, 0, MEM_RELEASE);
    CloseHandle(process);

}