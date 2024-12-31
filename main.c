// ===================================================================
// ||                      Politechnika Śląska                      ||
// ||                 Wydział Matematyki Stosowanej                 ||
// ||           Informatyka I st. niestacjonarne Semestr I          ||
// ||                   Projekt z Programowania I                   ||
// ||                      Data: 28.12.2024 r.                      ||
// ||                 Autor: Andrzej Machnik (316067)               ||
// ||                    Tytuł projektu: Literalnie                 ||
// ===================================================================
// Realizacja zadania 5 z edycji konkursu "Algorytmion" z 2023 roku pt. "LITERALNIE"
// LINK: https://algorytmion.ms.polsl.pl/storage/files/Zadania2023.pdf


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

// Prosta struktura danych przechowująca słowo oraz informację czy zostało ono odgadnięte.
typedef struct {
    char* word;
    bool guessed;
} Words;

typedef struct {
    Words* wordlist;
    int recordCount;
} WordsList;

// Deklaracje użytych funkcji programu
int countRecords(FILE* fp);
WordsList readWordlist(const char* filename);
int findGuessInWordlist(const char* guess, WordsList words);
int findLetterInWord(char letter, const char* word);
void displayHint(const char* guess, const char* drawn);
bool game(WordsList words, bool hardmode);
void initiateGame(WordsList words);
void printWordlist(WordsList words);
void freeWordlist(Words** wordlist);
void clearStdin();

// Funkcja Główna
int main(void) {
    WordsList words = readWordlist("slownik.txt");
    //printWordlist(words);
    initiateGame(words);
    freeWordlist(&words.wordlist);
    free(words.wordlist);
    return 0;
}


// Definicje funkcji

int countRecords(FILE* fp) {
    int recordCount = 0;
    char* buffer = (char*)malloc(10);
    //strcpy(buffer, "");

    while(fscanf(fp, "%s", buffer) != EOF) {
        if(strlen(buffer) == 5) {
            recordCount++;
            //printf("%s\n", buffer);
        }
    }
    free(buffer);

    fclose(fp);
    return recordCount;
}


WordsList readWordlist(const char* filename) {
    FILE* fp = fopen(filename, "r");
    WordsList words;

    if(!fp) {
        fprintf(stderr, "Blad odczytu pliku!\n");
    }

    words.recordCount = countRecords(fp);
    fp = fopen(filename, "r");
    words.wordlist = (Words*)malloc(words.recordCount*sizeof(words));

    char* buffer = (char*)malloc(10);
    int i = 0;
    while(i < words.recordCount && fscanf(fp, "%s", buffer) != EOF) {
        words.wordlist[i].guessed = false;
        words.wordlist[i].word = (char*)malloc(6);

        if(strlen(buffer) == 5) {
            strcpy(words.wordlist[i].word, buffer);
            i++;
        }
    }
    free(buffer);

    printf("Wczytano: %d rekordow.\n", words.recordCount);
    fclose(fp);
    return words;
}

// uzywam wyszukiwania binarnego, poniewaz z tresci zadania wiadomo, ze wyrazy w pliku sa posortowane alfabetycznie
int findGuessInWordlist(const char* guess, WordsList words) {
    int left = 0;
    int right = words.recordCount - 1;

    while(left <= right) {
        int mid = left + (right-left) / 2;
        int cmp = strcmp(words.wordlist[mid].word, guess);
        if(cmp == 0)
            return mid;
        else if(cmp < 0)
            left = mid+1;
        else
            right = mid-1;
    }

    return -1;
}

int findLetterInWord(char letter, const char* word) {
    for (int i=0;i<strlen(word);i++) {
        if (letter == word[i])
            return i;
    }
    return -1;
}

void displayHint(const char* guess, const char* drawn) {
    for (int i=0; i < strlen(guess); i++) {
        if (guess[i] == drawn[i])
            printf(GREEN "%c" RESET, guess[i]);
        else if (findLetterInWord(guess[i], drawn) != -1)
            printf(YELLOW "%c" RESET, guess[i]);
        else
            printf("%c", guess[i]);
    }
    printf("\n");
}

bool game(const WordsList words, const bool hardmode) {
    char* guess = (char*)malloc(10);
    const char* drawnWord = (char*)malloc(6);
    strcpy(drawnWord, words.wordlist[1].word);

    if(hardmode)
        printf("Wybrano tryb trudny.\n");
    else
        printf("Wybrano tryb latwy\n");

    for(int guesses=6; guesses > 0; guesses--) {
        printf("Wpisz 5-literowe slowo. Masz %d prob.\n", guesses);
        scanf("%s", guess);
        if(strlen(guess) != 5 || findGuessInWordlist(guess, words) == -1) {
            guesses++;
            printf("Podane przez ciebie slowo nie ma 5 liter lub nie znajduje sie w bazie\n");
            continue;
        }
        if(strcmp(guess, drawnWord) == 0) {
            printf("Gratulacje! Odgadles slowo.\n");
            return true;
        }
        else {
            displayHint(guess, drawnWord);
        }
    }
    printf("Koniec gry!\n");
    return false;
}

void initiateGame(const WordsList words) {
    char mode;
    bool continueGame;
    printf("***LITERALNIE***\n\n");

    do {
        continueGame=false;
        fflush(stdin); // clearing input buffer - in case of lack of compiler support for this function comment it and uncomment clearStdin() below
        //clearStdin();
        printf("Wybierz tryb gry: \n- wpisz \"l\" by wybrac tryb latwy\n- wpisz \"t\" by wybrac tryb trudny\n: ");
        scanf("%c", &mode);

        switch(mode) {
            case 'l':
                if(!game(words, false))
                    return;
                break;
            case 't':
                if(!game(words, true))
                    return;
                break;
            case 'w':
                printWordlist(words);
                break;
            default:
                printf("help");
        }
        fflush(stdin);
        //clearStdin();
        printf("Czy chcesz kontynuowac gre? (t/n): ");
        scanf("%c", &mode);
        if(mode == 't')
            continueGame=true;
    } while(continueGame);
}


void printWordlist(WordsList words) {
    printf("Baza slow zapisanych w bazie programu:\n");
    while(words.wordlist->word) {
        printf("Slowo: %s, odgadniete: %s\n", words.wordlist->word, words.wordlist->guessed ? "tak" : "nie");
        words.wordlist++;
    }
}


void freeWordlist(Words** wordlist) {
    while((*wordlist)->word) {
        free((*wordlist)->word);
        (*wordlist)->word = nullptr;
        (*wordlist)++;
    }
    free(*wordlist);
    *wordlist = nullptr;
}

void clearStdin() {
    int c;
    while( (c = getchar()) != '\n' && c != EOF) {}
}