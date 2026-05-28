/*
 * backend_detect.c — GPUsion Backend Detection
 *
 * Detects which inference backend is available at driver load time.
 * Falls back gracefully: ASIC → FPGA → CPU.
 *
 * Phase 1: Always returns GPUSION_BACKEND_CPU.
 * Phase 2: Scans USB devices for GPUsion FPGA dongle VID/PID.
 * Phase 3: Scans for GPUsion ASIC VID/PID.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/*
 * USB Vendor/Product IDs for GPUsion hardware.
 * These are placeholders — real IDs assigned when hardware is ready.
 * USB-IF VID must be obtained from usb.org before shipping.
 */
#define GPUSION_USB_VID_PLACEHOLDER     0x6750  /* Same as PCI vendor ID */
#define GPUSION_USB_PID_FPGA            0x0001  /* Phase 2 FPGA dongle   */
#define GPUSION_USB_PID_ASIC            0x0002  /* Phase 3 custom chip   */

/* ─── GpusionDetectBackend ──────────────────────────────────────────────── */

/*
 * GpusionDetectBackend
 *
 * Probe for available hardware backends.
 * Returns the best available backend.
 *
 * Called from GpusionAddDevice — runs at PASSIVE_LEVEL.
 */
GPUSION_BACKEND
GpusionDetectBackend(VOID)
{
    /*
     * Phase 2: Scan USB bus for our FPGA dongle.
     *
     * We would use IoGetDeviceInterfaces() with the USB device
     * interface GUID, then check VID/PID of each enumerated device.
     *
     * For Phase 1 this is always CPU. The USB scan is stubbed out
     * but the structure is in place for Phase 2 to fill in.
     */

#ifdef GPUSION_PHASE2_ENABLED
    /* TODO: USB device enumeration for FPGA dongle */
    BOOLEAN FpgaPresent = GpusionScanUsbForDevice(
        GPUSION_USB_VID_PLACEHOLDER,
        GPUSION_USB_PID_FPGA
    );

    if (FpgaPresent) {
        KdPrint(("GPUsion: FPGA dongle detected — using hardware backend\n"));
        return GPUSION_BACKEND_FPGA;
    }
#endif

#ifdef GPUSION_PHASE3_ENABLED
    /* TODO: ASIC detection */
    BOOLEAN AsicPresent = GpusionScanUsbForDevice(
        GPUSION_USB_VID_PLACEHOLDER,
        GPUSION_USB_PID_ASIC
    );

    if (AsicPresent) {
        KdPrint(("GPUsion: GPUsion ASIC detected — using ASIC backend\n"));
        return GPUSION_BACKEND_ASIC;
    }
#endif

    /* Phase 1: CPU inference always available */
    KdPrint(("GPUsion: No hardware dongle detected — using CPU backend\n"));
    return GPUSION_BACKEND_CPU;
}

/* ─── GpusionLogEvent ───────────────────────────────────────────────────── */

/*
 * GpusionLogEvent
 *
 * Write a driver event to the Windows Event Log.
 * Used for errors and important state changes visible in Event Viewer.
 */
VOID
GpusionLogEvent(
    _In_ PGPUSION_DEVICE_CONTEXT    Context,
    _In_ ULONG                      EventId,
    _In_ PCSTR                      Message
)
{
    UNREFERENCED_PARAMETER(Context);

    /* KdPrint goes to kernel debugger — always available */
    KdPrint(("GPUsion Event [%u]: %s\n", EventId, Message));

    /*
     * TODO: IoWriteErrorLogEntry for production Event Log entries.
     * This makes errors visible in Windows Event Viewer without
     * requiring a kernel debugger — important for user support.
     */
}
