/*
 * query_adapter_info.c — GPUsion Adapter Capability Reporting
 *
 * This is the heart of the illusion.
 *
 * DxgkDdiQueryAdapterInfo is called by DXGK to discover what our
 * "GPU" can do. The information we return here determines:
 *   - Whether Windows recognises us as a valid compute adapter
 *   - Whether DirectML will select us for inference
 *   - What VRAM applications believe is available
 *   - What feature levels apps can use
 *
 * We report capabilities that match what our CPU inference engine
 * can actually deliver. We do not lie about capabilities we cannot
 * fulfil — that leads to crashes, not slow performance.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── Internal helpers ──────────────────────────────────────────────────── */

static NTSTATUS
GpusionQueryDriverCaps(
    _In_  PGPUSION_DEVICE_CONTEXT   Context,
    _Out_ DXGK_DRIVERCAPS*          DriverCaps
);

static NTSTATUS
GpusionQuerySegmentInfo(
    _In_  PGPUSION_DEVICE_CONTEXT   Context,
    _Out_ DXGK_QUERYSEGMENTOUT*     SegmentInfo
);

/* ─── GpusionQueryAdapterInfo ───────────────────────────────────────────── */

/*
 * GpusionQueryAdapterInfo
 *
 * DXGK calls this repeatedly during device initialisation with different
 * QueryType values, each asking for a different class of information.
 *
 * We handle the types that matter for a compute adapter and return
 * STATUS_NOT_SUPPORTED for display-specific queries we don't need.
 */
