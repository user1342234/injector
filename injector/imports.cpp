#include "imports.h"

DEFINE_IMPORT(
	"\x48\x8B\xC4\x48\x89\x58\x10\x44\x89\x48\x20\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x81\xEC\x00\x00\x00\x00\x48\xF7\xC1\x00\x00\x00\x00\x0F\x57\xC9\xBE\x00\x00\x00\x00\x8B\xDA\x40\x0F\x95\xC6\x0F\x57\xC0",
	"xxxxxxxxxxxxxxxxxxxxxxxxx????xxx????xxxx????xxxxxxxxx",
	void*,
	MmAllocateIndependentPagesEx, SIZE_T, int, UINT64*, UINT32)

DEFINE_VAR(
		"",
		"",
		MmPfnDatabase)

	DEFINE_IMPORT(
		"\x48\x8B\xC4\x57\x48\x83\xEC\x40\x48\x89\x58\x08\x41\x8B\xF8\x48\x89\x68\x10\x48\x89\x70\x18\x41\x8B\xF0\x4C\x89\x60\xF0",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		void*,
		MiMakeValidPte, void*, UINT64, UINT64)

	DEFINE_IMPORT(
		"\x48\x89\x5C\x24\x00\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8B\xEC\x48\x83\xEC\x60\x48\x83\x65\x00\x00\xBE\x00\x00\x00\x00\x48\x83\x65\x00\x00\x0F\x57\xC0\x48\xF7\xC2\x00\x00\x00\x00", 
		"xxxx?xxxxxxxxxxxxxxxxxxxxx??x????xxx??xxxxxx????",
		void,
		MmFreeIndependentPages, PVOID, SIZE_T)
NTSTATUS FindImports(UINT64 kernel_base) {

	RESOLVE_IMPORT(MmAllocateIndependentPagesEx, kernel_base)
		if (!MmAllocateIndependentPagesEx)
			return STATUS_UNSUCCESSFUL;
	ACCESS(MmPfnDatabase) = (IMPORT_VAR)g_KdBlock.MmPfnDatabase;
		if (!ACCESS(MmPfnDatabase))
			return STATUS_UNSUCCESSFUL;
		RESOLVE_IMPORT(MiMakeValidPte, kernel_base)
			if (!MiMakeValidPte)
				return STATUS_UNSUCCESSFUL;
		RESOLVE_IMPORT(MmFreeIndependentPages, kernel_base)
			if (!MmFreeIndependentPages)
				return STATUS_UNSUCCESSFUL;
	return STATUS_SUCCESS;
}

