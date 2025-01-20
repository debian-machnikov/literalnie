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

// Funkcje do zwalniania pamieci zajmowanej przez struktury danych
void freeWordlistMetadata(WordlistMetadata* wordlistMetadata);
void freeLetterPlacementInfo(LetterPlacementInfo letterPlacement);

// Funkcje zwiazane z odczytem z pliku
int countNoOfRecords(FILE* fp);
WordlistMetadata readWordlist(const char* filename);
void printWordlist(WordlistMetadata data);

// Funkcje pomocnicze obslugujace mechanike gry
int findGuess(const char* guess, WordlistMetadata data);
int findLetter(char guessedLetter, const char* drawn, LetterPlacementInfo letterPlacement);
char* drawWord(WordlistMetadata data);

// Glowne funkcje obslugujace dzialanie gry i interakcje z graczem
void printFeedback(const char* guess, LetterPlacementInfo letterPlacement);
bool isValidGuess(const char* guess, const char* drawn, WordlistMetadata data, LetterPlacementInfo letterPlacement, bool hardmode);
bool game(WordlistMetadata data, bool hardmode);
void initiateGame(WordlistMetadata data);

// Inne funkcje pomocnicze
void clearStdin();



// Funkcja glowna
int main(void) {
    setlocale(LC_ALL, "UTF-8.pl_PL");
    WordlistMetadata data = readWordlist("slownik.txt");
    //printWordlist(data);
    initiateGame(data);
    freeWordlistMetadata(&data);
    return 0;
}


// DEFINICJE ZADEKLAROWANYCH WYZEJ FUNKCJI

void freeWordlistMetadata(WordlistMetadata* wordlistMetadata) {
    for (int i=0; i < (*wordlistMetadata).recordCount; i++) {
        WordMetadata* wordMetadata = &(*wordlistMetadata).wordlist[i];

        if (wordMetadata->word != NULL) {
            free(wordMetadata->word);
            wordMetadata->word = NULL;
        }
        if (wordMetadata->lettersCount != NULL) {
            free(wordMetadata->lettersCount);
            wordMetadata->lettersCount = NULL;
        }
    }
    if ((*wordlistMetadata).wordlist != NULL) {
        free((*wordlistMetadata).wordlist);
        (*wordlistMetadata).wordlist = NULL;
    }
}

void freeLetterPlacementInfo(const LetterPlacementInfo letterPlacement) {
    free(letterPlacement.correct);
    free(letterPlacement.misplaced);
    free(letterPlacement.missed);
}

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
    data.wordlist = NULL;
    data.recordCount = 0;

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

int findGuess(const char* guess, WordlistMetadata data) {
    int left = 0;
    int right = data.recordCount - 1;

    int mid=0, cmp=0;

    while (left <= right) {
        mid = left + (right - left) / 2;
        cmp = strcmp(data.wordlist[mid].word, guess);

        if (cmp == 0)
            return mid;
        else if (cmp < 0)
            left = mid+1;
        else
            right = mid-1;
    }

    return -1;
}

int findLetter(const char guessedLetter, const char* drawn, const LetterPlacementInfo letterPlacement) {
    for (int i = 0; i < 5; i++) {
        if (guessedLetter == drawn[i] && !letterPlacement.correct[i]) {
            return i;
        }
    }
    return -1;
}

char* drawWord(const WordlistMetadata data) {
    char* draw = (char*)malloc(6);
    unsigned int seed = time(0);

    int randomIndex;
    const int maxConsecutiveDraws = 50;
    int drawCount = 0;

    do {
        drawCount++;
        randomIndex = rand_r(&seed) % data.recordCount;
    } while (data.wordlist[randomIndex].guessed == true && drawCount < maxConsecutiveDraws);

    //DEBUG
    randomIndex = 4245;

    printf("Wylosowany indeks: %d\n", randomIndex);
    strcpy(draw, data.wordlist[randomIndex].word);
    return draw;
}

