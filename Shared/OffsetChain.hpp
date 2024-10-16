#pragma once
#include <Windows.h>
#include <vector>
#include <exception>

#pragma warning(push)
#pragma warning(disable: 4312)

// This class will be used to store the pointer chain to the values we want to read from the game's memory.
template <typename PointerType>
class OffsetChain
{
public:
	OffsetChain& base(PointerType base_address) {
		m_base_address = base_address;
		return *this;

	}
	OffsetChain& offsets(const std::initializer_list<PointerType>& offsets) 
	{
		m_offset_chain.insert(m_offset_chain.end(), offsets.begin(), offsets.end());
		return *this;
	}

	OffsetChain& resolve_destination_address(const HANDLE process)
	{
		PointerType current_address = m_base_address;
		for (size_t i = 0; i < m_offset_chain.size() - 1; ++i)
		{
			current_address += m_offset_chain[i];
			ReadProcessMemory(process, reinterpret_cast<LPCVOID>(current_address), &current_address, sizeof(current_address), nullptr);
		}
		m_dest_address = current_address + m_offset_chain.back();
		return *this;
	}

	template <typename ValueType>
	std::pair<ValueType, size_t> read_dest(const HANDLE process) const
	{
		ValueType value = 0;
		size_t bytes_read = 0;
		ReadProcessMemory(process, reinterpret_cast<LPCVOID>(m_dest_address), &value, sizeof(value), &bytes_read);
		return { value, bytes_read };
	}

	template <typename ValueType>
	size_t write_dest(const HANDLE process, ValueType val) const
	{
		size_t bytes_written = 0;
		WriteProcessMemory(process, reinterpret_cast<LPVOID>(m_dest_address), &val, sizeof(val), &bytes_written);
		return bytes_written;
	}

	void PrintOffsetChain() const
	{
		std::cout << std::format("Base Address: {0}\n", m_base_address);
		
		for (size_t i = 0; i < m_offset_chain.size(); ++i)
		{
			std::cout << std::format("Offset {0}: {1}\n", i, m_offset_chain[i]);
		}
	}

private:
	std::vector<PointerType> m_offset_chain;
	PointerType m_base_address = 0;
	PointerType m_dest_address = 0;
};

#pragma warning(pop)