#define NOB_IMPLEMENTATION
#include "nob.h"

#define SOURCE_DIR "src/"
#define BUILD_DIR "build/"

#define APP_NAME "click-on-it"

static Nob_Cmd cmd = {0};
static Nob_Procs procs = {0};

#define SV(cstr) nob_sv_from_cstr(cstr)

int main(int argc, char *argv[]) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    bool ok;

    nob_mkdir_if_not_exists("build/");

    struct {
        const char *input;
        const char *output;
    } source_files[] = {
        {SOURCE_DIR"main.c", BUILD_DIR"main.o"}
    };

    for(int i = 0; i < NOB_ARRAY_LEN(source_files); ++i) {
        nob_cc(&cmd);
        nob_cmd_append(&cmd, "-c");
        nob_cc_flags(&cmd);
        nob_cmd_append(&cmd, "-Iinclude/");
        nob_cc_inputs(&cmd, source_files[i].input);
        nob_cc_output(&cmd, source_files[i].output);

        nob_cmd_run_opt(&cmd, (Nob_Cmd_Opt){
            .async = &procs
        });
    }

    if(!nob_procs_wait_and_reset(&procs)) return 1;

    nob_cc(&cmd);
    nob_cc_output(&cmd, APP_NAME);
    for(int i = 0; i < NOB_ARRAY_LEN(source_files); ++i) {
        nob_cc_inputs(&cmd, source_files[i].output);
    }

    nob_cmd_append(&cmd, "-L./lib/");
    nob_cmd_append(&cmd, "-lSDL3");
    nob_cmd_append(&cmd, "-lSDL3_ttf");

    if(!nob_cmd_run(&cmd)) return 1;

    return 0;
}
