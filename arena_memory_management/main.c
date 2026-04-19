#include <stdio.h>
#define DA_ARENA new_arena
#include "arena.h"

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Nums;

Arena new_arena = {0};

int main(void)
{
    // test with dynamic array from helpers and try to build B-tree
    Nums nums1 = {0};
    Nums nums2 = {0};
    Nums nums3 = {0};
    Nums nums4 = {0};

    for (size_t i = 0; i < 1000000; ++i) {
       da_append(&nums1, 1 * i);
       da_append(&nums2, 2 * i);
       da_append(&nums3, 3 * i);
       da_append(&nums4, 4 * i);
    }

    long sum1, sum2, sum3, sum4;
    sum1 = sum2 = sum3 = sum4 = 0;
    da_foreach(&nums1, int, n) {
        sum1+= (long)*n;
    }
    da_foreach(&nums2, int, n) {
        sum2+= (long)*n;
    }
    da_foreach(&nums3, int, n) {
        sum3+= (long)*n;
    }
    da_foreach(&nums4, int, n) {
        sum4+= (long)*n;
    }

    // printf("%ld %ld %zu %zu %zu %d\n", (long)500000*999999, sum, sizeof(int), nums.count, nums.capacity, nums.items[5000]);
    printf("%ld %ld %ld %ld\n", sum1, sum2, sum3, sum4);

    // free_da_arena();
    arena_free(&new_arena);

    return 0;
}

int main_new(void)
{
    return 0;
}
