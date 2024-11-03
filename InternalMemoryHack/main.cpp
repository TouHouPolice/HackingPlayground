#include <Windows.h>  
#include <iostream>

unsigned char* const hook_location = (unsigned char*)0x00CCAF8A;
constexpr DWORD ret_address = 0xCCAF90;
volatile DWORD* const player_base = reinterpret_cast<DWORD*> (0x017EEFEC + 0x60);
volatile DWORD* const game_base = reinterpret_cast<DWORD*> (*player_base + 0xA90);
volatile DWORD* const gold = reinterpret_cast<DWORD*>(*game_base + 0x4);

__declspec(naked) void codecave() {
	__asm {
		pushad
	}

	*gold = 888;

	__asm {
		popad
		mov eax, dword ptr ds : [ecx]
		lea esi, dword ptr ds : [esi]
		jmp ret_address
	}
}



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

	DWORD old_protect;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		//MessageBoxA(NULL, "Injection Successful!", "DLL Injection", MB_OK | MB_ICONINFORMATION);
		// Code to execute when the process is loaded
		std::cout << "DLL Injected!" << std::endl;
		VirtualProtect((void*)hook_location, 6, PAGE_EXECUTE_READWRITE, &old_protect);
		*hook_location = 0xE9;
		*(DWORD*)(hook_location + 1) = (DWORD)&codecave - ((DWORD)hook_location + 5);
		*(hook_location + 5) = 0x90;
	}

	return true;
}

//int main() {
//	constexpr DWORD base_address = 0x017EEFEC;
//
//	const HWND wesnoth_window = FindWindow(NULL, L"The Battle for Wesnoth - 1.14.9");
//	DWORD process_id = 0;
//	GetWindowThreadProcessId(wesnoth_window, &process_id);
//
//	const HANDLE wesnoth_process = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
//	if (wesnoth_process == NULL) {
//		// Handle error  
//		std::cout << std::format("Unable to open process, Error: {0}", GetLastError()) << std::endl;
//		return 1;
//	}
//
//	OffsetChain<DWORD> gold_chain{};
//	gold_chain.base(base_address)
//		.offsets({ 0x60, 0xA90, 0x4 })
//		.resolve_destination_address(wesnoth_process);
//
//	{
//		auto [gold_value, bytes_read] = gold_chain.read_dest<DWORD>(wesnoth_process);
//		std::cout << std::format("Bytes Read: {}, Gold: {}\n", bytes_read, gold_value);
//	}
//
//	constexpr DWORD new_gold_value = 9999;
//	std::cout << std::format("Writing new gold value [{}]...\n", new_gold_value);
//	gold_chain.write_dest<DWORD>(wesnoth_process, new_gold_value);
//
//	// read the value again to verify that it was written
//	{
//		auto [gold_value, bytes_read] = gold_chain.read_dest<DWORD>(wesnoth_process);
//		std::cout << std::format("Bytes Read: {}, Gold: {}\n", bytes_read, gold_value);
//	}
//
//	CloseHandle(wesnoth_process);
//
//	// Wait for user input before closing the console window
//	//std::cout << "Press Enter to exit...";
//	//std::cin.get();
//
//	return 0;
//}