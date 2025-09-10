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


#if defined(_WIN32) || defined(_WIN64)
    const char* output_exe = nob_temp_sprintf("%s/%s.exe", BUILD_PATH, PROJECT_NAME);
    const char* out = nob_temp_sprintf("%s/%s.exe", DIST_PATH, PROJECT_NAME);
#elif defined(__linux__)
    const char* output_exe = nob_temp_sprintf("%s/%s", BUILD_PATH, PROJECT_NAME);
    const char* out = nob_temp_sprintf("%s/%s", DIST_PATH, PROJECT_NAME);
#else
    #error "Unsupported OS"
#endif

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

    for (size_t i = 0; i < NOB_ARRAY_LEN(minijam_module); i++) {
        const char* in_path = nob_temp_sprintf("src/%s.c", minijam_module[i]);
        const char* out_path = nob_temp_sprintf("%s/%s.o", project_build_path, minijam_module[i]);

        nob_da_append(&project_obj, out_path);
        if (nob_needs_rebuild(out_path, &in_path, 1)) {
            cmd.count = 0;
            nob_cmd_append(&cmd, "gcc");
            nob_cc_inputs(&cmd, "-c", in_path);
            nob_cmd_append(&cmd, "-I"RAYLIB_SRC);
            nob_cmd_append(&cmd, "-L./build");
            nob_cc_output(&cmd, out_path);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&project_procs, proc);
        }
    }

    if (!nob_procs_wait(project_procs)) nob_return_defer(false);

    const char* output_exe = nob_temp_sprintf("./build/%s", PROJECT_NAME);
    if (nob_needs_rebuild(output_exe, project_obj.items, project_obj.count)) {
        cmd.count = 0;
        nob_cmd_append(&cmd, "gcc");

        for (size_t i = 0; i < NOB_ARRAY_LEN(minijam_module); i++) {
            const char* in_path = nob_temp_sprintf("%s/%s.o", project_build_path, minijam_module[i]);
            nob_cc_inputs(&cmd, in_path);
        }

        nob_cmd_append(&cmd, "-L./build/raylib/");
        nob_cmd_append(&cmd, "-lraylib");
        nob_cmd_append(&cmd, "-lwinmm");
        nob_cmd_append(&cmd, "-lgdi32");
        nob_cmd_append(&cmd, "-lopengl32");
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
            nob_cmd_append(&cmd, "gcc");
            nob_cc_inputs(&cmd, "-c", in_path);
            nob_cmd_append(&cmd, "-I"RAYLIB_SRC"/external/glfw/include");
            nob_cmd_append(&cmd, "-I"RAYLIB_SRC);
            nob_cmd_append(&cmd, "-DPLATFORM_DESKTOP");
#if defined(__linux__)
            nob_cmd_append(&cmd, "-D_GLFW_X11");
            nob_cmd_append(&cmd, "-fPIC");
#endif
            nob_cc_output(&cmd, out_path);

            Nob_Proc proc = nob_cmd_run_async(cmd);
            nob_da_append(&raylib_procs, proc);
        }
    }

    if (!nob_procs_wait(raylib_procs)) nob_return_defer(false);


#if defined(_WIN32) || defined(_WIN64)
    const char* lib_path = nob_temp_sprintf("%s/libraylib.a", raylib_build_path);
    if (nob_needs_rebuild(lib_path, raylib_obj.items, raylib_obj.count)) {
        cmd.count = 0;
        nob_cmd_append(&cmd, "ar");
        nob_cmd_append(&cmd, "-crs", lib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_module); i++) {
            const char* in_path = nob_temp_sprintf("%s/%s.o", raylib_build_path, raylib_module[i]);
            nob_cc_inputs(&cmd, in_path);
        }
        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }

#elif defined(__linux__)
    const char* lib_path = nob_temp_sprintf("%s/libraylib.a", raylib_build_path);
    if (nob_needs_rebuild(lib_path, raylib_obj.items, raylib_obj.count)) {
        cmd.count = 0;
        nob_cmd_append(&cmd, "gcc");
        nob_cmd_append(&cmd, "-shared", "-o", lib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_module); i++) {
            const char* in_path = nob_temp_sprintf("%s/%s.o", raylib_build_path, raylib_module[i]);
            nob_cc_inputs(&cmd, in_path);
        }
        if (!nob_cmd_run_sync(cmd)) nob_return_defer(false);
    }

#else
    #error "Unsupported OS"
#endif


defer:
    nob_cmd_free(cmd);
    nob_da_free(raylib_obj);
    return result;
}
