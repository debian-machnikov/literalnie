#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

typedef struct {
    char* word;
    int* lettersCount;
    bool guessed;
} WordMetadata;

typedef struct {
    WordMetadata* wordlist;
    int recordCount;
} WordlistMetadata;

typedef struct {
    bool* correct;
    char* misplaced;
    bool* missed;
} LetterPlacementInfo;

int countNoOfRecords(FILE* fp) {
    int noOfRecords = 0;
    if (!fp) {
        fprintf(stderr, "Nie można odczytać pliku.");
        return noOfRecords;
    }

    char* buffer = (char*)malloc(10);

    while (fscanf(fp, "%s", buffer) != EOF) {
        if (strlen(buffer) == 5) {
            noOfRecords++;
        }
    }
    free(buffer);

    fclose(fp);
    return noOfRecords;
}

WordlistMetadata readWordlist(const char* filename) {
    FILE* fp = fopen(filename, "r");
    WordlistMetadata data;

    if (!fp) {
        fprintf(stderr, "Nie można odczytać pliku: %s", filename);
        return data;
    }

    data.recordCount = countNoOfRecords(fp);
    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Nie można odczytać pliku: %s", filename);
        return data;
    }
    data.wordlist = (WordMetadata*)malloc(data.recordCount * sizeof(WordMetadata));

    char* buffer = (char*)malloc(10);
    int i = 0;
    while (i < data.recordCount && fscanf(fp, "%s", buffer) != EOF) {
        data.wordlist[i].word = (char*)malloc(6);
        data.wordlist[i].guessed = false;
        data.wordlist[i].lettersCount = (int*)calloc(26, sizeof(int));

        if (strlen(buffer) == 5) {
            strcpy(data.wordlist[i].word, buffer);
            for (int j = 0; j < 5; j++) {
                int letter = data.wordlist[i].word[j]-'a';
                data.wordlist[i].lettersCount[letter]++;
            }
            i++;
        }
    }
    free(buffer);

    printf("Wczytano: %d rekordów.\n", data.recordCount);
    fclose(fp);
    return data;
}

int findGuess(const char* guess, WordlistMetadata data) {
    int left = 0;
    int right = data.recordCount - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(data.wordlist[mid].word, guess);

        if (cmp == 0)
            return mid;
        else if (cmp < 0)
            left = mid+1;
        else
            right = mid-1;
    }

    return -1;
}

int findLetter(char guessedLetter, const char* drawn, WordlistMetadata data, LetterPlacementInfo letterPlacement) {
    for (int i = 0; i < 5; i++) {
        if (guessedLetter == drawn[i] && !letterPlacement.correct[i]) {
            return i;
        }
    }
    return -1;
}

char* drawWord(WordlistMetadata data) {
    char* draw = (char*)malloc(6);
    unsigned int seed = time(0);

    int randomIndex;
    const int maxConsecutiveDraws = 50;
    int drawCount = 0;

    do {
        drawCount++;
        randomIndex = rand_r(&seed) % data.recordCount;
    } while (data.wordlist[randomIndex].guessed == true && drawCount < maxConsecutiveDraws);

    printf("Wylosowany indeks: %d\n", randomIndex);
    strcpy(draw, data.wordlist[randomIndex].word);
    return draw;
}

void printFeedback(const char* guess, WordlistMetadata data, LetterPlacementInfo letterPlacement) {
    for (int i = 0; i < 5; i++) {
        if (letterPlacement.correct[i] == true) {
            printf(GREEN "%c" RESET, guess[i]);
        }
        else if (letterPlacement.misplaced[i] != '0') {
            printf(YELLOW "%c" RESET, guess[i]);
        }
        else {
            printf("%c",guess[i]);
        }
    }
    printf("\n");
}

bool isValidGuess(const char* guess, const char* drawn, WordlistMetadata data, LetterPlacementInfo letterPlacement, bool hardmode) {
    bool enforce = false;
    bool misplacedExists = false;
    bool* notices = (bool*)malloc(4);
    notices[0] = false;
    notices[1] = false;
    notices[2] = false;
    notices[3] = false;

    if (hardmode) {
        for (int i = 0; i < 5; i++) {
            misplacedExists = false;
            if (letterPlacement.missed[guess[i]-'a'] && !notices[0]) {
                printf("Nie można już używać liter, które zostały oznaczone jako niewystępujące w wyrazie!\n");
                enforce = true;
                notices[0] = true;
            }
            if (letterPlacement.correct[i] && !notices[1]) {
                printf("Znaki, które zostały odgadnięte na prawidłowym miejscu, muszą na nim pozostać!\n");
                enforce = true;
                notices[1] = true;
            }
            if (letterPlacement.misplaced[i] == guess[i] && !notices[2]) {
                printf("Znaki, które zostały odgadnięte na nieprawidłowym miejscu, nie mogą się już w tym miejscu znajdować!\n");
                enforce = true;
                notices[2] = true;
            }
            if (letterPlacement.misplaced[i] != '0' && notices[3]) {
                for (int j = 0; j < 5; j++) {
                    if (drawn[j] == letterPlacement.misplaced[i] && findLetter(letterPlacement.misplaced[i], guess, data, letterPlacement) != -1) {
                        misplacedExists = true;
                    }
                }
                if (!misplacedExists) {
                    printf("Znaki, które zostały odgadnięte na nieprawidłowym miejscu, muszą zostać użyte w następnych próbach!\n");
                    enforce = true;
                    notices[3] = true;
                }
            }
        }

        if (enforce) {
            return false;
        }
    }

    for (int i = 0; i < 5; i++) {
        if (guess[i] == drawn[i]) {
            letterPlacement.correct[i] = true;
            letterPlacement.misplaced[i] = '0';
            letterPlacement.missed[guess[i]-'a'] = false;
        }
        else {
            letterPlacement.correct[i] = false;
            letterPlacement.misplaced[i] = '0';
            letterPlacement.missed[guess[i]-'a'] = true;
        }
    }
    int* guessLettersCount = (int*)calloc(26,sizeof(int));
    for (int i = 0; i < 5; i++) {
        guessLettersCount[guess[i]-'a']++;
    }
    for (int i = 0; i < 5; i++) {
        if (guessLettersCount[guess[i]-'a'] == data.wordlist[findGuess(drawn[i], data)].lettersCount[guess[i]-'a']) {
            if (findLetter(guess[i], drawn, data, letterPlacement) != -1) {
                letterPlacement.correct[i] = false;
                letterPlacement.misplaced[i] = guess[i];
                letterPlacement.missed[guess[i]-'a'] = false;
            }
        }
        else if (guessLettersCount[guess[i]-'a'] > data.wordlist[findGuess(drawn[i], data)].lettersCount[guess[i]-'a'] && data.wordlist[findGuess(drawn[i], data)].lettersCount[guess[i]-'a'] != 0) {
            continue;
        }
    }

    printFeedback(guess, data, letterPlacement);

    return true;
}

