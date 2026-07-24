#include <stdio.h>
#include "qosa_def.h"

int main(void)
{
    int *arr = NULL;
    int i;
    int size = 10;

    // 1. Allocate memory
    arr = (int*)qosa_malloc(size * sizeof(int));
    if (arr == QOSA_NULL) {
        printf("memory allocation failed!\n");
        return -1;
    }
    printf("memory allocation succeeded, size: %d ints\n", size);

    // 2. Use memory - write data
    for (i = 0; i < size; i++) {
        arr[i] = i * 10;
    }

    // 3. Use memory - read data
    printf("array contents: ");
    for (i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 4. Free memory
    qosa_free(arr);
    printf("memory released\n");

    return 0;
}