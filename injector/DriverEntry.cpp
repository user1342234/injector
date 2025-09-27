#include "nt.h"
#include "communication.h"
#include "logging.h"
#include "debug.h"
NTSTATUS start();

EXTERN_C NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT  DriverObject,
	_In_ PUNICODE_STRING RegistryPath
) {
	UNREFERENCED_PARAMETER(RegistryPath);
	UNREFERENCED_PARAMETER(DriverObject);
	
	KIRQL irql = KeGetCurrentIrql();
	if (irql > APC_LEVEL)
		KeLowerIrql(PASSIVE_LEVEL);

	HANDLE hThread;

	if (!NT_SUCCESS(PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, 0, 0, 0, (PKSTART_ROUTINE)&start, nullptr)))
		fail("PsCreateSystemThread failed\n");

	KIRQL result{};
	KeRaiseIrql(irql, &result);
	return STATUS_SUCCESS;
}