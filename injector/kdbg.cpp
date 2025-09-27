#include "kdbg.h"

// credit: Darthton 
KDDEBUGGER_DATA64 g_KdBlock = {};
NTSTATUS InitializeDebuggerDataBlock()
{
    CONTEXT context = { 0 };
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    PDUMP_HEADER dumpHeader = PDUMP_HEADER(ExAllocatePool2(POOL_FLAG_PAGED, DUMP_BLOCK_SIZE, 'enoN'));
    if (dumpHeader)
    {
        KeCapturePersistentThreadState(&context, NULL, 0, 0, 0, 0, 0, dumpHeader);
        RtlCopyMemory(&g_KdBlock, (PUCHAR)dumpHeader + KDDEBUGGER_DATA_OFFSET, sizeof(g_KdBlock));
        ExFreePool(dumpHeader);
        return STATUS_SUCCESS;
    }
    return STATUS_UNSUCCESSFUL;
}
