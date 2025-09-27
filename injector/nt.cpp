#include "nt.h"

NTSTATUS FindProcess(IN CHAR* process_name, OUT PEPROCESS* process)
{
	WINDOWS::PEPROCESS sys_process = (WINDOWS::PEPROCESS)PsInitialSystemProcess;
	WINDOWS::PEPROCESS cur_entry = sys_process;
	do
	{
		if (strstr((CHAR*)cur_entry->ImageFileName, process_name))
		{
				*process = (PEPROCESS)cur_entry;
				return STATUS_SUCCESS;
			
		}
		WINDOWS::PLIST_ENTRY list = &cur_entry->ActiveProcessLinks; /*EPROCESS->ActiveProcessLinks*/;
		cur_entry = (WINDOWS::PEPROCESS)((PUCHAR)list->Flink - UFIELD_OFFSET(WINDOWS::EPROCESS, ActiveProcessLinks));
	} while (cur_entry != sys_process);

	return STATUS_UNSUCCESSFUL;
}
