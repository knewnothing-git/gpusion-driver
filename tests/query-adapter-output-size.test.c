/*
 * Portable regression test for QueryAdapterInfo output-size validation.
 * This exercises the WDDM dispatch logic without loading a Windows driver.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../driver/gpusion.h"
#include "../driver/wddm/query_adapter_info.c"

enum { CANARY_BYTES = 8 };

typedef union {
    DXGK_DRIVERCAPS DriverCaps;
    DXGK_QUERYSEGMENTOUT SegmentInfo;
    unsigned char Bytes[sizeof(DXGK_QUERYSEGMENTOUT) + CANARY_BYTES];
} OUTPUT_BUFFER;

static void
run_size_case(
    DXGK_QUERYADAPTERINFOTYPE type,
    size_t required_size,
    BOOLEAN segment_query,
    UINT32 output_size
)
{
    OUTPUT_BUFFER buffer;
    unsigned char before[sizeof(buffer.Bytes)];
    GPUSION_DEVICE_CONTEXT context = {0};
    DXGKARG_QUERYADAPTERINFO query = {0};
    NTSTATUS result;
    size_t i;

    context.Signature = GPUSION_DEVICE_SIGNATURE;
    memset(buffer.Bytes, 0xA5, sizeof(buffer.Bytes));
    if (segment_query) {
        /* The valid path uses NbSegment == 0 as the count query. */
        ((DXGK_QUERYSEGMENTOUT*)buffer.Bytes)->NbSegment = 0;
        ((DXGK_QUERYSEGMENTOUT*)buffer.Bytes)->pSegmentDescriptor = NULL;
    }
    memcpy(before, buffer.Bytes, sizeof(before));

    query.Type = type;
    query.pOutputData = buffer.Bytes;
    query.OutputDataSize = output_size;
    result = GpusionQueryAdapterInfo(&context, &query);

    if (output_size < required_size) {
        assert(result == STATUS_BUFFER_TOO_SMALL);
        assert(memcmp(buffer.Bytes, before, sizeof(before)) == 0);
        return;
    }

    assert(result == STATUS_SUCCESS);
    for (i = required_size; i < sizeof(buffer.Bytes); i++) {
        assert(buffer.Bytes[i] == 0xA5);
    }
}

static void
test_type(DXGK_QUERYADAPTERINFOTYPE type, size_t required_size, BOOLEAN segment_query)
{
    const UINT32 sizes[] = {
        0,
        1,
        (UINT32)(required_size - 1),
        (UINT32)required_size,
        (UINT32)(required_size + CANARY_BYTES),
    };
    size_t i;

    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        run_size_case(type, required_size, segment_query, sizes[i]);
    }
}

int
main(void)
{
    test_type(DXGKQAITYPE_DRIVERCAPS, sizeof(DXGK_DRIVERCAPS), FALSE);
    test_type(DXGKQAITYPE_QUERYSEGMENT, sizeof(DXGK_QUERYSEGMENTOUT), TRUE);
    test_type(DXGKQAITYPE_QUERYSEGMENT3, sizeof(DXGK_QUERYSEGMENTOUT), TRUE);
    test_type(DXGKQAITYPE_QUERYSEGMENT4, sizeof(DXGK_QUERYSEGMENTOUT), TRUE);
    puts("PASS: QueryAdapterInfo rejects undersized output buffers and preserves canaries");
    return 0;
}
