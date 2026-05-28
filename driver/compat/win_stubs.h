/*
 * win_stubs.h — Windows API Stubs for Linux CI
 *
 * Allows the driver C code to be syntax-checked on Linux
 * (GitHub Actions CI) without the Windows Driver Kit.
 *
 * These stubs define just enough of the Windows types and macros
 * for the compiler to parse our driver code. They do not implement
 * any real functionality — they exist only for CI syntax validation.
 *
 * Real builds use the Windows Driver Kit on Windows.
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#ifndef WIN_STUBS_H
#define WIN_STUBS_H

#ifdef _WIN32
  #error "win_stubs.h must only be included on non-Windows platforms"
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ─── Basic Windows types ───────────────────────────────────────────────── */

typedef uint8_t     UINT8,  BYTE,   BOOLEAN;
typedef uint16_t    UINT16, USHORT, WCHAR;
typedef uint32_t    UINT32, ULONG,  DWORD;
typedef uint64_t    UINT64;
typedef unsigned long long ULONGLONG;
typedef int8_t      INT8;
typedef int16_t     INT16;
typedef int32_t     INT32,  LONG,   NTSTATUS;
typedef int64_t     INT64;
typedef long long LONGLONG;
typedef size_t      SIZE_T;
typedef size_t      ULONG_PTR;
typedef void        VOID,  *PVOID,  *HANDLE;
typedef char        CHAR,  *PSTR,  *PCSTR;
typedef WCHAR      *PWSTR, *PCWSTR;
typedef ULONG      *PULONG;
typedef LONGLONG   *PLONGLONG;

typedef struct { LONGLONG QuadPart; } LARGE_INTEGER, PHYSICAL_ADDRESS;
typedef LARGE_INTEGER *PLARGE_INTEGER;

#define TRUE    1
#define FALSE   0
#ifndef NULL
#define NULL    ((void*)0)
#endif

/* CONST — Windows uses this as a qualifier in DDI signatures */
#define CONST   const

/* ─── Windows calling conventions (no-op on Linux) ─────────────────────── */

#define NTAPI
#define WINAPI
#define CALLBACK
#define __cdecl
#define __stdcall
#define FORCEINLINE     __attribute__((always_inline)) inline
#define UNREFERENCED_PARAMETER(p)   ((void)(p))

/* ─── SAL annotations (no-op on Linux) ─────────────────────────────────── */

#define _In_
#define _Out_
#define _Inout_
#define _In_opt_
#define _Out_opt_
#define _Inout_opt_
#define _In_reads_(x)
#define _Out_writes_(x)
#define _In_reads_bytes_(x)
#define _Out_writes_bytes_(x)
#define _Success_(x)
#define _Must_inspect_result_

/* ─── NTSTATUS values ───────────────────────────────────────────────────── */

#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#define STATUS_INVALID_PARAMETER        ((NTSTATUS)0xC000000DL)
#define STATUS_NO_MEMORY                ((NTSTATUS)0xC0000017L)
#define STATUS_INSUFFICIENT_RESOURCES   ((NTSTATUS)0xC000009AL)
#define STATUS_NOT_SUPPORTED            ((NTSTATUS)0xC00000BBL)
#define STATUS_BUFFER_TOO_SMALL         ((NTSTATUS)0xC0000023L)
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)

#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)

/* ─── Pool allocation stubs ─────────────────────────────────────────────── */

typedef enum { NonPagedPool = 0, PagedPool = 1 } POOL_TYPE;

#define ExAllocatePoolWithTag(pool, size, tag) \
    (calloc(1, (size_t)(size)))

#define ExFreePoolWithTag(ptr, tag) \
    free((void*)(ptr))

/* ─── Memory operation stubs ────────────────────────────────────────────── */

#define RtlZeroMemory(dest, len)        memset((dest), 0, (len))
#define RtlCopyMemory(dest, src, len)   memcpy((dest), (src), (len))

/* ─── Spinlock stubs ────────────────────────────────────────────────────── */

