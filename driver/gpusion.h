/*
 * gpusion.h — GPUsion Virtual GPU Driver
 * Core types, constants, and function declarations
 *
 * GPUsion creates a WDDM-compliant virtual display adapter that presents
 * itself to Windows as a real GPU. AI inference calls are intercepted
 * and routed to an optimised CPU inference engine.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#ifndef GPUSION_H
#define GPUSION_H

#ifdef _WIN32
  #include <ntddk.h>
  #include <wdm.h>
  #include <dispmprt.h>
  #include <d3dkmddi.h>
  #include <d3dkmthk.h>
#else
  /* Linux cross-compile stubs for syntax checking and CI */
  #include "compat/win_stubs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ─── Version ──────────────────────────────────────────────────────────── */

#define GPUSION_DRIVER_MAJOR        1
#define GPUSION_DRIVER_MINOR        0
#define GPUSION_DRIVER_PATCH        0
#define GPUSION_DRIVER_VERSION      ((GPUSION_DRIVER_MAJOR << 16) | \
                                     (GPUSION_DRIVER_MINOR << 8)  | \
                                      GPUSION_DRIVER_PATCH)
#define GPUSION_DRIVER_VERSION_STR  "1.0.0"

/* ─── Virtual Adapter Identity ─────────────────────────────────────────── */

/*
 * Vendor ID 0x6750 = 'Gp' in ASCII — not assigned to any real vendor.
 * We use a clearly fictional ID to avoid conflicts with real hardware.
 */
#define GPUSION_VENDOR_ID           0x6750
#define GPUSION_DEVICE_ID           0x0001
#define GPUSION_SUBSYSTEM_ID        0x00000001
#define GPUSION_REVISION_ID         0x01

#define GPUSION_ADAPTER_NAME        L"GPUsion Virtual Adapter"
#define GPUSION_ADAPTER_NAME_A      "GPUsion Virtual Adapter"
#define GPUSION_DRIVER_NAME         L"GPUsion"

/* ─── Virtual VRAM Configuration ───────────────────────────────────────── */

/*
 * We report 8GB of VRAM to the OS regardless of actual system RAM.
 * Actual allocations are proxied into system RAM with intelligent
 * management. See driver/vram/vram_proxy.c for details.
 */
#define GPUSION_VRAM_SIZE_MB        8192ULL
#define GPUSION_VRAM_SIZE_BYTES     (GPUSION_VRAM_SIZE_MB * 1024ULL * 1024ULL)
#define GPUSION_SHARED_MEM_BYTES    (GPUSION_VRAM_SIZE_BYTES)

/* Minimum system RAM to allow driver load */
#define GPUSION_MIN_SYSTEM_RAM_MB   2048ULL

/* ─── Backend Selection ─────────────────────────────────────────────────── */

typedef enum _GPUSION_BACKEND {
    GPUSION_BACKEND_CPU  = 0,   /* Phase 1: CPU inference via llama.cpp/ONNX */
    GPUSION_BACKEND_FPGA = 1,   /* Phase 2: FPGA dongle via USB              */
    GPUSION_BACKEND_ASIC = 2,   /* Phase 3: Custom GPUsion chip              */
} GPUSION_BACKEND;

/* ─── Device Context ────────────────────────────────────────────────────── */

/*
 * GPUSION_DEVICE_CONTEXT
 * Per-device state maintained by the driver.
 * Allocated at DxgkDdiAddDevice, freed at DxgkDdiRemoveDevice.
 */
typedef struct _GPUSION_DEVICE_CONTEXT {

    /* Signature for debug validation */
    ULONG                   Signature;
#define GPUSION_DEVICE_SIGNATURE    0x47505553U

    /* DXGK adapter handle — provided by DXGK at AddDevice */
    HANDLE                  DxgkHandle;

    /* Physical device object */
    PDEVICE_OBJECT          PhysicalDeviceObject;

    /* Backend currently in use */
    GPUSION_BACKEND         Backend;

    /* VRAM proxy state */
    struct {
        SIZE_T              TotalBytes;       /* Always GPUSION_VRAM_SIZE_BYTES  */
        SIZE_T              AllocatedBytes;   /* Currently allocated             */
        KSPIN_LOCK          Lock;             /* Protects allocation state       */
    } Vram;

    /* Adapter capabilities reported to DXGK */
    struct {
        UINT32              MaxTextureWidth;
        UINT32              MaxTextureHeight;
        UINT32              MaxTextureDepth;
        BOOLEAN             SupportsDirectML;
        BOOLEAN             SupportsVulkan;
        BOOLEAN             SupportsOpenCL;
    } Caps;

    /* Statistics (for diagnostics and benchmarking) */
    struct {
        LONGLONG            InferenceRequests;
        LONGLONG            InferenceCompleted;
        LONGLONG            InferenceFailed;
        LONGLONG            BytesAllocated;
        LONGLONG            BytesFreed;
    } Stats;

} GPUSION_DEVICE_CONTEXT, *PGPUSION_DEVICE_CONTEXT;

