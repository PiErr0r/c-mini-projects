#ifndef ARENA_H
#define ARENA_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#define ARENA_REGION_INIT_CAP (8*1024)

typedef struct Region Region;
struct Region {
    Region* next;
    size_t count;
    size_t capacity;
    uintptr_t data[];
};

typedef struct {
    Region *begin, *end;
} Arena;

Region *new_region(size_t capacity)
{
    size_t bytes_size = sizeof(Region) + capacity * sizeof(uintptr_t);
    Region *r = (Region *)malloc(bytes_size);
    assert(r);
    r->count = 0;
    r->capacity = capacity;
    r->next = NULL;

    return r;
}

void free_region(Region *r)
{
    free(r);
}

void *arena_alloc(Arena *arena, size_t size_bytes)
{
    size_t size = (size_bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
    if (arena->end == NULL) {
        assert(arena->begin == NULL);
        size_t capacity = ARENA_REGION_INIT_CAP;
        if (size > capacity) capacity = size;
        arena->end = new_region(capacity);
        arena->begin = arena->end;
    }

    while (arena->end->count + size < arena->end->capacity && arena->end->next != NULL) {
        arena->end = arena->end->next;
    }

    if (arena->end->count + size > arena->end->capacity) {
        assert(arena->end->next == NULL);
        size_t capacity = ARENA_REGION_INIT_CAP;
        if (size > capacity) capacity = size;
        arena->end->next = new_region(capacity);
        arena->end = arena->end->next;
    }

    void* result = &arena->end->data[arena->end->count];
    arena->end->count += size;
    return result;
}

void *arena_realloc(Arena *arena, void* oldptr, size_t old_size, size_t new_size)
{
    if (new_size < old_size) return oldptr;
    void *newptr = arena_alloc(arena, new_size);
    char *old_c = (char*)oldptr;
    char *new_c = (char*)newptr;
    for (size_t i = 0; i < old_size; ++i) {
        new_c[i] = old_c[i];
    }
    return newptr;
}

void arena_reset(Arena *arena)
{
    for (Region *r = arena->begin; r != NULL; r = r->next) {
        r->count = 0;
    }
    arena->end = arena->begin;
}


void arena_free(Arena *arena)
{
    Region *r = arena->begin;
    while (r) {
        Region *rd = r;
        r = r->next;
        free_region(rd);
    }
    arena->begin = NULL;
    arena->end = NULL;
}

static Arena da_arena = {0};
#ifndef DA_ARENA
#define DA_ARENA da_arena
#endif

void free_da_arena(void)
{
    arena_free(&da_arena);
}

#define da_realloc(da, new_capacity)\
    do { \
        (da)->items = arena_realloc(\
            &DA_ARENA, \
            (da)->items, \
            (da)->capacity * sizeof(*(da)->items), \
            (new_capacity) * sizeof(*(da)->items)); \
        (da)->capacity = new_capacity; \
    } while(0);

#define DA_INIT_CAPACITY 256
#define da_reserve(da, expected_capacity) \
    do { \
        if ((expected_capacity) > (da)->capacity) {\
            size_t new_capacity = (da)->capacity; \
            if (new_capacity == 0) {\
                new_capacity = DA_INIT_CAPACITY; \
            } \
            while ((expected_capacity) > new_capacity) {\
                new_capacity *= 1.5; \
            }\
            da_realloc(da, new_capacity); \
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

#define da_remove_unordered(da, idx)\
    do {\
        (da)->items[(idx)] = (da)->items[--(da)->count];\
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

#endif // ARENA_H
