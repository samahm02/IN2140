// Nodvendige biblioteker aa inkludere for kjoering av diverse funksjoner
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    
    char *inputStreng = argv[argc-2];
    //lager et char array med legnde av input setning + 1 for nullbyte
    int lengde = strlen(inputStreng);
    char *ferdigSetning = malloc(lengde+1);
    //eksplisitt legger inn nullbyte i siste indeks
    ferdigSetning[lengde] = '\0';

    

    // sjekker om inputSetning for vokaler og bytter de ut
    for(int i = 0; i < lengde; i++){
        if(inputStreng[i] == 'a' || inputStreng[i] == 'e' || inputStreng[i] == 'i' || inputStreng[i] == 'o' || inputStreng[i] == 'u' || inputStreng[i] == 'y'){
              ferdigSetning[i] = *argv[argc-1];
        }
        else{
            ferdigSetning[i] = inputStreng[i];
        }
    }

    printf("%s\n", ferdigSetning);
    free(ferdigSetning);
    return 0;
}
