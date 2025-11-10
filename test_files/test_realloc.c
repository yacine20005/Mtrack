#include <stdlib.h>
#include <string.h>

int main(void)
{
    char *ptr1, *ptr2;

    /* Erreur 7: Realloc sur un pointeur décalé */
    ptr1 = malloc(20);
    
    ptr2 = realloc(ptr1, 40); // ERREUR: realloc sur pointeur invalide
    free(ptr2);

    return 0;
}
