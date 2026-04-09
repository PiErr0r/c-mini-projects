#include <stdio.h>
#include "arena.h"

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Nums;

int main_da(void)
{
    // test with dynamic array from helpers and try to build B-tree
    Nums nums = {0};

    for (size_t i = 0; i < 1000000; ++i) {
       da_append(&nums, i);
    }

    long sum = 0;
    da_foreach(&nums, int, n) {
        // printf("%d ", *n);
        sum += (long)*n;
    }

    printf("%ld %ld %zu %zu %zu %d\n", (long)500000*999999, sum, sizeof(int), nums.count, nums.capacity, nums.items[5000]);

    return 0;
}

int main(void)
{
    return 0;
}
