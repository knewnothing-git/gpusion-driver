/* Portable allocation-failure regression test; no Windows driver is loaded. */
#include "../driver/gpusion.h"

static size_t calls, fail_at, live_blocks;
static void *test_allocate(size_t size)
{
    void *result;
    if (++calls == fail_at) return NULL;
    result = calloc(1, size);
    if (result) live_blocks++;
    return result;
}
static void test_free(void *ptr)
{
    if (ptr) { assert(live_blocks > 0); live_blocks--; free(ptr); }
}
#undef ExAllocatePoolWithTag
#undef ExFreePoolWithTag
#undef KdPrint
#define ExAllocatePoolWithTag(pool, size, tag) test_allocate(size)
#define ExFreePoolWithTag(ptr, tag) test_free(ptr)
/* Existing diagnostic-format defects have a separate upstream PR. */
#define KdPrint(args) ((void)0)
#include "../driver/vram/vram_proxy.c"

int main(void)
{
    size_t failure;
    for (failure = 1; failure <= 6; failure++) {
        GPUSION_DEVICE_CONTEXT context = {0};
        DXGK_ALLOCATIONINFO existing_info = {0};
        DXGKARG_CREATEALLOCATION existing = {0};
        DXGK_ALLOCATIONINFO info[3] = {{0}};
        DXGKARG_CREATEALLOCATION request = {0};
        DXGKARG_DESTROYALLOCATION destroy = {0};
        HANDLE handle;
        size_t i;
        NTSTATUS result;
        context.Signature = GPUSION_DEVICE_SIGNATURE;
        existing_info.Size = 32;
        existing.NumAllocations = 1;
        existing.pAllocationInfo = &existing_info;
        calls = 0; fail_at = 0;
        assert(GpusionCreateAllocation(&context, &existing) == STATUS_SUCCESS);
        assert(live_blocks == 2);
        for (i = 0; i < 3; i++) info[i].Size = 64;
        request.NumAllocations = 3;
        request.pAllocationInfo = info;
        calls = 0; fail_at = failure;
        result = GpusionCreateAllocation(&context, &request);
        printf("fail_at=%zu status=%d live_blocks=%zu allocated_bytes=%zu\n",
               failure, result, live_blocks, context.Vram.AllocatedBytes);
        fflush(stdout);
        assert(result == STATUS_NO_MEMORY);
        assert(live_blocks == 2);
        assert(context.Vram.AllocatedBytes == 32);
        assert(context.Stats.BytesAllocated - context.Stats.BytesFreed == 32);
        for (i = 0; i < 3; i++) {
            assert(info[i].pDriverData == NULL);
            assert(info[i].DriverDataSize == 0);
        }
        /* Retry the same descriptors successfully after memory is available. */
        calls = 0; fail_at = 0;
        assert(GpusionCreateAllocation(&context, &request) == STATUS_SUCCESS);
        assert(live_blocks == 8);
        assert(context.Vram.AllocatedBytes == 224);
        destroy.NumAllocations = 1;
        destroy.pAllocationList = &handle;
        for (i = 0; i < 3; i++) {
            handle = info[i].pDriverData;
            assert(GpusionDestroyAllocation(&context, &destroy) == STATUS_SUCCESS);
        }
        handle = existing_info.pDriverData;
        assert(GpusionDestroyAllocation(&context, &destroy) == STATUS_SUCCESS);
        assert(live_blocks == 0);
        assert(context.Vram.AllocatedBytes == 0);
        assert(context.Stats.BytesAllocated == context.Stats.BytesFreed);
    }
    puts("PASS: six allocation failures, retries, existing allocation preservation");
    return 0;
}
