#include "helpers.h"

#define W 1600
#define H 1200
#define DT 1e-3

#define CELL_SIZE 10
#define NR (H/CELL_SIZE)
#define NC (W/CELL_SIZE)

// URDL
int dirs[4][2] = {{-1,0},{0,1},{1,0},{0,-1}};
#define RAND_DIR(dr, dc) \
    do {\
        int N = random() % 4; \
        dr = dirs[N][0]; \
        dc = dirs[N][1]; \
    } while (0)

typedef struct {
    unsigned char edges: 4; //URDL
    unsigned char visited: 4;
} Node;

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Moves;

Node graph[NR][NC] = {0};

void print_graph(void)
{
    if (CELL_SIZE < 50) return;
    for (size_t r = 0; r < NR; ++r) {
        for (size_t c = 0; c < NC; ++c) {
            printf("%d", graph[r][c].visited);
        }
        printf("\n");
    }
}

void add_edge(Node* a, Node *b, int dr, int dc)
{
    if (dr) {
        a->edges |= dr > 0 ? 0x2 : 0x8;
        b->edges |= dr > 0 ? 0x8 : 0x2;
    } else {
        a->edges |= dc > 0 ? 0x4 : 0x1;
        b->edges |= dc > 0 ? 0x1 : 0x4;
    }
}

char get_edge(int dr, int dc)
{
    if (dr) {
        return dr > 0 ? 0x2 : 0x8;
    } else {
        return dc > 0 ? 0x4 : 0x1;
    }
}

char reverse_edge(char edge)
{
    return (edge >> 2) ? (edge >> 2) : (edge << 2);
}

void get_dir_from_edge(char edge, int *dr, int *dc)
{
    switch (edge) { // URDL
        case 0x1: *dr =  0; *dc = -1; return;
        case 0x2: *dr =  1; *dc =  0; return;
        case 0x4: *dr =  0; *dc =  1; return;
        case 0x8: *dr = -1; *dc =  0; return;
        default: UNREACHABLE("Unknown edge");
    }
}

void aldous_broder(void)
{
    int r = 0; //random() % NR;
    int c = 0; //random() % NC;

    int dr, dc;
    size_t visited = 0;
    Node *prev = NULL;
    do {
        int nr, nc;
        do {
            RAND_DIR(dr, dc);
            nr = r + dr;
            nc = c + dc;
        } while (!(0 <= nr && nr < NR && 0 <= nc && nc < NC));

        Node *node = &graph[nr][nc];
        if (!node->visited) {
            ++visited;
            node->visited = 0x1;
            if (prev != NULL) {
                add_edge(prev, node, dr, dc);
            }
        }
        prev = node;
        r = nr;
        c = nc;
    } while (visited < NR * NC);
}

void delete_loop(int sr, int sc, Moves *moves)
{
    int r = sr;
    int c = sc;
    int dr, dc;
    do {
        char edge = da_last(moves);
        da_pop(moves);
        get_dir_from_edge(reverse_edge(edge), &dr, &dc);
        r += dr;
        c += dc;
        assert(0 <= r && r < NR && 0 <= c && c <= NC && graph[r][c].visited == 0x1);
        graph[r][c].visited = 0x0;
    } while (r != sr || c != sc);
    graph[sr][sc].visited = 0x1;
}

void add_to_maze(int sr, int sc, Moves *moves)
{
    int dr, dc;
    da_foreach(moves, char, edge) {
        get_dir_from_edge(*edge, &dr, &dc);
        assert(graph[sr][sc].visited == 0x1);
        graph[sr][sc].visited = 0x2;
        graph[sr][sc].edges |= *edge;
        sr += dr;
        sc += dc;
        graph[sr][sc].edges |= reverse_edge(*edge);
    }
    moves->count = 0;
}

void find_new_start(int *sr, int *sc)
{
    for (size_t r = 0; r < NR; ++r) {
        for (size_t c = 0; c < NC; ++c) {
            if (graph[r][c].visited == 0x0) {
                *sr = r;
                *sc = c;
                return;
            }
        }
    }
    UNREACHABLE("Faulty graph");
}

