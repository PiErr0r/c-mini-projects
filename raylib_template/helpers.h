#include <assert.h>
#include <raylib.h>
#include <raymath.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UNUSED(x) (void)(x)
#define TODO(message) do { fprintf(stderr, "%s:%d TODO: %s\n", __FILE__, __LINE__, message);} while (0)
#define UNREACHABLE(message) do { fprintf(stderr, "%s:%d UNREACHABLE: %s\n", __FILE__, __LINE__, message); abort(); } while (0)

#define da_realloc(da)\
            (da)->items = \
                realloc((da)->items, (da)->capacity * sizeof(*(da)->items))

#define DA_INIT_CAPACITY 256
#define da_reserve(da, expected_capacity) \
    do { \
        if ((expected_capacity) > (da)->capacity) {\
            if ((da)->capacity == 0) {\
                (da)->capacity = DA_INIT_CAPACITY; \
            } \
            while ((expected_capacity) > (da)->capacity) {\
                (da)->capacity *= 1.5; \
            }\
            da_realloc(da); \
            assert((da)->items != NULL); \
        }\
    } while (0)

#define da_append(da, item)\
    do {\
        da_reserve(da, (da)->count + 1);\
        (da)->items[(da)->count++] = (item);\
    } while (0)

#define da_last(da) (da)->items[(da)->count-1]

#define da_pop(da)\
    do {\
        (da)->count--;\
    } while (0)

#define da_remove(da, idx)\
    do {\
        for (size_t index = (idx); index + 1 < (da)->count; ++index) {\
            (da)->items[index] = (da)->items[index + 1];\
        }\
        --(da)->count;\
    } while (0)

#define da_remove_unordered(da, i)\
    do {\
        size_t j = (i);\
        (da)->items[j] = (da)->items[--(da)->count];\
    } while(0)

#define da_foreach(da, Type, it)\
    for (Type *it = (da)->items; it < (da)->items + (da)->count; ++it)

#define heap_parent(idx) ((idx) - 1) >> 1
#define heap_left(idx) ((idx) << 1) + 1
#define heap_right(idx) ((idx) << 1) + 2

#define heap_push(da, item, heap_cmp_fn)\
    do {\
        da_append(da, item);\
        int idx = (da)->count - 1;\
        while (idx && heap_cmp_fn(item, (da)->items[heap_parent(idx)]) > 0) {\
            (da)->items[idx] = (da)->items[heap_parent(idx)];\
            idx = heap_parent(idx);\
        }\
        (da)->items[idx] = item;\
    } while(0)

#define heap_swap(x,y)\
    do {\
        unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp,&y,sizeof(x)); \
        memcpy(&y,&x,       sizeof(x)); \
        memcpy(&x,swap_temp,sizeof(x)); \
    } while(0)

#define heap_heapify(da, idx, heap_cmp_fn)\
    do {\
        int jdx = idx;\
        bool should_heapify = true;\
        while (should_heapify) {\
            should_heapify = false;\
            int largest;\
            if (heap_left(jdx) < (da)->count && heap_cmp_fn((da)->items[heap_left(jdx)], (da)->items[jdx]) > 0) {\
                largest = heap_left(jdx);\
            } else {\
                largest = jdx;\
            }\
            if (heap_right(jdx) < (da)->count && heap_cmp_fn((da)->items[heap_right(jdx)], (da)->items[largest]) > 0) {\
                largest = heap_right(jdx);\
            }\
            if (largest != jdx) {\
                heap_swap((da)->items[jdx], (da)->items[largest]);\
                jdx = largest;\
                should_heapify = true;\
            }\
        }\
    } while(0)

#define heap_pop(da, heap_cmp_fn)\
    do {\
        (da)->items[0] = (da)->items[--(da)->count];\
        heap_heapify(da, 0, heap_cmp_fn); \
    } while(0)

/*
 * CmpFn takes two arguments of type T and returns
 * > 0 if `a` should be on top of the heap
 * < 0 if `b` should be on top of the heap
 * = 0 if `a` and `b` are equal
 * EXAMPLE:
 * typedef int (*CmpFn)(int a, int b);
 * */

#define C_N 20
Color COLORS[C_N] = {
//	LIGHTGRAY, used for DYING
	GRAY,
	DARKGRAY,
//	YELLOW, used for DYING
	GOLD,
	ORANGE,
	PINK,
	RED,
	MAROON,
	GREEN,
	LIME,
	DARKGREEN,
	SKYBLUE,
	BLUE,
	DARKBLUE,
	PURPLE,
	VIOLET,
	DARKPURPLE,
	BEIGE,
	BROWN,
	DARKBROWN,
	MAGENTA
};

void print_Vector2(Vector2 v) {
    printf("{x = %f, y = %f\n}", v.x, v.y);
}

void print_Vector3(Vector3 v) {
    printf("{x = %f, y = %f, z = %f}\n", v.x, v.y, v.z);
}

void print_Quaternion(Quaternion q) {
    printf("{w = %f, x = %f, y = %f, z = %f}\n", q.w, q.x, q.y, q.z);
}
