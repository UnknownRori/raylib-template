#include <stdio.h>
#include <raylib.h>

int main(void) {
    InitWindow(900, 600, "Huntress");

    Color background = (Color) {
        .a = 255,
        .b = 50,
        .g = 50,
        .r = 200,
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(background);
        EndDrawing();
    }
    return 0;
}
