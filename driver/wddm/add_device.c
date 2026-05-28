/*
 * add_device.c — GPUsion Device Context Lifecycle
 *
 * DxgkDdiAddDevice    — Called when PnP enumerates our virtual device.
 *                       Allocate and initialise our device context.
 *
 * DxgkDdiRemoveDevice — Called when the device is removed.
 *                       Free all resources.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── GpusionAddDevice ──────────────────────────────────────────────────── */

/*
 * GpusionAddDevice
 *
 * DXGK calls this when our device is enumerated by Plug and Play.
 * We allocate our GPUSION_DEVICE_CONTEXT and return a pointer to it.
 * DXGK will pass this pointer back to us in every subsequent DDI call.
 *
 * This is the foundation of the illusion — from this moment, Windows
 * tracks our virtual adapter as a real device in the system.
 */
NTSTATUS
GpusionAddDevice(
    _In_  HANDLE            DxgkHandle,
    _In_  PDEVICE_OBJECT    PhysicalDeviceObject,
    _Out_ PVOID*            ppMiniportDeviceContext
)
{
    PGPUSION_DEVICE_CONTEXT Context = NULL;
    NTSTATUS                Status;
    SYSTEM_BASIC_INFORMATION SysInfo;

    KdPrint(("GPUsion: AddDevice called\n"));

    /* Validate parameters */
    if (!DxgkHandle || !PhysicalDeviceObject || !ppMiniportDeviceContext) {
        return STATUS_INVALID_PARAMETER;
    }

    *ppMiniportDeviceContext = NULL;

    /*
     * Check system RAM meets our minimum.
     * A machine with < 2GB RAM cannot run any useful AI model.
     * Fail gracefully rather than loading and performing poorly.
     */
    Status = ZwQuerySystemInformation(
        SystemBasicInformation,
        &SysInfo,
        sizeof(SysInfo),
        NULL
    );

    if (NT_SUCCESS(Status)) {
        ULONGLONG SystemRamMb = ((ULONGLONG)SysInfo.NumberOfPhysicalPages
                                 * SysInfo.PageSize) / (1024 * 1024);

        if (SystemRamMb < GPUSION_MIN_SYSTEM_RAM_MB) {
            KdPrint(("GPUsion: Insufficient system RAM ((unsigned long long) MB, need (unsigned long long) MB)\n",
                     SystemRamMb, GPUSION_MIN_SYSTEM_RAM_MB));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KdPrint(("GPUsion: System RAM: (unsigned long long) MB — OK\n", SystemRamMb));
    }

    /*
     * Allocate the device context from non-paged pool.
     * Non-paged pool is required for kernel objects that may be accessed
     * at DISPATCH_LEVEL (during interrupt handling).
     */
    Context = (PGPUSION_DEVICE_CONTEXT)ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(GPUSION_DEVICE_CONTEXT),
        GPUSION_DEVICE_SIGNATURE
    );

    if (!Context) {
        KdPrint(("GPUsion: Failed to allocate device context\n"));
        return STATUS_NO_MEMORY;
    }

    /* Zero-initialise the context */
    RtlZeroMemory(Context, sizeof(GPUSION_DEVICE_CONTEXT));

    /* Set the signature for debug validation */
    Context->Signature              = GPUSION_DEVICE_SIGNATURE;

    /* Store handles */
    Context->DxgkHandle             = DxgkHandle;
    Context->PhysicalDeviceObject   = PhysicalDeviceObject;

    /* Detect which inference backend is available */
    Context->Backend                = GpusionDetectBackend();

    /* Initialise VRAM proxy state */
    Context->Vram.TotalBytes        = GPUSION_VRAM_SIZE_BYTES;
    Context->Vram.AllocatedBytes    = 0;
    KeInitializeSpinLock(&Context->Vram.Lock);

    /* Set adapter capabilities */
    Context->Caps.MaxTextureWidth   = 65536;
    Context->Caps.MaxTextureHeight  = 65536;
    Context->Caps.MaxTextureDepth   = 2048;
    Context->Caps.SupportsDirectML  = TRUE;
    Context->Caps.SupportsVulkan    = FALSE;   /* Phase 2 */
    Context->Caps.SupportsOpenCL    = FALSE;   /* Phase 2 */

    /* Zero statistics */
    Context->Stats.InferenceRequests    = 0;
    Context->Stats.InferenceCompleted   = 0;
    Context->Stats.InferenceFailed      = 0;
    Context->Stats.BytesAllocated       = 0;
    Context->Stats.BytesFreed           = 0;

    /* Return the context pointer to DXGK */
    *ppMiniportDeviceContext = Context;

    KdPrint(("GPUsion: Device context allocated at %p, backend=%d\n",
             Context, Context->Backend));

    return STATUS_SUCCESS;
}

/* ─── GpusionRemoveDevice ───────────────────────────────────────────────── */

/*
 * GpusionRemoveDevice
 *
 * Called when the device is removed or the driver is unloaded.
 * Free the device context and all associated resources.
 *
 * At this point, DXGK guarantees no other DDI calls are in flight.
 */
NTSTATUS
GpusionRemoveDevice(
    _In_ PVOID pMiniportDeviceContext
)
{
    PGPUSION_DEVICE_CONTEXT Context = (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;

    if (!Context) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Validate signature */
    ASSERT(Context->Signature == GPUSION_DEVICE_SIGNATURE);

    KdPrint(("GPUsion: RemoveDevice — stats: requests=(long long) completed=(long long) failed=(long long)\n",
             Context->Stats.InferenceRequests,
             Context->Stats.InferenceCompleted,
             Context->Stats.InferenceFailed));

    /* Warn if VRAM proxy has leaked allocations */
    if (Context->Vram.AllocatedBytes > 0) {
        KdPrint(("GPUsion: WARNING — (unsigned long long) bytes still allocated at RemoveDevice\n",
                 (ULONGLONG)Context->Vram.AllocatedBytes));
    }

    /* Poison the signature to catch use-after-free */
    Context->Signature = 0xDEADDEAD;

    /* Free the context */
    ExFreePoolWithTag(Context, GPUSION_DEVICE_SIGNATURE);

    KdPrint(("GPUsion: Device context freed\n"));

    return STATUS_SUCCESS;
}
