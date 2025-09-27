
#include "memory.h"
UINT64 VirtualToPhysical(IN const UINT64 virtualAddress)
{
	return MmGetPhysicalAddress(reinterpret_cast<PVOID>(virtualAddress)).QuadPart;
}

UINT64 PhysicalToVirtual(IN const UINT64 physicalAddress)
{
	PHYSICAL_ADDRESS physical;
	physical.QuadPart = physicalAddress;
	return (UINT64)(MmGetVirtualForPhysical(physical));
}


NTSTATUS ReadPhysicalAddress(IN UINT64 TargetAddress, IN PVOID lpBuffer, IN SIZE_T Size, OUT SIZE_T* BytesRead)
{
	MM_COPY_ADDRESS AddrToRead = { 0 };
	AddrToRead.PhysicalAddress.QuadPart = TargetAddress;
	return MmCopyMemory(lpBuffer, AddrToRead, Size, MM_COPY_MEMORY_PHYSICAL, BytesRead);
}


NTSTATUS WritePhysicalPage(IN UINT64 physical_address, IN PVOID lpBuffer, IN SIZE_T Size) {
	UINT32 byte_offset = physical_address & 0xFFF;
	if (Size + byte_offset > PAGE_SIZE)
		return STATUS_UNSUCCESSFUL;
	UINT64 pfn = PAGE_TO_PFN( (physical_address - byte_offset ));
	PUCHAR base = (PUCHAR)MmAllocateIndependentPagesEx(PAGE_SIZE, -1, 0 ,0);
	if (!base)
		return STATUS_UNSUCCESSFUL;
	PMMPTE pte = MiGetPteAddress(base);
	UINT32 old_pfn = pte->u.Hard.PageFrameNumber;
	pte->u.Hard.PageFrameNumber = pfn;
	memcpy(base + byte_offset, lpBuffer, Size);
	pte->u.Hard.PageFrameNumber = old_pfn;
	MmFreeIndependentPages(base, PAGE_SIZE);
	return STATUS_SUCCESS;
}

UINT64 DirbaseFromBaseAddress( IN PVOID base) { // credit: 0Dev

	VIRTUAL_ADDRESS virtual_base = {};
	virtual_base.Value = (UINT64)base;
	
	size_t read{};

	auto ranges = MmGetPhysicalMemoryRanges();

	for (int i = 0;; i++) {

		auto elem = &ranges[i];

		if (!elem->BaseAddress.QuadPart || !elem->NumberOfBytes.QuadPart)
			break;

		UINT64 current_phys_address = elem->BaseAddress.QuadPart;

		for (int j = 0; j < (elem->NumberOfBytes.QuadPart / PAGE_SIZE); j++, current_phys_address += PAGE_SIZE) {
			
			// I think we have to dereference mmpfndatabase to get it.
			_MMPFN* pfninfo = (_MMPFN*)((	*(PUINT64)ACCESS(MmPfnDatabase) + PAGE_TO_PFN(current_phys_address) * sizeof(_MMPFN)));

			//Found self referencing pml4e
			if (pfninfo->u4.PteFrame == PAGE_TO_PFN(current_phys_address))
			{
				MMPTE pml4e{};
				if (!NT_SUCCESS(ReadPhysicalAddress(current_phys_address + 8 * virtual_base.Pml4Index, &pml4e, 8, &read)))
					continue;
				if (!pml4e.u.Hard.Valid)
					continue;
				MMPTE pdpte{};
				if (!NT_SUCCESS(ReadPhysicalAddress((pml4e.u.Hard.PageFrameNumber << 12) + 8 * virtual_base.PdptIndex, &pdpte, 8, &read)))
					continue;
				if (!pdpte.u.Hard.Valid)
					continue;
				MMPTE pde{};
				if (!NT_SUCCESS(ReadPhysicalAddress((pdpte.u.Hard.PageFrameNumber << 12) + 8 * virtual_base.PdIndex, &pde, 8, &read)))
					continue;
				if (!pde.u.Hard.Valid)
					continue;
				MMPTE pte{};
				if (!NT_SUCCESS(ReadPhysicalAddress((pde.u.Hard.PageFrameNumber << 12) + 8 * virtual_base.PtIndex, &pte, 8, &read)))
					continue;
				if (!pte.u.Hard.Valid)
					continue;
				log("Found dirbase at %llx\n", current_phys_address);
				return current_phys_address;
			}

		}

	}

	return 0;
}

