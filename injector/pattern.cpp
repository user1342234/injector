#include "pattern.h"

int CheckMask(unsigned char* base, unsigned char* pattern, unsigned char* mask)
{
	for (; *mask; ++base, ++pattern, ++mask)
		if (*mask == 'x' && *base != *pattern)
			return 0;
	return 1;
}

void* FindPatternEx(unsigned char* base, UINT64 size, unsigned char* pattern, unsigned char* mask)
{
	size -= strlen((const char*)mask);
	for (UINT64 i = 0; i <= size; ++i) {
		void* addr = &base[i];
		if (CheckMask((unsigned char*)addr, pattern, mask))
			return addr;
	}
	return 0;
}

UINT64 FindPattern(UINT64 base, unsigned char* pattern, unsigned char* mask)
{
	if (!base)
		return 0;

	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)base;
	PIMAGE_NT_HEADERS64 nt_header = (PIMAGE_NT_HEADERS64)(base + dos_hdr->e_lfanew); // IMAGE_DOS_HEADER+lfanew
	
	PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(nt_header);
	UINT16 num_of_sections = nt_header->FileHeader.NumberOfSections;

	for (UINT16 i = 0; i < num_of_sections; i++) {
		PIMAGE_SECTION_HEADER cur_section = &section_header[i];
		UINT32 section_characteristics = cur_section->Characteristics; // gettung characteristics

		if (section_characteristics & IMAGE_SCN_CNT_CODE && !(section_characteristics & IMAGE_SCN_MEM_DISCARDABLE))
		{
			UINT64 virtual_address = (cur_section->VirtualAddress + base);
			UINT32 virtual_size = cur_section->Misc.VirtualSize;

			UINT64 addr = (UINT64)FindPatternEx((PUCHAR)virtual_address, virtual_size, pattern, mask);
			if (addr)
				return addr;
		}
	}
	return 0;
}