void printFeedback(const char* guess, const LetterPlacementInfo letterPlacement) {
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

bool isValidGuess(const char* guess, const char* drawn, const WordlistMetadata data, const LetterPlacementInfo letterPlacement, const bool hardmode) {
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
                if (guess[i] != drawn[i]) {
                    printf("Znaki, które zostały odgadnięte na prawidłowym miejscu, muszą na nim pozostać!\n");
                    enforce = true;
                    notices[1] = true;
                }
            }
            if (letterPlacement.misplaced[i] == guess[i] && !notices[2]) {
                printf("Znaki, które zostały odgadnięte na nieprawidłowym miejscu, nie mogą się już w tym miejscu znajdować!\n");
                enforce = true;
                notices[2] = true;
            }
            if (letterPlacement.misplaced[i] != '0' && !notices[3]) {
                for (int j = 0; j < 5; j++) {
                    if (drawn[j] == letterPlacement.misplaced[i] && findLetter(letterPlacement.misplaced[i], guess, letterPlacement) != -1) {
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
            free(notices);
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
        const int drawnLetterCount = data.wordlist[findGuess(drawn, data)].lettersCount[guess[i]-'a'];
        const int condition1 = (guessLettersCount[guess[i]-'a'] == drawnLetterCount);
        const int condition2 = (guessLettersCount[guess[i]-'a'] > drawnLetterCount && drawnLetterCount > 0);
        const int condition3 = (guessLettersCount[guess[i]-'a'] < drawnLetterCount && guessLettersCount[guess[i]-'a'] > 0);

        if ((condition1 + condition2 + condition3) == 1) {
            if (findLetter(guess[i], drawn, letterPlacement) != -1 && !letterPlacement.correct[i]) {
                letterPlacement.correct[i] = false;
                letterPlacement.misplaced[i] = guess[i];
                letterPlacement.missed[guess[i]-'a'] = false;
                guessLettersCount[guess[i]-'a'] = condition1 ? guessLettersCount[guess[i]-'a'] : 0;
            }
        }
    }
    free(guessLettersCount);
    free(notices);

    printFeedback(guess, letterPlacement);

    return true;
}

bool game(const WordlistMetadata data, const bool hardmode) {
    char* guess = (char*)malloc(10);
    char* drawn = (char*)malloc(6);
    strcpy(drawn, drawWord(data));
    if (drawn == NULL) {
        return false;
    }

    LetterPlacementInfo letterPlacement;
    letterPlacement.correct = (bool*)malloc(5);
    letterPlacement.missed = (bool*)malloc(26);
    letterPlacement.misplaced = (char*)malloc(5);
    if (letterPlacement.correct == NULL || letterPlacement.missed == NULL || letterPlacement.misplaced == NULL) {
        return false;
    }
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

        if (guess == NULL || strlen(guess) != 5 || findGuess(guess, data) == -1) {
            guesses++;
            printf("Podane przez ciebie słowo nie ma 5 liter lub nie znajduje się w bazie programu. Spróbuj jeszcze raz!\n");
            continue;
        }

        if (strcmp(guess, drawn) == 0) {
            printf("Gratulacje! Odgadłeś prawidłowe słowo.\n");
            data.wordlist[findGuess(guess, data)].guessed = true;
            freeLetterPlacementInfo(letterPlacement);
            return true;
        }

        if (!isValidGuess(guess, drawn, data, letterPlacement, hardmode)) {
            guesses++;
            printf("Zastosuj się do podanych wskazówek!\n");
        }
    }

    printf("Koniec gry!\n");
    freeLetterPlacementInfo(letterPlacement);
    return false;
}

void initiateGame(const WordlistMetadata data) {
    if (data.wordlist == NULL) {
        return;
    }

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
        //clearStdin();
        printf("Czy chcesz kontynuować grę? (t/n): ");
        scanf("%c", &mode);
        if (mode == 't') {
            continueGame = true;
        }
        fflush(stdin);
        //clearStdin();
    } while (continueGame);
}

void clearStdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}