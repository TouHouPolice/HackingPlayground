#include <Windows.h>  
#include <iostream>
#include <format>

#include "Alias.hpp"  
#include "OffsetChain.hpp"



int main() {
	constexpr DWORD base_address = 0x017EEFEC;

	const HWND wesnoth_window = FindWindow(NULL, L"The Battle for Wesnoth - 1.14.9");
	DWORD process_id = 0;
	GetWindowThreadProcessId(wesnoth_window, &process_id);

	const HANDLE wesnoth_process = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
	if (wesnoth_process == NULL) {
		// Handle error  
		std::cout << std::format("Unable to open process, Error: {0}", GetLastError()) << std::endl;
		return 1;
	}

	OffsetChain<DWORD> gold_chain{};
	gold_chain.base(base_address)
		.offsets({ 0x60, 0xA90, 0x4 })
		.resolve_destination_address(wesnoth_process);

	{
		auto [gold_value, bytes_read] = gold_chain.read_dest<DWORD>(wesnoth_process);
		std::cout << std::format("Bytes Read: {}, Gold: {}\n", bytes_read, gold_value);
	}

	constexpr DWORD new_gold_value = 9999;
	std::cout << std::format("Writing new gold value [{}]...\n", new_gold_value);
	gold_chain.write_dest<DWORD>(wesnoth_process, new_gold_value);

	// read the value again to verify that it was written
	{
		auto [gold_value, bytes_read] = gold_chain.read_dest<DWORD>(wesnoth_process);
		std::cout << std::format("Bytes Read: {}, Gold: {}\n", bytes_read, gold_value);
	}

	CloseHandle(wesnoth_process);

	// Wait for user input before closing the console window
	std::cout << "Press Enter to exit...";
	std::cin.get();

	return 0;
}