typedef uint32_t KSPIN_LOCK, *PKSPIN_LOCK;
typedef uint8_t  KIRQL;

#define KeInitializeSpinLock(lock)                  (*(lock) = 0)
#define KeAcquireSpinLock(lock, irql)               ((void)0)
#define KeReleaseSpinLock(lock, irql)               ((void)0)

/* ─── Interlocked operations ────────────────────────────────────────────── */

#define InterlockedIncrement64(ptr)     __sync_fetch_and_add((ptr), 1)
#define InterlockedAdd64(ptr, val)      __sync_fetch_and_add((ptr), (val))

/* ─── Debug print stub ──────────────────────────────────────────────────── */

#define KdPrint(args)   printf args

/* ─── Assert stub ───────────────────────────────────────────────────────── */

#include <assert.h>
#define ASSERT(x)   assert(x)

/* ─── Device object stub ────────────────────────────────────────────────── */

typedef struct _DEVICE_OBJECT { ULONG Reserved; } DEVICE_OBJECT, *PDEVICE_OBJECT;
typedef struct _DRIVER_OBJECT { ULONG Reserved; } DRIVER_OBJECT, *PDRIVER_OBJECT;
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

/* ─── WDDM / DXGK stubs ─────────────────────────────────────────────────── */

#define DXGKDDI_INTERFACE_VERSION_WDDM2_7   0x700A

typedef PVOID   HANDLE_DXGK;

typedef struct {
    LARGE_INTEGER   HighestAcceptableAddress;
    ULONG           MaxAllocationListSlotId;
    ULONGLONG       ApertureSegmentCommitLimit;
    ULONG           MaxPointerWidth;
    ULONG           MaxPointerHeight;
    struct { ULONG Value; } SchedulingCaps;
    struct { ULONG Value; } MemoryManagementCaps;
} DXGK_DRIVERCAPS;
typedef struct {
    LARGE_INTEGER   BaseAddress;
    LARGE_INTEGER   CpuTranslatedAddress;
    SIZE_T          Size;
    SIZE_T          CommitLimit;
    struct {
        UINT32      CpuVisible  : 1;
        UINT32      UseBanking  : 1;
        UINT32      Aperture    : 1;
        UINT32      Reserved    : 29;
    } Flags;
} DXGK_SEGMENTDESCRIPTOR;
typedef struct { ULONG NbSegment; DXGK_SEGMENTDESCRIPTOR* pSegmentDescriptor; ULONG PagingBufferSize; ULONG PagingBufferPrivateDataSize; } DXGK_QUERYSEGMENTOUT;

typedef enum {
    TypeUninitialized = 0,
    TypeVideoOutput   = 1,
} DXGK_CHILD_DEVICE_TYPE;

typedef enum {
    StatusConnection = 0,
} DXGK_CHILD_STATUS_TYPE;

typedef struct {
    DXGK_CHILD_DEVICE_TYPE ChildDeviceType;
    ULONG Reserved[8];
} DXGK_CHILD_DESCRIPTOR, *PDXGK_CHILD_DESCRIPTOR;

typedef struct {
    DXGK_CHILD_STATUS_TYPE Type;
    struct { BOOLEAN Connected; } HotPlug;
} DXGK_CHILD_STATUS, *PDXGK_CHILD_STATUS;

typedef struct {
    struct {
        UINT32 SupportSurpriseRemoval : 1;
        UINT32 GpuMmuSupported        : 1;
        UINT32 IoMmuSupported         : 1;
        UINT32 Reserved               : 29;
    } Flags;
} DXGK_PHYSICALADAPTERCAPS;
typedef struct { ULONG Flags; } DXGK_START_INFO, *PDXGK_START_INFO;
typedef struct { ULONG Reserved; } DXGKRNL_INTERFACE, *PDXGKRNL_INTERFACE;