void wilson(void)
{
    int r, c;
    int dr, dc;
    size_t visited = 1;
    graph[NR-1][NC-1].visited = 0x2;

    Moves moves = {0};
    int sr, sc;
    bool should_start = true;
    do {
        if (should_start) {
            should_start = false;
            find_new_start(&sr, &sc);
            r = sr;
            c = sc;
            graph[r][c].visited = 0x1;
        }
        int nr, nc;
        do {
            RAND_DIR(dr, dc);
            nr = r + dr;
            nc = c + dc;
        } while (!(0 <= nr && nr < NR && 0 <= nc && nc < NC));

        Node *node = &graph[nr][nc];
        da_append(&moves, get_edge(dr, dc));
        if (node->visited == 0x0) {
            node->visited = 0x1;
        } else if (node->visited == 0x1) {
            delete_loop(nr, nc, &moves);
        } else if (node->visited == 0x2) {
            visited += moves.count;
            add_to_maze(sr, sc, &moves);
            // print_graph();
            should_start = true;
        } else {
            UNREACHABLE("unexpected visited value");
        }
        r = nr;
        c = nc;
    } while (visited < NR*NC);
}

void draw_graph(void)
{
    for (size_t r = 0; r < NR; ++r) {
        for (size_t c = 0; c < NC; ++c) {
            Node *node = &graph[r][c];
            int left = c * CELL_SIZE;
            int right = (c+1) * CELL_SIZE;
            int top = r * CELL_SIZE;
            int bot = (r+1) * CELL_SIZE;
            Color color = GetColor(0x181818FF);
            switch (node->visited) {
                case 0x0: break; // default
                case 0x1: break; // default
                case 0X2: color = GREEN; break; // Visited
                case 0x4: color = BLUE;  break; // Start
                case 0x8: color = RED;   break; // Finish
                default:
                    UNREACHABLE("Unknown visited state");
            }
            DrawRectangle(left, top, CELL_SIZE, CELL_SIZE, color);
            if (c != NC - 1 && !(node->edges & 0x4)) { // right
                DrawLine(right, top, right, bot, RED);
            }
            if (r != NR - 1 && !(node->edges & 0x2)) { // down
                DrawLine(left, bot, right, bot, RED);
            }
        }
    }
}

void step(void)
{
    static int n = 1;
    static int prev[NC*NR][2] = {{0, 0}};
    static int next[NC*NR][2] = {0};
    static bool done = false;
    if (done) return;

    int m = 0, r, c;
    for (int i = 0; i < n; ++i) {
        int dr, dc;
        r = prev[i][0];
        c = prev[i][1];
        if (graph[r][c].visited & 0x8) {
            done = true;
            return;
        }
        if (graph[r][c].visited > 0x1 && graph[r][c].visited != 0x4) continue;
        if (graph[r][c].visited != 0x4)
            graph[r][c].visited = 0x2;
        for (int e = 0x1; e <= 0x8; e <<= 1) {
            get_dir_from_edge(e, &dr, &dc);
            int nr = r + dr;
            int nc = c + dc;
            if (0 <= nr && nr < NR && 0 <= nc && nc < NC && graph[r][c].edges & e) {
                if (graph[nr][nc].visited <= 0x1 || graph[nr][nc].visited & 0x8) {
                    next[m][0] = nr;
                    next[m][1] = nc;
                    ++m;
                }
            }
        }
    }
    memcpy(prev, next, 2 * m * sizeof(int));
    n = m;
}

int main(void)
{
    srand(time(NULL));
    InitWindow(W, H, "Raylib Template");
    SetTargetFPS(60);

    aldous_broder();
    graph[0][0].visited = 0x4;
    graph[NR-1][NC-1].visited = 0x8;
    // wilson();

    // return 0;


    float t = 0.f;
    float prev = 0.f;
    bool solve = false;
    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        }
        if (IsKeyPressed(KEY_SPACE)) {
            solve = !solve;
        }
        if (solve) step();
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        draw_graph();
        EndDrawing();
        t += DT;
    }
    CloseWindow();

    return 0;
}
