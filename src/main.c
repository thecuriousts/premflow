#include "app.h"
#include "premflow.h"
#include <stdio.h>
#include <stdlib.h>

int main(
    int argc,
    char *argv[]
) {
    if (!ensure_dirs()) {
        fprintf(stderr, "Failed to initialize directories. Exiting.\n");
        return 1;
    }
    read_config();

    PremflowMsg *msg = parse_argv(argc, argv);
    if (!msg) {
        fprintf(stderr, "Out of memory. Exiting.\n");
        return 1;
    }

    PremflowRuntime runtime;
    premflow_runtime_init(&runtime);

    ElomaxzProgram prog = {
        .init = pf_init,
        .update = pf_update,
        .view = pf_view,
        .msg_name = pf_msg_name,
        .free_model = pf_free_model,
        .free_msg = pf_free_msg,
        .free_cmd = pf_free_cmd,
        .handle_cmd = pf_handle_cmd,
        .debug = false,
        .user_data = &runtime,
    };

    /* One argv → one message; see docs/architecture.md for runner choice. */
    elomaxz_run_batch(&prog, (Msg *) &msg, 1);

    runtime = *premflow_runtime_get();
    return runtime.exit_code;
}
