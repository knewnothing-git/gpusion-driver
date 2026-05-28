/*
 * start_device.c — GPUsion Device Start / Stop
 *
 * DxgkDdiStartDevice — Called after AddDevice when the device is ready.
 *                      Report capabilities, initialise inference backend.
 *
 * DxgkDdiStopDevice  — Called before RemoveDevice.
 *                      Gracefully shut down the inference backend.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── GpusionStartDevice ────────────────────────────────────────────────── */

/*
 * GpusionStartDevice
 *
 * DXGK calls this after AddDevice when PnP has assigned resources
 * to the device and it is ready to operate.
 *
 * For a virtual adapter there are no real hardware resources to
 * initialise — no IRQs, no memory-mapped IO, no DMA channels.
 * Our job here is to:
 *   1. Store the DXGK interface callbacks we will need later
 *   2. Tell DXGK how many video present sources we have (zero — no display)
 *   3. Tell DXGK how many child devices we have (zero — no monitor output)
 *   4. Initialise the inference backend
 */
NTSTATUS
GpusionStartDevice(
    _In_  PVOID             pMiniportDeviceContext,
    _In_  PDXGK_START_INFO  DxgkStartInfo,
    _In_  PDXGKRNL_INTERFACE DxgkInterface,
    _Out_ PULONG            NumberOfVideoPresentSources,
    _Out_ PULONG            NumberOfChildren
)
{
    PGPUSION_DEVICE_CONTEXT Context = (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;
    NTSTATUS                Status  = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(DxgkStartInfo);
    UNREFERENCED_PARAMETER(DxgkInterface);

    if (!Context || Context->Signature != GPUSION_DEVICE_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    KdPrint(("GPUsion: StartDevice\n"));

    /*
     * GPUsion is a compute-only adapter — no display output.
     *
     * NumberOfVideoPresentSources = 0: no screens to drive.
     * NumberOfChildren = 0: no child devices (monitors, etc.)
     *
     * This is valid for WDDM compute adapters. DirectML and ONNX
     * Runtime will still enumerate and use us for inference.
     */
    *NumberOfVideoPresentSources = 0;
    *NumberOfChildren            = 0;

    /*
     * TODO (Phase 2): If GPUSION_BACKEND_FPGA, initialise USB connection
     * to the hardware dongle here. Verify firmware version compatibility.
     */
    if (Context->Backend == GPUSION_BACKEND_FPGA) {
        KdPrint(("GPUsion: FPGA backend detected — USB init placeholder\n"));
        /* GpusionFpgaInit(Context); */
    }

    KdPrint(("GPUsion: Device started successfully, backend=%d\n",
             Context->Backend));

    return Status;
}

/* ─── GpusionStopDevice ─────────────────────────────────────────────────── */

/*
 * GpusionStopDevice
 *
 * DXGK calls this before RemoveDevice.
 * We must stop all active inference operations and release hardware resources.
 * After this returns, no more DDI calls will arrive (except RemoveDevice).
 */
NTSTATUS
GpusionStopDevice(
    _In_ PVOID pMiniportDeviceContext
)
{
    PGPUSION_DEVICE_CONTEXT Context = (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;

    if (!Context || Context->Signature != GPUSION_DEVICE_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    KdPrint(("GPUsion: StopDevice\n"));

    /*
     * TODO (Phase 2): If FPGA backend, send shutdown command to dongle
     * and close USB connection gracefully.
     */
    if (Context->Backend == GPUSION_BACKEND_FPGA) {
        KdPrint(("GPUsion: FPGA backend — USB shutdown placeholder\n"));
        /* GpusionFpgaShutdown(Context); */
    }

    KdPrint(("GPUsion: Device stopped\n"));

    return STATUS_SUCCESS;
}