typedef enum {
    DXGKQAITYPE_DRIVERCAPS = 0,
    DXGKQAITYPE_QUERYSEGMENT,
    DXGKQAITYPE_QUERYSEGMENT3,
    DXGKQAITYPE_QUERYSEGMENT4,
    DXGKQAITYPE_UMDRIVERPRIVATE,
    DXGKQAITYPE_PHYSICALADAPTERCAPS,
    DXGKQAITYPE_DISPLAY_DRIVERCAPS_EXTENSION,
    DXGKQAITYPE_INTEGRATED_DISPLAY_DESCRIPTOR,
    DXGKQAITYPE_INTEGRATED_DISPLAY_DESCRIPTOR2,
} DXGK_QUERYADAPTERINFOTYPE;

typedef struct {
    DXGK_QUERYADAPTERINFOTYPE   Type;
    PVOID                       pInputData;
    UINT32                      InputDataSize;
    PVOID                       pOutputData;
    UINT32                      OutputDataSize;
} DXGKARG_QUERYADAPTERINFO, *CONST PDXGKARG_QUERYADAPTERINFO;

typedef struct {
    SIZE_T      Size;
    PVOID       pDriverData;
    ULONG       DriverDataSize;
} DXGK_ALLOCATIONINFO;

typedef struct {
    ULONG                   NumAllocations;
    DXGK_ALLOCATIONINFO    *pAllocationInfo;
    struct { ULONG SharedResource; } Flags;
} DXGKARG_CREATEALLOCATION, *PDXGKARG_CREATEALLOCATION;

typedef struct {
    ULONG   NumAllocations;
    HANDLE *pAllocationList;
} DXGKARG_DESTROYALLOCATION, *CONST PDXGKARG_DESTROYALLOCATION;

typedef struct {
    PHYSICAL_ADDRESS    DmaBufferPhysicalAddress;
    ULONG               DmaBufferSize;
} DXGKARG_SUBMITCOMMAND, *CONST PDXGKARG_SUBMITCOMMAND;

typedef struct {
    ULONG Version;
    PVOID DxgkDdiAddDevice;
    PVOID DxgkDdiStartDevice;
    PVOID DxgkDdiStopDevice;
    PVOID DxgkDdiRemoveDevice;
    PVOID DxgkDdiQueryAdapterInfo;
    PVOID DxgkDdiQueryChildRelations;
    PVOID DxgkDdiQueryChildStatus;
    PVOID DxgkDdiCreateAllocation;
    PVOID DxgkDdiDestroyAllocation;
    PVOID DxgkDdiSubmitCommand;
    PVOID DxgkDdiSetPowerState;
    PVOID DxgkDdiPresent;
    PVOID DxgkDdiRender;
    PVOID DxgkDdiPatch;
} DRIVER_INITIALIZATION_DATA, *PDRIVER_INITIALIZATION_DATA;

typedef struct {
    ULONG   NumberOfPhysicalPages;
    ULONG   PageSize;
} SYSTEM_BASIC_INFORMATION;

#define SystemBasicInformation  0

/* Stubs for functions we call */
static inline NTSTATUS DxgkInitialize(PVOID a, PVOID b, PVOID c) {
    (void)a; (void)b; (void)c; return STATUS_SUCCESS;
}
static inline PVOID DxgkCbGetDeviceInfo(PVOID h) { return h; }
static inline NTSTATUS ZwQuerySystemInformation(ULONG a, PVOID b, ULONG c, PVOID d) {
    SYSTEM_BASIC_INFORMATION* s = (SYSTEM_BASIC_INFORMATION*)b;
    (void)a; (void)c; (void)d;
    s->NumberOfPhysicalPages = 2097152; /* 8GB */
    s->PageSize = 4096;
    return STATUS_SUCCESS;
}

/* PAGE_SIZE */
#define PAGE_SIZE   4096UL

/* Driver macros */
#ifndef _WIN32
#define DRIVER_INITIALIZE   NTSTATUS
#define DRIVER_UNLOAD       VOID
#endif

#endif /* WIN_STUBS_H */
