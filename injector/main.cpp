#include "nt.h"
#include "logging.h"
#include "debug.h"
#include "communication.h"
#include "imports.h"
#include "kdbg.h"
#define READ_REGISTRY_32 (sizeof(ULONG)*4)
#define READ_REGISTRY_64 (sizeof(ULONG)*3) + sizeof(ULONG64)



NTSTATUS start() {

	
	KIRQL irql = KeGetCurrentIrql();
	if (irql > APC_LEVEL)
		KeLowerIrql(PASSIVE_LEVEL);

	if (!NT_SUCCESS(InitializeDebuggerDataBlock()))
		return STATUS_UNSUCCESSFUL;

	
	UINT64 kernel_base = {}; 
	RtlPcToFileHeader(RtlPcToFileHeader, (PVOID*)&kernel_base);
	if (!NT_SUCCESS(FindImports(kernel_base)))
		return STATUS_UNSUCCESSFUL;
	

	OBJECT_ATTRIBUTES obj_attr;
	UNICODE_STRING filestr = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Information");
	InitializeObjectAttributes(&obj_attr, &filestr, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, NULL);
	HANDLE pKey = {};;
	if (!NT_SUCCESS(ZwOpenKey(&pKey, KEY_ALL_ACCESS, &obj_attr)))
		fail("ZwOpenKey failed \n");

	UINT32 process_id = {};
	UNICODE_STRING process_id_str = RTL_CONSTANT_STRING(L"PROCESSID");
	ULONG res = {};
	PKEY_VALUE_PARTIAL_INFORMATION info = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool2(POOL_FLAG_PAGED, READ_REGISTRY_32, 'enoN');
	if (!info)
		fail("ExAllocatePool2 failed");
	RtlZeroMemory(info, READ_REGISTRY_32);
	if (!NT_SUCCESS(ZwQueryValueKey(pKey, &process_id_str, KeyValuePartialInformation, info, READ_REGISTRY_32, &res)))
		fail("ZwQueryValueKey failed \n");

	memcpy(&process_id, info->Data, sizeof(UINT32));
	ExFreePool(info);

	UINT64 shared_page = {};
	UNICODE_STRING shared_page_str = RTL_CONSTANT_STRING(L"SHAREDPAGE");
	PKEY_VALUE_PARTIAL_INFORMATION info2 = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool2(POOL_FLAG_PAGED, READ_REGISTRY_64, 'enoN');
	if (!info2)
		fail("ExAllocatePool2 failed");
	RtlZeroMemory(info2, READ_REGISTRY_64);
	if (!NT_SUCCESS(ZwQueryValueKey(pKey, &shared_page_str, KeyValuePartialInformation, info2, READ_REGISTRY_64, &res)))
		fail("ZwQueryValueKey failed \n");

	memcpy(&shared_page, info2->Data, sizeof(UINT64));
	ExFreePool(info2);

	log("Process id: %d", process_id);
	log("Shared page: %llx", shared_page);


	PEPROCESS communication_process = {};
	if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)process_id, &communication_process)))
		fail("PsLookupProcessByProcessId");

	ObDereferenceObject(communication_process);

	KAPC_STATE apc = {};
	KeStackAttachProcess(communication_process, &apc);
	
	PREQUEST req = (PREQUEST)shared_page;
	
	UINT64 dll_base = req->dll_base;
	size_t dll_size = req->size;
	char target_process[40] = {};
	if (strcpy_s(target_process, sizeof(target_process), req->target))
		return STATUS_UNSUCCESSFUL;

	PVOID dll_in_memory = ExAllocatePool2(POOL_FLAG_PAGED, dll_size, 'enoN');
	if (!dll_in_memory) { KeUnstackDetachProcess(&apc); return STATUS_UNSUCCESSFUL; }

	memcpy(dll_in_memory, (PVOID)dll_base, dll_size); // copy our memory into the buffer
	
	req->op = operation_t::op_finish;
	KeUnstackDetachProcess(&apc);
	PEPROCESS target = {};
	if (!NT_SUCCESS(FindProcess(target_process, &target)))
		fail("find_process failed \n");

	PVOID target_base = PsGetProcessSectionBaseAddress(target);
	if (!target_base)
		fail("PsGetProcessSectionBaseAddress failed");

	CR3 target_cr3 = { DirbaseFromBaseAddress(target_base) };
	if (!target_cr3.Value)
		fail("DirbaseFromBaseAddress failed");

	PVOID allocated_base = AllocateMemory(target_cr3, dll_size, dll_in_memory);
	if (!allocated_base)
		fail("MmAllocateMemory failed");
	log("Allocated memory at: %p", allocated_base);
	
	// Use PspCreateProcessNotifyRoutine to intercept target process creation. Then change thread context to dll.

	ExFreePool(dll_in_memory);
	return STATUS_SUCCESS;
}