#pragma once

#define NOB_IMPLEMENTATION
#include "../external/nob.h/nob.h"

#define PROJECT_NAME    "huntress"
#define BUILD_PATH      "./build"
#define DIST_PATH       "./dist"
#define RAYLIB_SRC      "./external/raylib/src"

char* raylib_module[] = {
    "rcore",
    "raudio",
#ifndef WEB_BUILD 
    "rglfw",
#endif
    "rmodels",
    "rshapes",
    "rtext",
    "rtextures",
    "utils",
};

char* minijam_module[] = {
    "main"
};

int build_raylib();
int build_project();
int build_dist();
