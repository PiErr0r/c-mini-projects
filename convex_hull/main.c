#include "helpers.h"

#define W 1600
#define H 1200
#define DT 1e-3

#define R 10
#define NUM_POINTS 20
#define NUM_RING_SEGMENTS 8

#define C_IDLE WHITE
#define C_CURR RED
#define C_NEXT YELLOW

typedef struct {
    Vector2* items;
    size_t count;
    size_t capacity;
} Points;

Vector2 get_random_vector(void) {
    Vector2 v = {
        .x = rand() % (W - R) + R / 2,
        .y = rand() % (H - R) + R / 2
    };
    return v;
}

// returns number >0 if w0->w1 is clockwise from v0->v1
double cross_direction(Vector2 vw0, Vector2 v1, Vector2 w1) {
    Vector2 a = Vector2Subtract(v1, vw0);
    Vector2 b = Vector2Subtract(w1, vw0);
    return a.x * b.y - a.y * b.x;
}

void jarvis_march(Points* points, Points* hull) {
    Vector2 point_on_hull = {W, H};
    da_foreach(points, Vector2, v) {
        if (v->x < point_on_hull.x) {
            point_on_hull.x = v->x;
            point_on_hull.y = v->y;
        }
    }
    assert(point_on_hull.x > 0);
    assert(point_on_hull.y > 0);

    size_t i = 0;
    Vector2 endpoint = {0};
    do {
        da_append(hull, point_on_hull);
        endpoint = points->items[0];
        da_foreach(points, Vector2, v) {
            if (Vector2Equals(endpoint, point_on_hull) || cross_direction(hull->items[i], endpoint, *v) < 0.f) {
                endpoint = *v;
            }
        }
        point_on_hull = endpoint;
        ++i;
    } while (endpoint.x != hull->items[0].x && endpoint.y != hull->items[0].y);
}

Vector2 *P_point_on_hull = NULL;
int compare_radial(const void* a, const void* b) {
    Vector2 va = *(Vector2*)a;
    Vector2 vb = *(Vector2*)b;
    assert(P_point_on_hull != NULL);
    Vector2 v0 = *P_point_on_hull;
    if (Vector2Equals((Vector2){0,0}, Vector2Subtract(va, v0))) {
        return -1;
    }
    if (Vector2Equals((Vector2){0,0}, Vector2Subtract(vb, v0))) {
        return 1;
    }
    return (int)cross_direction((Vector2){0,0}, Vector2Subtract(va, v0), Vector2Subtract(vb, v0));
}

void graham_scan(Points* points, Points* hull) {
    Vector2 point_on_hull = {-1, -1};
    da_foreach(points, Vector2, v) {
        if (v->y > point_on_hull.y) {
            point_on_hull = *v;
        } else if (fabsf(v->y - point_on_hull.y) < EPSILON && v->x > point_on_hull.x) {
            point_on_hull = *v;
        }
    }
    P_point_on_hull = malloc(sizeof(Vector2));
    *P_point_on_hull = point_on_hull;
    qsort(points->items, points->count, sizeof(Vector2), &compare_radial);
    free(P_point_on_hull);
    P_point_on_hull = NULL;

    da_append(hull, point_on_hull);
    for (size_t i = 1; i < points->count; ++i) {
        if (hull->count < 2) {
            da_append(hull, points->items[i]);
            continue;
        }
        Vector2 endpoint = points->items[i];
        do {
            if (cross_direction(hull->items[hull->count-2], hull->items[hull->count-1], endpoint) < 0.f) {
                da_append(hull, endpoint);
                break;
            } else {
                da_pop(hull);
            }
        } while (hull->count >= 2);
    }
}

int Area2(Vector2 a, Vector2 b, Vector2 c) {
    return (b.x-a.x)*(c.y-a.y) -
           (b.y-a.y)*(c.x-a.x);
}
// return true if c is on left of segment ab
bool is_left(Vector2 a, Vector2 b, Vector2 c) {
    return Area2(a, b, c) < 0.f;
}

