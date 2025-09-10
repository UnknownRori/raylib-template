#define NOB_IMPLEMENTATION
#include "./external/nob.h/nob.h"
#include <string.h>

#define BUILD_PATH "./build"

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    nob_mkdir_if_not_exists(BUILD_PATH);

    const char *program = nob_shift_args(&argc, &argv);
    if (argc <= 0) {
        const char* usage = nob_temp_sprintf("Usage %s <platform>", program);
        nob_log(NOB_INFO, "%s", usage);
        return 0;
    }

    const char *platform = nob_shift(argv, argc);

    Nob_Cmd cmd = {0};

    nob_log(NOB_INFO, "Starting Raylib Build");

    if (strcmp(platform, "desktop") == 0) {
        const char *cc = BUILD_PATH"/desktop";
        nob_cmd_append(&cmd, NOB_REBUILD_URSELF(cc, "./src-build/desktop.c"));
        if (!nob_cmd_run_sync(cmd)) return 1;

        cmd.count = 0;
        nob_cmd_append(&cmd, cc);
        if (!nob_cmd_run_sync(cmd)) return 1;
    }

    if (strcmp(platform, "web") == 0) {
        nob_log(NOB_INFO, "Please note that this is still experimental and it may not working properly");
        const char *cc = BUILD_PATH"/web";
        nob_cmd_append(&cmd, NOB_REBUILD_URSELF(cc, "./src-build/web.c"));
        if (!nob_cmd_run_sync(cmd)) return 1;

        cmd.count = 0;
        nob_cmd_append(&cmd, cc);
        if (!nob_cmd_run_sync(cmd)) return 1;
    }
    return 0;
}
