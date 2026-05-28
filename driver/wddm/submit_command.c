/*
 * submit_command.c — GPUsion Command Submission
 *
 * This is where the illusion meets the reality.
 *
 * DxgkDdiSubmitCommand is called when an application submits a
 * GPU workload — a buffer of GPU commands to execute.
 *
 * A real GPU driver would DMA this buffer to the hardware.
 * GPUsion parses the command buffer and routes the work to the
 * CPU inference engine (Phase 1) or hardware dongle (Phase 2+).
 *
 * License: MIT
 * Built in India 🇮🇳
 */

#include "../gpusion.h"

/* ─── Command Buffer Format ─────────────────────────────────────────────── */

/*
 * GPUsion command buffer header.
 *
 * When the user-mode driver (UMD) submits work, it writes a command
 * buffer in our private format. The kernel-mode driver (us) reads
 * this buffer and dispatches accordingly.
 *
 * For Phase 1, we support one command type: INFER.
 * The UMD fills this header, then the inference engine takes over.
 */

#define GPUSION_CMD_MAGIC       0x47505531  /* 'GPU1' */

typedef enum _GPUSION_CMD_TYPE {
    GPUSION_CMD_NOP     = 0x00,   /* No operation — used for testing */
    GPUSION_CMD_INFER   = 0x01,   /* Route to inference engine        */
    GPUSION_CMD_MEMCPY  = 0x02,   /* GPU-to-GPU memory copy           */
    GPUSION_CMD_FENCE   = 0x03,   /* Synchronisation fence            */
} GPUSION_CMD_TYPE;

typedef struct _GPUSION_CMD_HEADER {
    ULONG               Magic;          /* Must be GPUSION_CMD_MAGIC    */
    GPUSION_CMD_TYPE    Type;           /* Command type                 */
    ULONG               SizeBytes;      /* Total command buffer size    */
    ULONG               SequenceId;     /* Monotonic sequence number    */
} GPUSION_CMD_HEADER, *PGPUSION_CMD_HEADER;

typedef struct _GPUSION_CMD_INFER {
    GPUSION_CMD_HEADER  Header;
    ULONGLONG           InputGpuAddr;   /* GPU address of input tensor  */
    ULONGLONG           OutputGpuAddr;  /* GPU address of output tensor */
    ULONG               InputSizeBytes;
    ULONG               OutputSizeBytes;
    ULONG               OperationType; /* maps to ONNX operator type   */
} GPUSION_INFER_CMD, *PGPUSION_INFER_CMD;

/* ─── Forward declarations ──────────────────────────────────────────────── */

static NTSTATUS GpusionDispatchInfer(
    _In_ PGPUSION_DEVICE_CONTEXT    Context,
    _In_ PGPUSION_INFER_CMD Cmd
);

static NTSTATUS GpusionDispatchMemcpy(
    _In_ PGPUSION_DEVICE_CONTEXT    Context,
    _In_ PGPUSION_CMD_HEADER        Cmd
);

/* ─── GpusionSubmitCommand ──────────────────────────────────────────────── */

/*
 * GpusionSubmitCommand
 *
 * DXGK calls this when it has a command buffer ready to submit to the GPU.
 * We parse the command header and dispatch to the appropriate handler.
 *
 * This runs at DISPATCH_LEVEL — no blocking, no paged memory access,
 * no waiting. Heavy inference work is queued to a worker thread.
 */