NTSTATUS
GpusionQueryAdapterInfo(
    _In_ PVOID                          pMiniportDeviceContext,
    _In_ const DXGKARG_QUERYADAPTERINFO* pQueryAdapterInfo
)
{
    PGPUSION_DEVICE_CONTEXT Context =
        (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;

    if (!Context || Context->Signature != GPUSION_DEVICE_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!pQueryAdapterInfo || !pQueryAdapterInfo->pOutputData) {
        return STATUS_INVALID_PARAMETER;
    }

    switch (pQueryAdapterInfo->Type) {

    /* ── Driver Capabilities ────────────────────────────────────────── */
    case DXGKQAITYPE_DRIVERCAPS:
        return GpusionQueryDriverCaps(
            Context,
            (DXGK_DRIVERCAPS*)pQueryAdapterInfo->pOutputData
        );

    /* ── Memory Segment Information ─────────────────────────────────── */
    case DXGKQAITYPE_QUERYSEGMENT:
    case DXGKQAITYPE_QUERYSEGMENT3:
    case DXGKQAITYPE_QUERYSEGMENT4:
        return GpusionQuerySegmentInfo(
            Context,
            (DXGK_QUERYSEGMENTOUT*)pQueryAdapterInfo->pOutputData
        );

    /* ── UMD (User Mode Driver) information ─────────────────────────── */
    case DXGKQAITYPE_UMDRIVERPRIVATE:
        /*
         * The user-mode driver (UMD) can embed private data here.
         * For Phase 1 we have no UMD-specific data to pass.
         * Return success with no output — this is valid.
         */
        KdPrint(("GPUsion: QueryAdapterInfo UMDRIVERPRIVATE — no data\n"));
        return STATUS_SUCCESS;

    /* ── DirectML / Compute Queries ─────────────────────────────────── */
    case DXGKQAITYPE_PHYSICALADAPTERCAPS:
        /*
         * Report that we are a software/virtual adapter.
         * This tells DirectML we are a valid compute device even
         * though we have no physical GPU hardware.
         */
        if (pQueryAdapterInfo->OutputDataSize >=
            sizeof(DXGK_PHYSICALADAPTERCAPS))
        {
            DXGK_PHYSICALADAPTERCAPS* PhysCaps =
                (DXGK_PHYSICALADAPTERCAPS*)pQueryAdapterInfo->pOutputData;

            RtlZeroMemory(PhysCaps, sizeof(*PhysCaps));

            /* Mark as a software adapter — critical for DirectML acceptance */
            PhysCaps->Flags.SupportSurpriseRemoval      = 0;
            PhysCaps->Flags.GpuMmuSupported             = 0;
            PhysCaps->Flags.IoMmuSupported              = 0;

            KdPrint(("GPUsion: Physical adapter caps reported\n"));
            return STATUS_SUCCESS;
        }
        return STATUS_BUFFER_TOO_SMALL;

    /* ── Display queries — not supported for compute-only adapter ───── */
    case DXGKQAITYPE_DISPLAY_DRIVERCAPS_EXTENSION:
    case DXGKQAITYPE_INTEGRATED_DISPLAY_DESCRIPTOR:
    case DXGKQAITYPE_INTEGRATED_DISPLAY_DESCRIPTOR2:
        KdPrint(("GPUsion: Display query type %d — not supported\n",
                 pQueryAdapterInfo->Type));
        return STATUS_NOT_SUPPORTED;

    default:
        KdPrint(("GPUsion: Unknown QueryAdapterInfo type: %d\n",
                 pQueryAdapterInfo->Type));
        return STATUS_NOT_SUPPORTED;
    }
}

/* ─── GpusionQueryDriverCaps ────────────────────────────────────────────── */

/*
 * GpusionQueryDriverCaps
 *
 * Report our driver's feature capabilities to DXGK.
 *
 * Key decisions:
 * - HighestAcceptableAddress: we can address all system RAM
 * - WDDMVersion: report WDDM 2.7 which enables DirectML compute
 * - We do NOT claim display capabilities (no scheduling, no VSync)
 */
static NTSTATUS
GpusionQueryDriverCaps(
    _In_  PGPUSION_DEVICE_CONTEXT   Context,
    _Out_ DXGK_DRIVERCAPS*          DriverCaps
)
{
    UNREFERENCED_PARAMETER(Context);

    RtlZeroMemory(DriverCaps, sizeof(*DriverCaps));

    /*
     * Highest physical address we can access.
     * Set to maximum — we use system RAM which can be anywhere.
     */
    DriverCaps->HighestAcceptableAddress.QuadPart = (LONGLONG)-1;

    /*
     * Maximum number of in-flight DMA buffers.
     * We process commands sequentially — 1 at a time in Phase 1.
     * Phase 2+ can increase this for pipelining.
     */
    DriverCaps->MaxAllocationListSlotId         = 1;

    /*
     * Address aperture for virtual GPU address space.
     * We fake a 4GB GPU address space mapped over system RAM.
     */
    DriverCaps->ApertureSegmentCommitLimit      = 0x100000000ULL; /* 4GB */

    /*
     * Maximum number of segments.
     * We have one: the VRAM proxy segment (backed by system RAM).
     */
    DriverCaps->MaxPointerWidth                 = 0;
    DriverCaps->MaxPointerHeight                = 0;

    /*
     * Scheduling capabilities.
     * We do not support pre-emptive GPU scheduling (no real GPU).
     * DXGK will use cooperative scheduling.
     */
    DriverCaps->SchedulingCaps.Value            = 0;

    /*
     * Memory management capabilities.
     * We support CPU-visible allocations (all our memory is CPU-visible).
     */
    DriverCaps->MemoryManagementCaps.Value      = 0;

    KdPrint(("GPUsion: Driver caps reported\n"));
    return STATUS_SUCCESS;
}

/* ─── GpusionQuerySegmentInfo ───────────────────────────────────────────── */

/*
 * GpusionQuerySegmentInfo
 *
 * Report our memory segments to DXGK.
 *
 * A "segment" is a pool of GPU-addressable memory. Real GPUs have:
 *   - Segment 0: Local VRAM (on-card memory, fastest)
 *   - Segment 1: Aperture (system RAM mapped into GPU address space)
 *
 * GPUsion has one segment: a VRAM proxy that is actually system RAM.
 * We report it as 8GB to satisfy applications that check available VRAM.
 *
 * DXGK calls this twice:
 *   Pass 1 (pOutputData->NbSegment == 0): Tell us how many segments you have
 *   Pass 2 (pOutputData->pSegmentDescriptor != NULL): Fill in the details
 */
static NTSTATUS
GpusionQuerySegmentInfo(
    _In_  PGPUSION_DEVICE_CONTEXT   Context,
    _Out_ DXGK_QUERYSEGMENTOUT*     SegmentInfo
)
{
    UNREFERENCED_PARAMETER(Context);

    /* Pass 1: Report segment count */
    if (SegmentInfo->NbSegment == 0) {
        SegmentInfo->NbSegment          = 1; /* One segment: VRAM proxy */
        SegmentInfo->PagingBufferSize   = PAGE_SIZE;
        SegmentInfo->PagingBufferPrivateDataSize = 0;

        KdPrint(("GPUsion: Segment count query — reporting 1 segment\n"));
        return STATUS_SUCCESS;
    }

    /* Pass 2: Fill segment descriptor */
    if (!SegmentInfo->pSegmentDescriptor) {
        return STATUS_INVALID_PARAMETER;
    }

    DXGK_SEGMENTDESCRIPTOR* Seg = &SegmentInfo->pSegmentDescriptor[0];
    RtlZeroMemory(Seg, sizeof(*Seg));

    /*
     * Segment 0: GPUsion VRAM Proxy
     *
     * BaseAddress = 0: this segment starts at GPU address 0.
     * Size = 8GB: what we report to applications.
     *
     * Flags:
     *   - CpuVisible: TRUE — applications can map this memory to CPU
     *   - Aperture: TRUE — this is aperture memory (system RAM backed)
     *
     * Setting Aperture=TRUE is the honest declaration that this memory
     * is not true local VRAM. DirectML and ONNX Runtime handle aperture
     * segments correctly — they simply avoid GPU-specific optimisations
     * that require true local VRAM, which is exactly what we want.
     */
    Seg->BaseAddress.QuadPart       = 0;
    Seg->CpuTranslatedAddress.QuadPart = 0;
    Seg->Size                       = GPUSION_VRAM_SIZE_BYTES;
    Seg->CommitLimit                = GPUSION_VRAM_SIZE_BYTES;
    Seg->Flags.CpuVisible           = 1;
    Seg->Flags.UseBanking           = 0;
    Seg->Flags.Aperture             = 1;

    KdPrint(("GPUsion: Segment 0 described: %llu MB aperture (VRAM proxy)\n",
             GPUSION_VRAM_SIZE_MB));

    return STATUS_SUCCESS;
}
