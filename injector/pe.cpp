#include "pe.h"


NTSTATUS resolve_relocations(UINT64 file_base) {
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)file_base;
	PIMAGE_NT_HEADERS64 nt_header = (PIMAGE_NT_HEADERS64)(file_base + dos_hdr->e_lfanew);

	if (!nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
		return STATUS_UNSUCCESSFUL;

	UINT64 delta = (UINT64)(file_base - nt_header->OptionalHeader.ImageBase);
	if (!delta)
		return STATUS_UNSUCCESSFUL;

	PIMAGE_BASE_RELOCATION relocation_data = (PIMAGE_BASE_RELOCATION)(file_base + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
	PIMAGE_BASE_RELOCATION relocation_data_end = (PIMAGE_BASE_RELOCATION)( (PUCHAR)relocation_data + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);
	while (relocation_data < relocation_data_end && relocation_data->SizeOfBlock){
		UINT32 num_of_entries = (relocation_data->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(UINT16);
		PRELOCATION_ENTRY relative_info = (PRELOCATION_ENTRY)(relocation_data + 1); // past IMAGE_BASE_RELOCATION is where the next struct is, for some reason it's not part of image_base_relocation (due to alignment?)
		for (UINT32 j = 0; j < num_of_entries; ++j) {
			if (RELOC_FLAG64(relative_info->Type)) {
				PUINT64 patch = (PUINT64)(file_base + relocation_data->VirtualAddress + relative_info->Offset); 
				*patch += (UINT64)(delta);
			}
		}
		relocation_data = (PIMAGE_BASE_RELOCATION)((PUCHAR)relocation_data + relocation_data->SizeOfBlock);
	}
	return STATUS_SUCCESS;

}

NTSTATUS resolve_imports(UINT64 file_base) {
	PIMAGE_DOS_HEADER dos_hdr = (PIMAGE_DOS_HEADER)file_base;
	PIMAGE_NT_HEADERS64 nt_header = (PIMAGE_NT_HEADERS64)(file_base + dos_hdr->e_lfanew);

	if (!nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
		return STATUS_UNSUCCESSFUL;

	PIMAGE_IMPORT_DESCRIPTOR import_data = (PIMAGE_IMPORT_DESCRIPTOR)(file_base + nt_header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	log("import_data: %p", import_data);

	PIMAGE_THUNK_DATA thunk = nullptr;
	for(; import_data->Name != 0; import_data++){
		thunk = (PIMAGE_THUNK_DATA)(file_base + import_data->OriginalFirstThunk);
		log("thunk: %p", thunk);
		for (; thunk->u1.AddressOfData != 0; thunk++) {
			
			PIMAGE_IMPORT_BY_NAME nom_de_guerre = (PIMAGE_IMPORT_BY_NAME)(file_base + (PUCHAR)thunk->u1.AddressOfData);
			log("name address: %s", nom_de_guerre->Name);
		}
	}

	return STATUS_SUCCESS;

}