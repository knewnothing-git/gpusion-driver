/*
 * dxgi_enum.c — GPUsion DXGI Child Device Enumeration
 *
 * DXGK calls QueryChildRelations to discover what child devices
 * our adapter has (monitors, connectors, etc).
 *
 * GPUsion is a compute-only adapter — no display output, no monitors.
 * We report zero children. This is valid for compute adapters and
 * tells Windows not to expect any display from us.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── GpusionQueryChildRelations ────────────────────────────────────────── */

/*
 * GpusionQueryChildRelations
 *
 * DXGK calls this to enumerate child devices attached to our adapter.
 * For a compute-only virtual adapter, there are no children.
 *
 * The ChildRelationsSize tells us how large the output buffer is.
 * DXGK always allocates space for (NumberOfChildren + 1) descriptors,
 * where the last one is a sentinel with ChildDeviceType = TypeUninitialized.
 *
 * Since we reported NumberOfChildren = 0 in StartDevice, DXGK allocates
 * space for exactly 1 sentinel descriptor. We fill it in and return.
 */
NTSTATUS
GpusionQueryChildRelations(
    _In_  PVOID                     pMiniportDeviceContext,
    _Out_ PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    _In_  ULONG                     ChildRelationsSize
)
{
    PGPUSION_DEVICE_CONTEXT Context =
        (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;

    UNREFERENCED_PARAMETER(Context);

    if (!ChildRelations) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ChildRelationsSize < sizeof(DXGK_CHILD_DESCRIPTOR)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    /*
     * Write the sentinel descriptor.
     * ChildDeviceType = TypeUninitialized signals end of list to DXGK.
     * No real child devices — we are compute-only.
     */
    RtlZeroMemory(ChildRelations, sizeof(DXGK_CHILD_DESCRIPTOR));
    ChildRelations[0].ChildDeviceType = TypeUninitialized;

    KdPrint(("GPUsion: QueryChildRelations — compute-only, no children\n"));

    return STATUS_SUCCESS;
}

/* ─── GpusionQueryChildStatus ───────────────────────────────────────────── */

/*
 * GpusionQueryChildStatus
 *
 * DXGK calls this to check whether a child device (e.g. monitor)
 * is currently connected.
 *
 * Since we have no children, this should never be called.
 * If it is called for any reason, return not-connected.
 */
NTSTATUS
GpusionQueryChildStatus(
    _In_    PVOID                   pMiniportDeviceContext,
    _Inout_ PDXGK_CHILD_STATUS      ChildStatus,
    _In_    BOOLEAN                 NonDestructiveOnly
)
{
    UNREFERENCED_PARAMETER(pMiniportDeviceContext);
    UNREFERENCED_PARAMETER(NonDestructiveOnly);

    if (!ChildStatus) {
        return STATUS_INVALID_PARAMETER;
    }

    /*
     * Report no device connected.
     * HotPlug.Connected = FALSE means no monitor attached.
     */
    ChildStatus->Type = StatusConnection;
    ChildStatus->HotPlug.Connected = FALSE;

    KdPrint(("GPUsion: QueryChildStatus — no child device connected\n"));

    return STATUS_SUCCESS;
}