bool game(WordlistMetadata data, bool hardmode) {
    char* guess = (char*)malloc(10);
    char* drawn = (char*)malloc(6);
    strcpy(drawn, drawWord(data));

    LetterPlacementInfo letterPlacement;
    letterPlacement.correct = (bool*)malloc(5);
    letterPlacement.missed = (bool*)malloc(26);
    letterPlacement.misplaced = (char*)malloc(5);
    for (int i = 0; i < 26; i++) {
        if (i<5) {
            letterPlacement.correct[i] = false;
            letterPlacement.misplaced[i] = '0';
        }
        letterPlacement.missed[i] = false;
    }

    if (hardmode) {
        printf("Wybrano tryb trudny.\n");
    } else {
        printf("Wybrano tryb łatwy.\n");
    }

    for (int guesses = 6; guesses > 0; guesses--) {
        printf("Wpisz 5-literowe słowo. Masz jeszcze: %d prób.\n", guesses);
        scanf("%s", guess);

        if (strlen(guess) != 5 || findGuess(guess, data) == -1) {
            guesses++;
            printf("Podane przez ciebie słowo nie ma 5 liter lub nie znajduje się w bazie programu. Spróbuj jeszcze raz!\n");
            continue;
        }

        if (strcmp(guess, drawn) == 0) {
            printf("Gratulacje! Odgadłeś prawidłowe słowo.\n");
            data.wordlist[findGuess(guess, data)].guessed = true;
            return true;
        }

        if (!isValidGuess(guess, drawn, data, letterPlacement, hardmode)) {
            if (hardmode) {
                guesses++;
                printf("Zastosuj się do podanych wskazówek!\n");
            }
            else {
                printFeedback(guess, data, letterPlacement);
            }
        }
    }

    printf("Koniec gry!\n");
    return false;
}


void printWordlist(WordlistMetadata data) {
    if (data.recordCount <= 0) {
        return;
    }
    printf("Baza zapisanych w programie słów (ilość rekordów: %d):\n",data.recordCount);
    while (data.wordlist->word) {
        printf("Słowo: %s, Czy odgadnięte: %s\n", data.wordlist->word, data.wordlist->guessed ? "tak" : "nie");
        data.wordlist++;
    }
}



void freeWordlistMetadata(WordlistMetadata wordlistMetadata) {
    for (int i=0; i < wordlistMetadata.recordCount; i++) {
        WordMetadata* wordMetadata = &wordlistMetadata.wordlist[i];

        if (wordMetadata->word != NULL) {
            free(wordMetadata->word);
        }
        if (wordMetadata->lettersCount != NULL) {
            free(wordMetadata->lettersCount);
        }
    }
    if (wordlistMetadata.wordlist != NULL) {
        free(wordlistMetadata.wordlist);
    }
}

void clearStdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void initiateGame(WordlistMetadata data) {
    char mode;
    bool continueGame;
    printf("***LITERALNIE***\n");

    do {
        continueGame = false;
        printf("Wybierz tryb gry: \n- wpisz \"l\" by wybrać tryb łatwy\n- wpisz \"t\" by wybrać tryb trudny\n: ");
        scanf("%c", &mode);

        switch (mode) {
            case 'l':
                if (!game(data, false))
                    return;
                break;
            case 't':
                if (!game(data, true))
                    return;
                break;
            case 'w':
                printWordlist(data);
                break;
            default:
                printf("Tu będzie pomoc.\n");
        }

        fflush(stdin);
        clearStdin();
        printf("Czy chcesz kontynuować grę? (t/n): ");
        scanf("%c", &mode);
        if (mode == 't') {
            continueGame = true;
        }
        fflush(stdin);
        clearStdin();
    } while (continueGame);
}


int main(void) {
    setlocale(LC_ALL, "UTF-8.pl_PL");
    WordlistMetadata data = readWordlist("slownik.txt");
    //printWordlist(data);
    initiateGame(data);
    freeWordlistMetadata(data);
    return 0;
}