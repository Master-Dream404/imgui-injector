#pragma once
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
class injector
{
private:
	int pid;
public:
	static injector& get_instance();
	DWORD find_game(const std::string& game);
	void inject(DWORD pid, const std::string& path);
};

class process : public injector {
public:
	static process& get_instance();
	int pid;
	std::string game, file_path, window_name, cheat_path;
	bool locked;
	std::string get_address(std::string game);
	int base_address;
	DWORD ptraddress;
	void WriteProcess();
private:
	std::uint64_t GetModuleBaseAddress(const char* modname);
	std::uint64_t address;
};
