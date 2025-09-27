#pragma once
#ifndef IMPORTS_H
#define IMPORTS_H
#include "nt.h"
#include "pattern.h"
#include "_imports_readonly.h"
#include "kdbg.h"

typedef struct _IMPORT {
	unsigned char pattern[128];
	unsigned char mask[128];
	PVOID pointer;
}IMPORT, * PIMPORT;

	
#define RESOLVE_POINTER(address, instr_size) ((PUCHAR)address + instr_size + sizeof(UINT32) + *(UINT32*)(address + instr_size))
#define CALL(import_name, ...) ((pfn##import_name)import_name)(__VA_ARGS__);
#define ACCESS(import_var_name) (_IMPORT_VAR(import_var_name))



DECLARE_IMPORT(void*, MmAllocateIndependentPagesEx, SIZE_T, int, UINT64*, UINT32)
DECLARE_IMPORT(void*, MiMakeValidPte, void*, UINT64, UINT64)
DECLARE_IMPORT(void, MmFreeIndependentPages, PVOID, SIZE_T)
DECLARE_VAR(MmPfnDatabase);

NTSTATUS FindImports(UINT64 kernel_base);

#endif