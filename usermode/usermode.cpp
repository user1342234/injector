// usermode.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "nt.h"
#include "status.h"
#include "communication.h"
int main(int argc, const char* argv[])
{
    if (argc < 2)
        fail("invalid arguments");


    PVOID shared_page = VirtualAlloc(nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (FAIL(shared_page))
        fail("Failed to allocate memory");
    
    log("Shared page: %llx", (UINT64)shared_page);

    UINT32 process_id = GetCurrentProcessId();

    log("Process id: %d", process_id);

    HKEY our_key = {};
    DWORD lpdwDisposition = {};

    if (!NT_SUCCESS(RegCreateKeyExA(HKEY_LOCAL_MACHINE, "Software\\INFORMATION", 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &our_key, &lpdwDisposition)))
        fail("RegCreateKeyExA failed");

    if (lpdwDisposition == REG_OPENED_EXISTING_KEY) {
        if (!NT_SUCCESS(RegDeleteKeyExA(HKEY_LOCAL_MACHINE, "Software\\INFORMATION", KEY_WOW64_64KEY, 0)))
        fail("RegCreateKeyExA failed");

        if (!NT_SUCCESS(RegCreateKeyExA(HKEY_LOCAL_MACHINE, "Software\\INFORMATION", 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL, &our_key, &lpdwDisposition)))
            fail("RegCreateKeyExA failed");
    }

    if (!NT_SUCCESS(RegSetKeyValueA(HKEY_LOCAL_MACHINE, "Software\\INFORMATION", "PROCESSID", REG_DWORD, &process_id, sizeof(DWORD))))
        fail("RegSetKeyValueA failed");

    if (!NT_SUCCESS(RegSetKeyValueA(HKEY_LOCAL_MACHINE, "Software\\INFORMATION", "SHAREDPAGE", REG_QWORD, &shared_page, sizeof(QWORD))))
        fail("RegSetKeyValueA failed");

    // allocate memory for DLL
   

    const char* dll_name = argv[1];
     const char* process_name = argv[2];
    if (FAIL(dll_name) || FAIL(process_name))
        return -1;

    FILE* f;
    errno_t err = fopen_s(&f, dll_name, "r+");
    if (err)
        return -1;


    fseek(f, 0, SEEK_END);
    UINT64 fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    PVOID dll_base = VirtualAlloc(nullptr, fsize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (FAIL(dll_base))
        return -1;
    fread(dll_base, fsize, 1, f);
    fclose(f);

    // write the request,
    volatile PREQUEST req = (PREQUEST)shared_page;
    req->dll_base = (UINT64)dll_base;
    req->size = fsize;
    if (strcpy_s(req->target, sizeof(req->target), process_name))
        return -1;
    req->op = operation_t::op_initialized;

    log("Waiting...");
    while (req->op != operation_t::op_finish) {};

    // Newly allocated memory
    UINT64 dll = req->dll_base;
    log("Dll @ 0x%llx", dll);
    

    if (FAIL(VirtualFree(dll_base, 0, MEM_RELEASE)))
        return -1;
    if (FAIL(VirtualFree(shared_page, 0, MEM_RELEASE)))
        return -1;


    return 0;
}

