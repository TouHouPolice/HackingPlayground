#include <Windows.h>
#include <iostream>
#include <format>

int main() {

	const char* dll_path = "C:\\Users\\a2376\\source\\repos\\hacking-playground\\Debug\\InternalMemoryHack.dll";


	constexpr DWORD base_address = 0x017EEFEC;

	const HWND wesnoth_window = FindWindow(NULL, L"The Battle for Wesnoth - 1.14.9");
	DWORD process_id = 0;
	GetWindowThreadProcessId(wesnoth_window, &process_id);

	std::cout << "Opening process" << std::endl;
	const HANDLE wesnoth_process = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
	if (wesnoth_process == NULL) {
		// Handle error  
		std::cout << std::format("Unable to open process, Error: {0}", GetLastError()) << std::endl;
		return 1;
	}

	std::cout << "Allocating memory in the target process" << std::endl;
	LPVOID remoteMemory = VirtualAllocEx(wesnoth_process, nullptr, strlen(dll_path) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!remoteMemory)
	{
		std::cerr << "Failed to allocate memory in the target process.\n";
		CloseHandle(wesnoth_process);
		return 1;
	}

	std::cout << "Writing DLL path to target process memory" << std::endl;
	if (!WriteProcessMemory(wesnoth_process, remoteMemory, dll_path, strlen(dll_path) + 1, nullptr))
	{
		std::cerr << "Failed to write DLL path to target process memory.\n";
		VirtualFreeEx(wesnoth_process, remoteMemory, 0, MEM_RELEASE);
		CloseHandle(wesnoth_process);
		return 1;
	}

	std::cout << "Getting LoadLibraryA address" << std::endl;
	LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
	if (!loadLibraryAddr)
	{
		std::cerr << "Failed to locate LoadLibraryA.\n";
		VirtualFreeEx(wesnoth_process, remoteMemory, 0, MEM_RELEASE);
		CloseHandle(wesnoth_process);
		return 1;
	}

	std::cout << "Creating remote thread" << std::endl;
	HANDLE hThread = CreateRemoteThread(wesnoth_process, nullptr, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteMemory, 0, nullptr);
	if (!hThread)
	{
		std::cerr << "Failed to create remote thread.\n";
		VirtualFreeEx(wesnoth_process, remoteMemory, 0, MEM_RELEASE);
		CloseHandle(wesnoth_process);
		return 1;
	}

	std::cout << "Waiting for DLL to be loaded" << std::endl;
	// Wait for the DLL to be loaded
	WaitForSingleObject(hThread, INFINITE);
	std::cout << "DLL Loaded" << std::endl;

	// Press any key to continue
	std::cout << "Press any key to exit" << std::endl;
	std::cin.get();

	// Cleanup
	CloseHandle(hThread);
	VirtualFreeEx(wesnoth_process, remoteMemory, 0, MEM_RELEASE);
	CloseHandle(wesnoth_process);




	return 0;
}