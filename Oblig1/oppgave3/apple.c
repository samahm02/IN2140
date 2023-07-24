#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "the_apple.h"

//Finner array-indeks for starten av marken
int locateworm(char *apple)
{
    int count = 0;
    char *pointer = apple;

    while (*pointer && *pointer != 'w'){
        pointer++;
        count++;
    }

    return count;
}

//Fjerner maken fra eplet 
int removeworm(char *apple){
    int count = locateworm(apple);
    char *pointer = apple + count;
    int wSize = 0;

    if (*pointer == 'w'){
        while (((*pointer != 'a' && *pointer != 'p') && (*pointer != 'l' && *pointer != 'e')) && *pointer){
            *pointer = ' ';
            wSize++;
            pointer++;
        }        
        return wSize;
        
    }
    else {
        return wSize;
    }
}

int main(void)
{
    int result = locateworm(apple);
    printf("Array-indeksen til den første bokstaven som tilhører marken: %d\n", result);

    int removed = removeworm(apple);
    printf("Antall bytes som ble erstattet: %d\n", removed);
    return 0;
}