PMMPTE MiGetPteAddress(IN PVOID VirtualAddress)
{
	return (PMMPTE)(g_KdBlock.PteBase + (((ULONG_PTR)VirtualAddress >> 9) & 0x7FFFFFFFF8));
}




__drv_allocatesMem(Mem)
_Check_return_
_Ret_maybenull_
_IRQL_requires_max_(APC_LEVEL)
_Post_writable_byte_size_(NumberOfBytes)
PVOID
AllocateMemory(IN CR3 cr3, IN SIZE_T NumberOfBytes, OPTIONAL IN PVOID lpBuffer) {
	VIRTUAL_ADDRESS usermode_address = {};
	PMMPTE PteBuffer[MAX_DLL_PAGES];
	SIZE_T allocated_pages = 0;
	SIZE_T needed_pages = BYTES_TO_PAGES(NumberOfBytes);

	if (needed_pages > MAX_DLL_PAGES)
		fail("needed_pages exceeded buffer size in MmAllocateMemory");

	PUCHAR PageBuffer = (PUCHAR)ExAllocatePool2(POOL_FLAG_PAGED, PAGE_SIZE*PAGE_TABLE_HIERARCHY_MAX, 'enoN');
	if (!PageBuffer)
		return NULL;

	RtlZeroMemory(PageBuffer, PAGE_SIZE);

	UINT64 read = 0;

	if (!NT_SUCCESS(ReadPhysicalAddress(PFN_TO_PAGE(cr3.Pml4), PageBuffer + (PML4_HIERARCHY * PAGE_SIZE), PAGE_SIZE, &read))) { // Read PML4 physical!
		ExFreePool(PageBuffer);
		return NULL;
	}

	PMMPTE pml4 = (PMMPTE)(PageBuffer + (PML4_HIERARCHY * PAGE_SIZE));
	const PMMPTE pml4e = pml4;
	
	for (UINT32 pml4_index = 0; pml4_index < 512; pml4_index++) {
		if (!pml4e[pml4_index].u.Hard.Valid)
			continue;

		if (!NT_SUCCESS(ReadPhysicalAddress(PFN_TO_PAGE(pml4e[pml4_index].u.Hard.PageFrameNumber), PageBuffer + (PDPT_HIERARCHY * PAGE_SIZE), PAGE_SIZE, &read))) {
			ExFreePool(PageBuffer);
			return NULL;
		}

		PMMPTE pdpt = (PMMPTE)(PageBuffer + (PDPT_HIERARCHY * PAGE_SIZE));;
		const PMMPTE pdpte = pdpt;

		if (!MmIsAddressValid((PVOID)pdpte))
			continue;

		for (UINT32 pdpt_index = 0; pdpt_index < 512; pdpt_index++) {
			if (!pdpte[pdpt_index].u.Hard.Valid)
				continue;


			if (!NT_SUCCESS(ReadPhysicalAddress(PFN_TO_PAGE(pdpte[pdpt_index].u.Hard.PageFrameNumber), PageBuffer+(PD_HIERARCHY*PAGE_SIZE), PAGE_SIZE, &read))){
				ExFreePool(PageBuffer);
				return NULL;
			}
			PMMPTE pd = (PMMPTE)(PageBuffer + (PD_HIERARCHY * PAGE_SIZE));
			const PMMPTE pde = pd;
			if (!MmIsAddressValid((PVOID)pde))
				continue;

			for (UINT32 pde_index = 0; pde_index < 512; pde_index++) {
				if (!pde[pde_index].u.Hard.Valid)
					continue;
				
				 // We have to Save memory here since we just ReUse a page for Each Hierarchy
				if (!NT_SUCCESS(ReadPhysicalAddress(PFN_TO_PAGE(pde[pde_index].u.Hard.PageFrameNumber), PageBuffer+ (PT_HIERARCHY * PAGE_SIZE), PAGE_SIZE, &read))){
					ExFreePool(PageBuffer);
					return NULL;
				}

				PMMPTE pt = (PMMPTE)(PageBuffer + (PT_HIERARCHY * PAGE_SIZE));
				PMMPTE pte = pt;
				if (!MmIsAddressValid((PVOID)pte))
					continue;
				
				for (UINT32 pte_index = 0; pte_index < 512; pte_index++) {
					if (!pte[pte_index].u.Hard.Valid) { // Find contiguous invalid PTEs
						
						// address is aligned from mmallocateindependentpagesex
						// Find Invalid pte and save it.
						PteBuffer[allocated_pages++] = &pte[pte_index]; // insert invalid pte into our buffer.
						if (allocated_pages == needed_pages) {
							//DEBUG_BREAK;
							usermode_address.Pml4Index = pml4_index;
							usermode_address.PdptIndex = pdpt_index;
							usermode_address.PdIndex = pde_index;
							usermode_address.PtIndex = _abs64(pte_index - allocated_pages);
							usermode_address.Offset = 0;
						

							log("pml4e contains: %llx", PFN_TO_PAGE(pml4e[pml4_index].u.Hard.PageFrameNumber));
							log("pdpt contains: %llx", PFN_TO_PAGE(pdpte[pdpt_index].u.Hard.PageFrameNumber));
							log("pd contains: %llx", PFN_TO_PAGE(pde[pde_index].u.Hard.PageFrameNumber));
							log("pt contains: %llx", PFN_TO_PAGE(pte[pte_index].u.Hard.PageFrameNumber));
							// Perform allocation here!
							
							const PVOID allocation_base = MmAllocateIndependentPagesEx(needed_pages*PAGE_SIZE, -1, 0, 0);
							if (lpBuffer)
								memcpy(allocation_base, lpBuffer, NumberOfBytes); // If allocation was specified, write whatever we need into it.

							RtlZeroMemory(allocation_base, needed_pages*PAGE_SIZE);
							UINT64 current_page = (UINT64)allocation_base;
							
							for (UINT32 i = 0; i < needed_pages; ++i) {
								
								PMMPTE cur_pte = MiGetPteAddress((PVOID)current_page); 
								// Change the PFN in one of our saved ptes
								//DEBUG_BREAK;
								PteBuffer[i]->u.Long = (UINT64)MiMakeValidPte(PteBuffer[i], 0, 0xA0000004);
								PteBuffer[i]->u.Hard.Valid = 1;
								PteBuffer[i]->u.Hard.NoExecute = 0;
								PteBuffer[i]->u.Hard.PageFrameNumber = cur_pte->u.Hard.PageFrameNumber;

								if (!NT_SUCCESS(WritePhysicalPage(PFN_TO_PAGE(pde[pde_index].u.Hard.PageFrameNumber) + (sizeof(MMPTE) * (usermode_address.PtIndex + i)), (PVOID)PteBuffer[i], 8))) { // MmMapIoSpaceEx cannot write page tables.
									ExFreePool(PageBuffer);
									MmFreeIndependentPages(allocation_base, needed_pages*PAGE_SIZE);
									return NULL;
								}
								
								current_page += PAGE_SIZE;
							}
							//DEBUG_BREAK;
							ExFreePool(PageBuffer);
							return usermode_address.Pointer;
						}
							

					}
					else { 
						
						allocated_pages = 0;
					}
						
					

				}

			}

		}

	}
	return NULL;
	

}
