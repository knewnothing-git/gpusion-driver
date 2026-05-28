/*
 * vram_proxy.c — GPUsion VRAM Proxy
 *
 * GPU applications allocate VRAM for model weights and activations.
 * GPUsion proxies these allocations into system RAM transparently.
 *
 * The application believes it is allocating GPU-local VRAM.
 * We allocate system RAM and return a fake GPU address.
 * The inference engine accesses the real system RAM pointer directly.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── GpusionCreateAllocation ───────────────────────────────────────────── */

/*
 * GpusionCreateAllocation
 *
 * Called when an application or runtime allocates GPU memory.
 * We allocate real system RAM and record the mapping.
 *
 * DXGK calls this with an array of allocation descriptors.
 * Each descriptor specifies size, alignment, and usage flags.
 * We fill in the private driver data for each allocation.
 */
NTSTATUS
GpusionCreateAllocation(
    _In_    PVOID                       pMiniportDeviceContext,
    _Inout_ DXGKARG_CREATEALLOCATION*   pCreateAllocation
)
{
    PGPUSION_DEVICE_CONTEXT Context =
        (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;
    KIRQL OldIrql; (void)OldIrql;
    ULONG                   i;

    if (!Context || Context->Signature != GPUSION_DEVICE_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!pCreateAllocation || !pCreateAllocation->pAllocationInfo) {
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < pCreateAllocation->NumAllocations; i++) {

        DXGK_ALLOCATIONINFO*    AllocInfo = &pCreateAllocation->pAllocationInfo[i];
        PGPUSION_ALLOCATION     Alloc     = NULL;
        SIZE_T                  SizeBytes = AllocInfo->Size;

        /* Allocate our private tracking structure */
        Alloc = (PGPUSION_ALLOCATION)ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(GPUSION_ALLOCATION),
            GPUSION_ALLOC_SIGNATURE
        );

        if (!Alloc) {
            KdPrint(("GPUsion: CreateAllocation — out of memory for tracking struct\n"));
            return STATUS_NO_MEMORY;
        }

        RtlZeroMemory(Alloc, sizeof(GPUSION_ALLOCATION));
        Alloc->Signature  = GPUSION_ALLOC_SIGNATURE;
        Alloc->SizeBytes  = SizeBytes;

        /*
         * Allocate backing system RAM.
         *
         * We use ExAllocatePoolWithTag for small allocations (< 1MB)
         * and MmAllocateContiguousMemory for large ones (model weights).
         *
         * Model weight tensors are typically 100MB–8GB.
         * For these we need contiguous physical memory for efficient
         * DMA in Phase 2 (FPGA backend).
         */
        if (SizeBytes <= (1 * 1024 * 1024)) {
            /* Small allocation — paged pool is fine */
            Alloc->SystemMemory = ExAllocatePoolWithTag(
                PagedPool,
                SizeBytes,
                GPUSION_ALLOC_SIGNATURE
            );
        } else {
            /* Large allocation — non-paged for Phase 2 DMA compatibility */
            Alloc->SystemMemory = ExAllocatePoolWithTag(
                NonPagedPool,
                SizeBytes,
                GPUSION_ALLOC_SIGNATURE
            );
        }

        if (!Alloc->SystemMemory) {
            KdPrint(("GPUsion: CreateAllocation — failed to allocate (unsigned long long) bytes\n",
                     (ULONGLONG)SizeBytes));
            ExFreePoolWithTag(Alloc, GPUSION_ALLOC_SIGNATURE);
            return STATUS_NO_MEMORY;
        }

        /* Zero the memory — GPU drivers always zero-init allocations */
        RtlZeroMemory(Alloc->SystemMemory, SizeBytes);

        /*
         * Assign a fake GPU virtual address.
         * We use the system memory pointer cast as a GPU address.
         * This works because our "GPU" is the CPU — same address space.
         */
        Alloc->VirtualGpuAddress.QuadPart = (LONGLONG)(ULONG_PTR)Alloc->SystemMemory;
        Alloc->IsCpuVisible               = TRUE;
        Alloc->IsShared                   = (pCreateAllocation->Flags.SharedResource != 0);

        /* Store our allocation pointer in the DXGK allocation info */
        AllocInfo->pDriverData      = Alloc;
        AllocInfo->DriverDataSize   = sizeof(GPUSION_ALLOCATION);

        /* Update VRAM accounting */
        KeAcquireSpinLock(&Context->Vram.Lock, &OldIrql);
        Context->Vram.AllocatedBytes += SizeBytes;
        InterlockedAdd64(&Context->Stats.BytesAllocated, (LONGLONG)SizeBytes);
        KeReleaseSpinLock(&Context->Vram.Lock, OldIrql);

        KdPrint(("GPUsion: Allocated (unsigned long long) bytes at %p (GPU addr 0x%llX)\n",
                 (ULONGLONG)SizeBytes,
                 Alloc->SystemMemory,
                 Alloc->VirtualGpuAddress.QuadPart));
    }

    return STATUS_SUCCESS;
}

/* ─── GpusionDestroyAllocation ──────────────────────────────────────────── */

/*
 * GpusionDestroyAllocation
 *
 * Called when the application frees GPU memory.
 * We free the backing system RAM and the tracking structure.
 */
NTSTATUS
GpusionDestroyAllocation(
    _In_ PVOID                              pMiniportDeviceContext,
    _In_ const DXGKARG_DESTROYALLOCATION*   pDestroyAllocation
)
{
    PGPUSION_DEVICE_CONTEXT Context =
        (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;
    KIRQL OldIrql; (void)OldIrql;
    ULONG                   i;

    if (!Context || Context->Signature != GPUSION_DEVICE_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!pDestroyAllocation || !pDestroyAllocation->pAllocationList) {
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < pDestroyAllocation->NumAllocations; i++) {

        HANDLE              AllocHandle = pDestroyAllocation->pAllocationList[i];
        PGPUSION_ALLOCATION Alloc       = (PGPUSION_ALLOCATION)AllocHandle;

        if (!Alloc) {
            KdPrint(("GPUsion: DestroyAllocation — NULL handle at index %u\n", i));
            continue;
        }

        if (Alloc->Signature != GPUSION_ALLOC_SIGNATURE) {
            KdPrint(("GPUsion: DestroyAllocation — bad signature 0x%08X\n",
                     Alloc->Signature));
            continue;
        }

        /* Update VRAM accounting */
        KeAcquireSpinLock(&Context->Vram.Lock, &OldIrql);
        Context->Vram.AllocatedBytes -= Alloc->SizeBytes;
        InterlockedAdd64(&Context->Stats.BytesFreed, (LONGLONG)Alloc->SizeBytes);
        KeReleaseSpinLock(&Context->Vram.Lock, OldIrql);

        KdPrint(("GPUsion: Freeing (unsigned long long) bytes at %p\n",
                 (ULONGLONG)Alloc->SizeBytes,
                 Alloc->SystemMemory));

        /* Free the backing memory */
        if (Alloc->SystemMemory) {
            ExFreePoolWithTag(Alloc->SystemMemory, GPUSION_ALLOC_SIGNATURE);
            Alloc->SystemMemory = NULL;
        }

        /* Poison and free the tracking structure */
        Alloc->Signature = 0xDEADDEAD;
        ExFreePoolWithTag(Alloc, GPUSION_ALLOC_SIGNATURE);
    }

    return STATUS_SUCCESS;
}
