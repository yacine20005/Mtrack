#include <stdlib.h>
#include <string.h>

int main(void)
{
    char *ptr1, *ptr2, *ptr3;

    /* Erreur 1: Double free */
    ptr1 = malloc(10);
    free(ptr1);
    free(ptr1); // ERREUR: double liberation

    /* Erreur 2: Free d'un pointeur invalide (décalé) */
    ptr1 = malloc(20);
    ptr1 += 5;
    free(ptr1); // ERREUR: free sur pointeur décalé

    /* Erreur 3: Realloc puis free du pointeur original */
    ptr1 = malloc(10);
    ptr2 = realloc(ptr1, 50);
    free(ptr1); // ERREUR: ptr1 peut être invalide après realloc
    free(ptr2);

    /* Erreur 4: Calloc puis débordement de buffer */
    // ptr1 = calloc(5, sizeof(char));
    // strcpy(ptr1, "Cette chaine est trop longue");  // ERREUR: débordement
    // free(ptr1);

    /* Erreur 5: Utilisation après free */
    // ptr1 = calloc(10, sizeof(char));
    // strcpy(ptr1, "test");
    // free(ptr1);
    // ptr1[0] = 'X';  // ERREUR: utilisation après free

    /* Erreur 6: Fuite mémoire */
    ptr1 = malloc(100);
    ptr2 = calloc(10, 10);
    // ptr3 = realloc(NULL, 50);
    // ERREUR: aucun free, fuite mémoire
    return 0;

    /* Erreur 7: Realloc sur un pointeur décalé */
    ptr1 = malloc(20);
    ptr1 += 3;
    ptr2 = realloc(ptr1, 40); // ERREUR: realloc sur pointeur invalide
    free(ptr2);

    return 0;
}
