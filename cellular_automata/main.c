#include <string.h>
#include "helpers.h"

#define W 1600
#define H 1200
#define DT 1e-3

#define CELL_SIZE 10
#define ROW_SIZE (W / CELL_SIZE)
#define COL_SIZE (H / CELL_SIZE)
#define GRID_SIZE (ROW_SIZE * COL_SIZE)

bool grid[GRID_SIZE] = {0};

#define AT(i, j) grid[(j) + (i) * ROW_SIZE]
#define ATS(Xgrid, i, j) (Xgrid[(j) + (i) * ROW_SIZE])
#define ATW(y, x) grid[(int)(x/CELL_SIZE) + (int)(y/CELL_SIZE) * ROW_SIZE]

#define SET_B(bs, n) (*bs) |= (1 << (n +  0))
#define SET_S(bs, n) (*bs) |= (1 << (n + 16))
#define IS_SET_B(bs, n) (*bs) & (1 << (n +  0))
#define IS_SET_S(bs, n) (*bs) & (1 << (n + 16))

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

void draw_grid(bool playing) {
    for (size_t i = 0; i < COL_SIZE; ++i) {
        for (size_t j = 0; j < ROW_SIZE; ++j) {
            if (!playing) {
                DrawRectangle(j*CELL_SIZE + CELL_SIZE / 2, i*CELL_SIZE + CELL_SIZE / 2, 1, 1, RED);
            }
            if (AT(i, j)) {
                DrawRectangle(j*CELL_SIZE, i*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLUE);
            }
        }
    }
}

void random_grid(void) {
    clear_bg();
    for (size_t i = 0; i < COL_SIZE; ++i) {
        for (size_t j = 0; j < ROW_SIZE; ++j) {
            AT(i, j) = (custom_bad_random() + 1) % 2;
        }
    }
}

void step(int *bs) {
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
                ATS(ngrid, i, j) = IS_SET_S(bs, count);
            } else {
                ATS(ngrid, i, j) = IS_SET_B(bs, count);
            }
        }
    }
    memcpy(grid, ngrid, GRID_SIZE);
}

void bs_error(const char *bs_string) {
    fprintf(stderr, "Faulty BS rulestring: %s\n", bs_string);
    exit(1);
}

void consume(const char *bs_string, size_t *i, char expected) {
    if (bs_string[*i] != expected) {
        bs_error(bs_string);
    }
    *i += 1;
}

void append(char digit, int *bs, bool should_shift) {
    if (should_shift) {
        SET_S(bs, digit - '0');
    } else {
        SET_B(bs, digit - '0');
    }
}

void parse_bs_rulestring(const char *bs_string, int *bs) {
    size_t i = 0;
    size_t L = strlen(bs_string);
    consume(bs_string, &i, 'B');
    while (i < L && bs_string[i] != '/') {
        if (bs_string[i] < '0' || bs_string[i] > '9') {
            bs_error(bs_string);
        }
        append(bs_string[i], bs, false);
        ++i;
    }
    consume(bs_string, &i, '/');
    consume(bs_string, &i, 'S');
    while (i < L && bs_string[i] != '\0') {
        if (bs_string[i] < '0' || bs_string[i] > '9') {
            bs_error(bs_string);
        }
        append(bs_string[i], bs, true);
        ++i;
    }
}

int main(int argc, const char** argv)
{
    int bs = 0;
    if (argc != 2) {
        printf("Usage: %s [bs-rulestring]\n", argv[0]);
        return 1;
    }
    parse_bs_rulestring(argv[1], &bs);
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

    // random_grid();
    float prev_t = 0;
    float t = 0.f;
    bool play = false;
    bool down = false;
    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            down = true;
        }
        if (down && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            down = false;
        }
        if (down) {
            Vector2 mouse = GetMousePosition();
            ATW(mouse.y, mouse.x) = true;
        }
        if (IsKeyPressed(KEY_SPACE)) {
            play = !play;
        }
        if (IsKeyPressed(KEY_R)) {
            random_grid();
        }
        if (IsKeyPressed(KEY_C)) {
            clear_bg();
        }
        if (play && t - prev_t > 0.01f) {
            prev_t = t;
            step(&bs);
        }

        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        draw_grid(play);
        EndDrawing();
        t += DT;
    }
    CloseWindow();

    return 0;
}