/* Retrieve device context from DXGK handle */
#define GPUSION_GET_CONTEXT(handle) \
    ((PGPUSION_DEVICE_CONTEXT)DxgkCbGetDeviceInfo(handle))

/* ─── Allocation Context ────────────────────────────────────────────────── */

/*
 * GPUSION_ALLOCATION
 * Represents a single VRAM allocation (proxied to system RAM).
 * Created per DxgkDdiCreateAllocation call.
 */
typedef struct _GPUSION_ALLOCATION {
    ULONG                   Signature;
#define GPUSION_ALLOC_SIGNATURE     0x4750414CU

    SIZE_T                  SizeBytes;
    PVOID                   SystemMemory;       /* Actual backing RAM pointer */
    PHYSICAL_ADDRESS        VirtualGpuAddress;  /* Fake GPU address reported  */
    BOOLEAN                 IsCpuVisible;
    BOOLEAN                 IsShared;
} GPUSION_ALLOCATION, *PGPUSION_ALLOCATION;

/* ─── Function Declarations ─────────────────────────────────────────────── */

/* driver/kmdf/driver_entry.c */
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
void GpusionDriverUnload(PVOID DriverObject);

/* driver/wddm/add_device.c */
NTSTATUS GpusionAddDevice(
    _In_ HANDLE             DxgkHandle,
    _In_ PDEVICE_OBJECT     PhysicalDeviceObject,
    _Out_ PVOID*            ppMiniportDeviceContext
);

/* driver/wddm/start_device.c */
NTSTATUS GpusionStartDevice(
    _In_ PVOID              pMiniportDeviceContext,
    _In_ PDXGK_START_INFO   DxgkStartInfo,
    _In_ PDXGKRNL_INTERFACE DxgkInterface,
    _Out_ PULONG            NumberOfVideoPresentSources,
    _Out_ PULONG            NumberOfChildren
);

/* driver/wddm/stop_device.c */
NTSTATUS GpusionStopDevice(
    _In_ PVOID              pMiniportDeviceContext
);

/* driver/wddm/remove_device.c */
NTSTATUS GpusionRemoveDevice(
    _In_ PVOID              pMiniportDeviceContext
);

/* driver/wddm/query_adapter_info.c */
NTSTATUS GpusionQueryAdapterInfo(
    _In_ PVOID                      pMiniportDeviceContext,
    _In_ const DXGKARG_QUERYADAPTERINFO* pQueryAdapterInfo
);

/* driver/dxgi/dxgi_enum.c */
NTSTATUS GpusionQueryChildRelations(
    _In_  PVOID                     pMiniportDeviceContext,
    _Out_ PDXGK_CHILD_DESCRIPTOR    ChildRelations,
    _In_  ULONG                     ChildRelationsSize
);

NTSTATUS GpusionQueryChildStatus(
    _In_    PVOID                   pMiniportDeviceContext,
    _Inout_ PDXGK_CHILD_STATUS      ChildStatus,
    _In_    BOOLEAN                 NonDestructiveOnly
);

/* driver/vram/vram_proxy.c */
NTSTATUS GpusionCreateAllocation(
    _In_    PVOID                       pMiniportDeviceContext,
    _Inout_ DXGKARG_CREATEALLOCATION*   pCreateAllocation
);

NTSTATUS GpusionDestroyAllocation(
    _In_ PVOID                          pMiniportDeviceContext,
    _In_ const DXGKARG_DESTROYALLOCATION* pDestroyAllocation
);

/* driver/wddm/submit_command.c */
NTSTATUS GpusionSubmitCommand(
    _In_ PVOID                          pMiniportDeviceContext,
    _In_ const DXGKARG_SUBMITCOMMAND*   pSubmitCommand
);

/* Utility */
GPUSION_BACKEND GpusionDetectBackend(VOID);

VOID GpusionLogEvent(
    _In_ PGPUSION_DEVICE_CONTEXT    Context,
    _In_ ULONG                      EventId,
    _In_ PCSTR                      Message
);

#ifdef __cplusplus
}
#endif

#endif /* GPUSION_H */
