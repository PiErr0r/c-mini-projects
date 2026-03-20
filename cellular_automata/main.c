#include <string.h>
#include "helpers.h"

#define W 1600
#define H 1200
#define DT 1e-3

#define CELL_SIZE 15
#define ROW_SIZE (W / CELL_SIZE)
#define COL_SIZE (H / CELL_SIZE)
#define GRID_SIZE (ROW_SIZE * COL_SIZE)

bool grid[GRID_SIZE] = {0};

#define AT(i, j) grid[(j) + (i) * ROW_SIZE]
#define ATS(Xgrid, i, j) (Xgrid[(j) + (i) * ROW_SIZE])
#define ATW(y, x) grid[(x/CELL_SIZE) + (y/CELL_SIZE) * ROW_SIZE]

void clear_bg(void) {
    memset(grid, 0, GRID_SIZE * sizeof(bool));
}

unsigned int custom_bad_random() {
    static unsigned int r = 197998543;
    unsigned int a = 155921;
    unsigned int b = 352757;
    unsigned int c = 787757;
    unsigned int d = 368492711;
    r = (r * r * a + r * b + c) % d;
    return r;
}

void draw_grid(void) {
    for (size_t i = 0; i < COL_SIZE; ++i) {
        for (size_t j = 0; j < ROW_SIZE; ++j) {
            if (AT(i, j)) {
                DrawRectangle(j*CELL_SIZE, i*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLUE);
            }
        }
    }
}

void random_grid(void) {
    clear_bg();
    for (size_t i = 0; i < COL_SIZE/8; ++i) {
        for (size_t j = 0; j < ROW_SIZE/8; ++j) {
            AT(i, j) = (custom_bad_random() + 1) % 2;
        }
    }
}

void step(void) {
    // B3/S1234
    static bool ngrid[GRID_SIZE] = {0};
    for (size_t i = 0; i < COL_SIZE; ++i) {
        for (size_t j = 0; j < ROW_SIZE; ++j) {
            size_t count = 0;
            for (int di = -1; di <= 1; ++di) {
                if (i == 0 && di == -1) continue;
                if (i == COL_SIZE && di == 1) continue;
                for (int dj = -1; dj <= 1; ++dj) {
                    if (j == 0 && dj == -1) continue;
                    if (j == ROW_SIZE && dj == 1) continue;
                    if (di == 0 && dj == 0) continue;
                    if (AT((int)i+di, (int)j+dj)) {
                        ++count;
                    }
                }
            }
            if (AT(i, j)) {
                ATS(ngrid, i, j) = count >= 1 && count <= 4 ? true : false;
            } else {
                ATS(ngrid, i, j) = count == 3;
            }
        }
    }
    memcpy(grid, ngrid, GRID_SIZE);
}

int main(void)
{
    srand(time(NULL));
    InitWindow(W, H, "Raylib Template");
    SetTargetFPS(60);
    int count[10] = {0};
    for (size_t i = 0; i < COL_SIZE; ++i) {
        for (size_t j = 0; j < ROW_SIZE; ++j) {
            // AT(i, j) = (i+j)%2;
            count[custom_bad_random()%10]++;
        }
    }

    random_grid();
    float prev_t = 0;
    float t = 0.f;
    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            clear_bg();
            Vector2 mouse = GetMousePosition();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            random_grid();
        }
        if (t - prev_t > 0.01f) {
            prev_t = t;
            step();
        }

        BeginDrawing();
        draw_grid();
        ClearBackground(GetColor(0x181818FF));
        EndDrawing();
        t += DT;
    }
    CloseWindow();

    return 0;
}
