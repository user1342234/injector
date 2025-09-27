#pragma once
#ifndef NT_H
#define NT_H

#define MM_COPY_MEMORY_PHYSICAL             0x1
#define MM_COPY_MEMORY_VIRTUAL              0x2
#define PAGE_SIZE 0x1000
#include <wdm.h>
typedef UINT64 QWORD;
typedef UINT16 WORD;
typedef UINT32 DWORD;
#include "memory.h"
#include "structs.h"



//0x30 bytes (sizeof)
typedef struct _KAPC_STATE
{
    struct _LIST_ENTRY ApcListHead[2];                                      //0x0
    struct _KPROCESS* Process;                                              //0x20
    union
    {
        UCHAR InProgressFlags;                                              //0x28
        struct
        {
            UCHAR KernelApcInProgress : 1;                                    //0x28
            UCHAR SpecialApcInProgress : 1;                                   //0x28
        };
    };
    UCHAR KernelApcPending;                                                 //0x29
    union
    {
        UCHAR UserApcPendingAll;                                            //0x2a
        struct
        {
            UCHAR SpecialUserApcPending : 1;                                  //0x2a
            UCHAR UserApcPending : 1;                                         //0x2a
        };
    };
}KAPC_STATE, *PKAPC_STATE;
typedef struct _MM_COPY_ADDRESS {
    union {
        PVOID            VirtualAddress;
        PHYSICAL_ADDRESS PhysicalAddress;
    };
} MM_COPY_ADDRESS, * PMMCOPY_ADDRESS;
typedef struct _PHYSICAL_MEMORY_RANGE {
    PHYSICAL_ADDRESS BaseAddress;
    LARGE_INTEGER NumberOfBytes;
} PHYSICAL_MEMORY_RANGE, * PPHYSICAL_MEMORY_RANGE;



EXTERN_C NTKERNELAPI void KeStackAttachProcess(
    IN PEPROCESS  PROCESS,
    IN PKAPC_STATE ApcState
);

EXTERN_C NTKERNELAPI void KeUnstackDetachProcess(
    IN PKAPC_STATE ApcState
);

EXTERN_C NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(
    IN HANDLE    ProcessId,
    OUT PEPROCESS* Process
);

EXTERN_C NTKERNELAPI NTSTATUS MmCopyMemory(
      IN PVOID           TargetAddress,
      IN MM_COPY_ADDRESS SourceAddress,
      IN SIZE_T          NumberOfBytes,
      IN ULONG           Flags,
      OUT PSIZE_T         NumberOfBytesTransferred
);


EXTERN_C NTKERNELAPI ULONG KeCapturePersistentThreadState(
    IN PCONTEXT Context,
    IN PKTHREAD Thread,
    IN ULONG BugCheckCode,
    IN ULONG BugCheckParameter1,
    IN ULONG BugCheckParameter2,
    IN ULONG BugCheckParameter3,
    IN ULONG BugCheckParameter4,
    OUT PVOID VirtualAddress
);

EXTERN_C NTKERNELAPI VOID RtlCaptureContext(
    OUT PCONTEXT ContextRecord
);


EXTERN_C NTKERNELAPI VOID KeBugCheck(
    IN ULONG BugCheckCode
);

EXTERN_C NTKERNELAPI PPHYSICAL_MEMORY_RANGE MmGetPhysicalMemoryRanges();

EXTERN_C NTKERNELAPI PVOID  MmGetVirtualForPhysical(
    IN PHYSICAL_ADDRESS PhysicalAddress
);

EXTERN_C NTKERNELAPI PVOID RtlPcToFileHeader(
    IN  PVOID PcValue,
    OUT PVOID* BaseOfImage
);

EXTERN_C NTKERNELAPI PEPROCESS PsInitialSystemProcess;

EXTERN_C NTKERNELAPI  PVOID PsGetProcessSectionBaseAddress( 
    IN PEPROCESS Process
);

EXTERN_C NTKERNELAPI PHYSICAL_ADDRESS MmGetPhysicalAddress(
    IN PVOID BaseAddress
);

EXTERN_C NTKERNELAPI BOOLEAN MmIsAddressValid(
    IN PVOID VirtualAddress
);
// Non-System Functions

NTSTATUS FindProcess(IN CHAR* process_name, OUT PEPROCESS* process);

#endif