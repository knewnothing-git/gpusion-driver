/* Run from the repository root:
 * cc -std=c99 -Wall -Wextra -Werror tests/submit-command.test.c -o /tmp/gpusion-submit-test
 * /tmp/gpusion-submit-test
 * Uses the existing non-Windows stubs; does not test WDK or hardware behavior.
 */
#include "../driver/wddm/submit_command.c"

int main(void)
{
    GPUSION_DEVICE_CONTEXT context = {0};
    DXGKARG_SUBMITCOMMAND submission = {0};
    GPUSION_INFER_CMD command = {0};
    ULONG size;

    context.Signature = GPUSION_DEVICE_SIGNATURE;
    context.Backend = GPUSION_BACKEND_CPU;
    command.Header.Magic = GPUSION_CMD_MAGIC;
    command.Header.Type = GPUSION_CMD_INFER;
    submission.DmaBufferPhysicalAddress.QuadPart = (LONGLONG)(ULONG_PTR)&command;

    /* Keep backing storage valid while testing every advertised short length. */
    for (size = 0; size < sizeof(command); size++) {
        submission.DmaBufferSize = size;
        command.Header.SizeBytes = size;
        assert(GpusionSubmitCommand(&context, &submission) == STATUS_BUFFER_TOO_SMALL);
    }
    assert(context.Stats.InferenceCompleted == 0);
    assert(context.Stats.InferenceFailed == sizeof(command));

    submission.DmaBufferSize = sizeof(command);
    command.Header.SizeBytes = sizeof(command);
    assert(GpusionSubmitCommand(&context, &submission) == STATUS_SUCCESS);
    assert(context.Stats.InferenceCompleted == 1);

    command.Header.Type = GPUSION_CMD_NOP;
    submission.DmaBufferSize = sizeof(command.Header);
    command.Header.SizeBytes = sizeof(command.Header);
    assert(GpusionSubmitCommand(&context, &submission) == STATUS_SUCCESS);
    puts("PASS: truncated INFER commands rejected; full INFER and header-only NOP accepted");
    return 0;
}