float distance(Vector2 a, Vector2 b, Vector2 c) {
    float A = c.x - a.x;
    float B = c.y - a.y;
    float C = b.x - a.x;
    float D = b.y - a.y;

    float dot = A * C + B * D;
    float len_sq = C*C + D*D;

    float dx = c.x - a.x + C * dot / len_sq;
    float dy = c.y - a.y + D * dot / len_sq;

    return dx*dx + dy*dy;
}

void _quickhull(Vector2 a, Vector2 b, Points *S, Points *hull) {
    if (S->count == 0) {
        return;
    }
    float max_d = -1;
    Vector2 c = {0};
    da_foreach(S, Vector2, pt) {
        float d = distance(a, b, *pt);
        if (d > max_d) {
            max_d = d;
            c = *pt;
        }
    }
    da_append(hull, c);
    Points A = {0};
    Points B = {0};
    da_foreach(S, Vector2, pt) {
        if (is_left(a, c, *pt)) {
            da_append(&A, *pt);
        }
        if (is_left(c, b, *pt)) {
            da_append(&B, *pt);
        }
    }
    _quickhull(a, c, &A, hull);
    _quickhull(c, b, &B, hull);
}

void quickhull(Points* points, Points* hull) {
    Vector2 tl = {W + 1, H + 1};
    Vector2 br = {-1, -1};

    da_foreach(points, Vector2, pt) {
        if (pt->y < tl.y) {
            tl = *pt;
        } else if (fabs(pt->y - tl.y) < EPSILON && pt->x < tl.x) {
            tl = *pt;
        }
        if (pt->y > br.y) {
            br = *pt;
        } else if (fabs(pt->y - br.y) < EPSILON && pt->x > br.x) {
            br = *pt;
        }
    }

    Points A = {0};
    Points B = {0};
    da_foreach(points, Vector2, pt) {
        if (is_left(br, tl, *pt)) {
            da_append(&A, *pt);
        } else {
            da_append(&B, *pt);
        }
    }

    da_append(hull, br);
    _quickhull(br, tl, &A, hull);
    da_append(hull, tl);
    _quickhull(tl, br, &B, hull);
}


int main(void)
{
    time_t seed = time(NULL);
    srand(seed);
    printf("SEED: %ld\n", seed);
    InitWindow(W, H, "Raylib Template");
    SetTargetFPS(60);

    Points points = {0};
    for (size_t i = 0; i < NUM_POINTS; ++i) {
        Vector2 v = get_random_vector();
        da_append(&points, v);
    }

    Points hull = {0};
    // jarvis_march(&points, &hull);
    // graham_scan(&points, &hull);
    quickhull(&points, &hull);

    Vector2 vw0 = { 0, 0 };
    Vector2 v1 = { 2, 1 };
    Vector2 w1 = { 1, 2 };
    printf("%f\n", cross_direction(vw0, v1, w1));

    float t = 0.f;
    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        }
        if (IsKeyPressed(KEY_SPACE)) {
        }
        BeginDrawing();

        da_foreach(&points, Vector2, v) {
            DrawRing(*v, R - 2, R, 0, 360, NUM_RING_SEGMENTS, C_IDLE);
        }
        // DrawCircleV(points.items[0], R, RED);
        // for (size_t i = 1; i <= 4; ++i) {
        //     DrawCircleV(points.items[i], R, C_NEXT);
        // }

        Vector2* prev = &hull.items[hull.count - 1];
        da_foreach(&hull, Vector2, v) {
            // DrawLineV(*prev, *v, C_CURR);
            DrawCircleV(*v, R, C_CURR);
            prev = v;
        }

        ClearBackground(GetColor(0x181818FF));
        EndDrawing();
        t += DT;
    }
    CloseWindow();

    return 0;
}
