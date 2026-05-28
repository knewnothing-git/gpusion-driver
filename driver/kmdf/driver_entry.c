/*
 * driver_entry.c — GPUsion Driver Entry Point
 *
 * DriverEntry is the first function called when Windows loads the driver.
 * We register our WDDM DDI (Device Driver Interface) function table here,
 * telling the DXGK subsystem which of our functions to call for each
 * GPU operation.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── Forward Declarations ──────────────────────────────────────────────── */

static NTSTATUS GpusionBuildDdiTable(
    _Out_ DRIVER_INITIALIZATION_DATA* DdiTable
);

/* ─── DriverEntry ───────────────────────────────────────────────────────── */

/*
 * DriverEntry
 *
 * Windows calls this when the driver is first loaded.
 * We call DxgkInitialize() with our DDI table — this hands control
 * of GPU operations over to the DXGK subsystem which calls our
 * registered functions as needed.
 */
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{
    NTSTATUS                        Status;
    DRIVER_INITIALIZATION_DATA      DdiTable;

    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("GPUsion: DriverEntry — version %s\n", GPUSION_DRIVER_VERSION_STR));

    /* Zero the DDI table — any unset function pointers must be NULL */
    RtlZeroMemory(&DdiTable, sizeof(DdiTable));

    /* Fill in our DDI function pointers */
    Status = GpusionBuildDdiTable(&DdiTable);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("GPUsion: Failed to build DDI table: 0x%08X\n", Status));
        return Status;
    }

    /*
     * Hand off to DXGK. From this point, DXGK owns device lifecycle.
     * It will call GpusionAddDevice when our hardware is enumerated by PnP.
     *
     * Note: On success, DXGK sets DriverObject->DriverUnload for us.
     * We do not set it ourselves.
     */
    Status = DxgkInitialize(DriverObject, RegistryPath, &DdiTable);
    if (!NT_SUCCESS(Status)) {
        KdPrint(("GPUsion: DxgkInitialize failed: 0x%08X\n", Status));
        return Status;
    }

    KdPrint(("GPUsion: Driver loaded successfully\n"));
    return STATUS_SUCCESS;
}

/* ─── DDI Table Construction ────────────────────────────────────────────── */

/*
 * GpusionBuildDdiTable
 *
 * Fills the DRIVER_INITIALIZATION_DATA structure with our function pointers.
 *
 * WDDM requires us to implement a minimum set of DDIs. Functions we do not
 * implement must be NULL — DXGK handles the missing-function cases for
 * display-less compute adapters.
 *
 * For a virtual compute adapter (no display output), the required DDIs are:
 *   - AddDevice / RemoveDevice
 *   - StartDevice / StopDevice
 *   - QueryAdapterInfo
 *   - CreateAllocation / DestroyAllocation
 *   - SubmitCommand
 *   - QueryChildRelations / QueryChildStatus
 *
 * Display DDIs (Present, SetVidPnSourceAddress, etc.) are NULL because
 * GPUsion is a compute-only adapter with no display output.
 */
static NTSTATUS
GpusionBuildDdiTable(
    _Out_ DRIVER_INITIALIZATION_DATA* DdiTable
)
{
    /*
     * Set the WDDM version we support.
     * DXGKDDI_INTERFACE_VERSION_WDDM2_7 = Windows 10 2004 minimum.
     * This covers all Windows 10/11 machines in our target market.
     */
    DdiTable->Version = DXGKDDI_INTERFACE_VERSION_WDDM2_7;

    /* ── Device Lifecycle ─────────────────────────────────────────────── */

    DdiTable->DxgkDdiAddDevice              = GpusionAddDevice;
    DdiTable->DxgkDdiStartDevice            = GpusionStartDevice;
    DdiTable->DxgkDdiStopDevice             = GpusionStopDevice;
    DdiTable->DxgkDdiRemoveDevice           = GpusionRemoveDevice;

    /* ── Adapter Capabilities ─────────────────────────────────────────── */

    DdiTable->DxgkDdiQueryAdapterInfo       = GpusionQueryAdapterInfo;
    DdiTable->DxgkDdiQueryChildRelations    = GpusionQueryChildRelations;
    DdiTable->DxgkDdiQueryChildStatus       = GpusionQueryChildStatus;

    /* ── Memory Allocation ────────────────────────────────────────────── */

    DdiTable->DxgkDdiCreateAllocation       = GpusionCreateAllocation;
    DdiTable->DxgkDdiDestroyAllocation      = GpusionDestroyAllocation;

    /* ── Command Submission ───────────────────────────────────────────── */

    DdiTable->DxgkDdiSubmitCommand          = GpusionSubmitCommand;

    /*
     * Display DDIs — NULL for compute-only adapter.
     * DXGK accepts NULL here for adapters without display output.
     */
    DdiTable->DxgkDdiSetPowerState          = NULL;
    DdiTable->DxgkDdiPresent                = NULL;
    DdiTable->DxgkDdiRender                 = NULL;
    DdiTable->DxgkDdiPatch                  = NULL;

    KdPrint(("GPUsion: DDI table built, WDDM version 0x%08X\n",
             DdiTable->Version));

    return STATUS_SUCCESS;
}