NTSTATUS
GpusionSubmitCommand(
    _In_ PVOID                          pMiniportDeviceContext,
    _In_ const DXGKARG_SUBMITCOMMAND*   pSubmitCommand
)
{
    PGPUSION_DEVICE_CONTEXT Context =
        (PGPUSION_DEVICE_CONTEXT)pMiniportDeviceContext;
    PGPUSION_CMD_HEADER     CmdHeader = NULL;
    NTSTATUS                Status    = STATUS_SUCCESS;

    if (!Context || Context->Signature != GPUSION_DEVICE_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!pSubmitCommand) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Track submission */
    InterlockedIncrement64(&Context->Stats.InferenceRequests);

    /*
     * The command buffer is in kernel-accessible memory.
     * Validate minimum size before reading the header.
     */
    if (pSubmitCommand->DmaBufferSize < sizeof(GPUSION_CMD_HEADER)) {
        KdPrint(("GPUsion: SubmitCommand — buffer too small: %u bytes\n",
                 pSubmitCommand->DmaBufferSize));
        InterlockedIncrement64(&Context->Stats.InferenceFailed);
        return STATUS_BUFFER_TOO_SMALL;
    }

    CmdHeader = (PGPUSION_CMD_HEADER)(ULONG_PTR)pSubmitCommand->DmaBufferPhysicalAddress.QuadPart;

    /* Validate magic number */
    if (CmdHeader->Magic != GPUSION_CMD_MAGIC) {
        KdPrint(("GPUsion: SubmitCommand — bad magic 0x%08X\n", CmdHeader->Magic));
        InterlockedIncrement64(&Context->Stats.InferenceFailed);
        return STATUS_INVALID_PARAMETER;
    }

    KdPrint(("GPUsion: SubmitCommand type=%d seq=%u size=%u\n",
             CmdHeader->Type, CmdHeader->SequenceId, CmdHeader->SizeBytes));

    /* Dispatch by command type */
    switch (CmdHeader->Type) {

    case GPUSION_CMD_NOP:
        /* No-op — used for driver health checks and benchmarking */
        Status = STATUS_SUCCESS;
        break;

    case GPUSION_CMD_INFER:
        if (pSubmitCommand->DmaBufferSize < sizeof(GPUSION_CMD_INFER)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }
        Status = GpusionDispatchInfer(Context, (PGPUSION_INFER_CMD)CmdHeader);
        break;

    case GPUSION_CMD_MEMCPY:
        Status = GpusionDispatchMemcpy(Context, CmdHeader);
        break;

    case GPUSION_CMD_FENCE:
        /*
         * Fence commands signal synchronisation points.
         * Phase 1: immediately signal completion (single-threaded).
         * Phase 2: wait for FPGA pipeline to drain.
         */
        Status = STATUS_SUCCESS;
        break;

    default:
        KdPrint(("GPUsion: SubmitCommand — unknown type %d\n", CmdHeader->Type));
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (NT_SUCCESS(Status)) {
        InterlockedIncrement64(&Context->Stats.InferenceCompleted);
    } else {
        InterlockedIncrement64(&Context->Stats.InferenceFailed);
    }

    return Status;
}

/* ─── GpusionDispatchInfer ──────────────────────────────────────────────── */

/*
 * GpusionDispatchInfer
 *
 * Route an inference command to the appropriate backend.
 *
 * Phase 1: CPU inference via llama.cpp / ONNX Runtime (user-mode component).
 *          The kernel driver queues the work; the user-mode inference
 *          engine picks it up via shared memory.
 *
 * Phase 2: Forward to FPGA via USB. The FPGA handles the matrix ops
 *          and returns results. Kernel driver manages the USB transfer.
 *
 * NOTE: Actual inference execution happens in user-mode (llama.cpp/ONNX)
 * or in the FPGA firmware. The kernel driver's job is routing only.
 */
static NTSTATUS
GpusionDispatchInfer(
    _In_ PGPUSION_DEVICE_CONTEXT    Context,
    _In_ PGPUSION_INFER_CMD Cmd
)
{
    KdPrint(("GPUsion: Infer — input=%llu bytes output=%llu bytes backend=%d\n",
             (ULONGLONG)Cmd->InputSizeBytes,
             (ULONGLONG)Cmd->OutputSizeBytes,
             Context->Backend));

    switch (Context->Backend) {

    case GPUSION_BACKEND_CPU:
        /*
         * Phase 1: CPU inference.
         *
         * The actual inference runs in user-mode via llama.cpp or ONNX Runtime.
         * The kernel driver signals the user-mode inference service via
         * a shared memory ring buffer. The service processes the request
         * and writes results back to the output GPU address (system RAM).
         *
         * TODO: Implement shared memory ring buffer signalling.
         *       For the skeleton this is a verified routing stub.
         */
        KdPrint(("GPUsion: CPU backend — queuing to user-mode inference engine\n"));
        return STATUS_SUCCESS;

    case GPUSION_BACKEND_FPGA:
        /*
         * Phase 2: FPGA dongle.
         *
         * Copy input tensor to FPGA via USB, trigger inference,
         * wait for completion interrupt, copy output back.
         *
         * TODO: Implement USB transfer pipeline.
         */
        KdPrint(("GPUsion: FPGA backend — USB transfer placeholder\n"));
        return STATUS_SUCCESS;

    case GPUSION_BACKEND_ASIC:
        /*
         * Phase 3: Custom ASIC.
         * TODO: Implement when hardware is ready.
         */
        KdPrint(("GPUsion: ASIC backend — placeholder\n"));
        return STATUS_SUCCESS;

    default:
        return STATUS_NOT_SUPPORTED;
    }
}

/* ─── GpusionDispatchMemcpy ─────────────────────────────────────────────── */

/*
 * GpusionDispatchMemcpy
 *
 * GPU-to-GPU memory copy within our VRAM proxy.
 * Since all our "GPU memory" is system RAM, this is a simple RtlCopyMemory.
 */
static NTSTATUS
GpusionDispatchMemcpy(
    _In_ PGPUSION_DEVICE_CONTEXT    Context,
    _In_ PGPUSION_CMD_HEADER        Cmd
)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Cmd);

    /*
     * TODO: Parse source/dest GPU addresses from command buffer,
     * validate they are within our VRAM proxy range,
     * then RtlCopyMemory(dest, src, size).
     */
    KdPrint(("GPUsion: MemCpy — stub\n"));
    return STATUS_SUCCESS;
}
