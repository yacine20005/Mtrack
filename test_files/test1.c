#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    /* Test malloc */
    char *a = malloc(10);
    printf("malloc: %p\n", a);
    
    /* Test calloc */
    int *b = calloc(5, sizeof(int));
    printf("calloc: %p\n", b);
    
    /* Test realloc */
    a = realloc(a, 20);
    printf("realloc: %p\n", a);
    
    /* Test free */
    free(a);
    free(b);
    
    return 0;
}
