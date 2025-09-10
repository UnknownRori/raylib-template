#define WEB_BUILD
#include "./config.h"

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    nob_mkdir_if_not_exists(BUILD_PATH);

    if (!build_raylib()) {
        nob_log(NOB_ERROR, "Raylib build failed!");
        return 1;
    }

    nob_log(NOB_INFO, "Starting Game Build");

    if (!build_project()) {
        nob_log(NOB_ERROR, "Game build failed!");
        return 1;
    }

    nob_log(NOB_INFO, "Bundling...");

    if (!build_dist()) {
        nob_log(NOB_ERROR, "Bundling failed!");
        return 1;
    }

    return 0;
}

int build_dist()
{
    bool result = true;
    Nob_Cmd cmd = {0};
    nob_mkdir_if_not_exists(DIST_PATH);


    const char* output_exe = nob_temp_sprintf("%s/%s.html", BUILD_PATH, PROJECT_NAME);
    const char* out = nob_temp_sprintf("%s/%s.html", DIST_PATH, PROJECT_NAME);

    nob_copy_file(output_exe, out);
    const char* resources = DIST_PATH"/resources";
    nob_copy_directory_recursively("./resources", resources);
}

int build_project()
{
    bool result = true;
    Nob_Cmd cmd = {0};
    const char* project_build_path = BUILD_PATH"/"PROJECT_NAME;

    Nob_File_Paths project_obj = {0};
    Nob_Procs project_procs = {0};
    nob_mkdir_if_not_exists(project_build_path);
    const char* scene_path = nob_temp_sprintf("%s/scene", project_build_path);
    nob_mkdir_if_not_exists(scene_path);

    for (size_t i = 0; i < NOB_ARRAY_LEN(minijam_module); i++) {
        const char* in_path = nob_temp_sprintf("src/%s.c", minijam_module[i]);
        const char* out_path = nob_temp_sprintf("%s/%s.o", project_build_path, minijam_module[i]);

        nob_da_append(&project_obj, out_path);
        if (nob_needs_rebuild(out_path, &in_path, 1)) {
            cmd.count = 0;
#if defined(_WIN32) || defined(_WIN64)
            nob_cmd_append(&cmd, "emcc.bat");
#elif defined(__linux__)
            nob_cmd_append(&cmd, "emcc");
#else
    #error "Unsupported OS"
#endif
            nob_cc_inputs(&cmd, "-c", in_path);
            nob_cmd_append(&cmd, "-I"RAYLIB_SRC);
            nob_cmd_append(&cmd, "-L./build");
            nob_cc_output(&cmd, out_path);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&project_procs, proc);
        }
    }

    if (!nob_procs_wait(project_procs)) nob_return_defer(false);

    const char* output_exe = nob_temp_sprintf("./build/%s.html", PROJECT_NAME);
    if (nob_needs_rebuild(output_exe, project_obj.items, project_obj.count)) {
        cmd.count = 0;
#if defined(_WIN32) || defined(_WIN64)
            nob_cmd_append(&cmd, "emcc.bat");
#elif defined(__linux__)
            nob_cmd_append(&cmd, "emcc");
#else
    #error "Unsupported OS"
#endif

        for (size_t i = 0; i < NOB_ARRAY_LEN(minijam_module); i++) {
            const char* in_path = nob_temp_sprintf("%s/%s.o", project_build_path, minijam_module[i]);
            nob_cc_inputs(&cmd, in_path);
        }

        nob_cmd_append(&cmd, "-L./build/raylib/");
        nob_cmd_append(&cmd, "-I./build/raylib/");
        nob_cmd_append(&cmd, "-DPLATFORM_WEB");
        nob_cmd_append(&cmd, "--preload-file", "resources");
        nob_cmd_append(&cmd, "-s", "ASSERTIONS");
        nob_cmd_append(&cmd, "-s", "USE_GLFW=3");
        nob_cmd_append(&cmd, "-s", "ASYNCIFY");
        nob_cmd_append(&cmd, "-s", "TOTAL_STACK=64MB");
        nob_cmd_append(&cmd, "-s", "INITIAL_MEMORY=128MB");
        nob_cmd_append(&cmd, "-lraylib");
        nob_cc_output(&cmd, output_exe);

        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }


defer:
    nob_cmd_free(cmd);
    nob_da_free(project_obj);
    return result;
}

int build_raylib()
{

    bool result = true;
    Nob_Cmd cmd = {0};
    const char* raylib_build_path = "./build/raylib";

    Nob_File_Paths raylib_obj = {0};
    Nob_Procs raylib_procs = {0};
    nob_mkdir_if_not_exists(raylib_build_path);

    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_module); i++) {
        const char* in_path = nob_temp_sprintf(RAYLIB_SRC "/%s.c", raylib_module[i]);
        const char* out_path = nob_temp_sprintf("%s/%s.o", raylib_build_path, raylib_module[i]);

        nob_da_append(&raylib_obj, out_path);
        if (nob_needs_rebuild(out_path, &in_path, 1)) {
            cmd.count = 0;
#if defined(_WIN32) || defined(_WIN64)
            nob_cmd_append(&cmd, "emcc.bat");
#elif defined(__linux__)
            nob_cmd_append(&cmd, "emcc");
#else
    #error "Unsupported OS"
#endif
            nob_cc_inputs(&cmd, "-c", in_path);
            nob_cmd_append(&cmd, "-I./raylib/src/external/glfw/include/");
            nob_cmd_append(&cmd, "-DPLATFORM_WEB");
            nob_cc_output(&cmd, out_path);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&raylib_procs, proc);
        }
    }

    if (!nob_procs_wait(raylib_procs)) nob_return_defer(false);


    const char* lib_path = nob_temp_sprintf("%s/libraylib.a", raylib_build_path);
    if (nob_needs_rebuild(lib_path, raylib_obj.items, raylib_obj.count)) {
        cmd.count = 0;
#if defined(_WIN32) || defined(_WIN64)
            nob_cmd_append(&cmd, "emar.bat");
#elif defined(__linux__)
            nob_cmd_append(&cmd, "emar");
#else
    #error "Unsupported OS"
#endif
        nob_cmd_append(&cmd, "rcs", lib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_module); i++) {
            const char* in_path = nob_temp_sprintf("%s/%s.o", raylib_build_path, raylib_module[i]);
            nob_cc_inputs(&cmd, in_path);
        }
        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }



defer:
    nob_cmd_free(cmd);
    nob_da_free(raylib_obj);
    return result;
}
