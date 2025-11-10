#include <stdlib.h>

int main(void)
{
    char *a = malloc(1);
    a++;
    free(a);
    return 0;
}