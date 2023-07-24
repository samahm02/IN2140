#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int stringsum(char *s)
{
    // Vi tar med 1 verdi eksta på starten, slik at vi kan finne hvilket nummer bokstven er i alfabetet
    int BokstavVerdiStart = 96;
    int BokstavVerdiSlutt = 122;

    int sum = 0;
    int lengde = strlen(s);
    int i = 0;
    while (i < lengde)
    {
        char bokstav = tolower(s[i]);
        // Sjekker om indeksen er en bokstav
        if (bokstav > BokstavVerdiStart && bokstav <= BokstavVerdiSlutt)
        {
            int bokstavVerdi = bokstav;
            sum += (bokstavVerdi - BokstavVerdiStart);
            i++;
        }
        // Hvis det er mellomrom hopper vi til neste indeks
        else if (isspace(s[i]))
        {
            i++;
            continue;
        }
        // Her har vi funnert noe annet enn en bokstav
        else
        {
            return -1;
        }
    }
    return sum;
}

int distance_between(char *s, char c)
{
    char *forste = strchr(s, c);
    char *siste = strrchr(s, c);

    if (!forste)
    {
        return -1;
    }
    return siste - forste;
}

char *string_between(char *s, char c){
    int forskjell = distance_between(s, c);
    char *mellom = malloc(forskjell+1);
    while (*s && *s != c){
        s++;
    }
    if(forskjell == 0){
        free(mellom);
        return strdup("");
    }
    if (forskjell == -1){
        free(mellom);
        return NULL;
    }
    for (int i = 1; i < (forskjell); i++)
    {
        mellom[i - 1] = s[i];
    }
    mellom[forskjell-1] = 0;
    return mellom;
}
 

int stringsum2(char *s, int *res)
{
    int sum = stringsum(s);
    *res = sum;
    return -1;
}
