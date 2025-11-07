
// for winodws mac and linux

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <wchar.h>
#include <locale.h>
#endif

#define MAP_WIDTH     32
#define MAP_HEIGHT    16
#define FOV           3.14159f / 4.0f
#define DEPTH         16.0f
#define SCREEN_WIDTH  120
#define SCREEN_HEIGHT 40


char map[] =
    "11111111111111111111111111111111"
    "10000000000000000000000000000001"
    "10111111111111111100000000000001"
    "10000000000000000100000000000001"
    "10111100011110000100000000000001"
    "10000100010010000100000000000001"
    "10000100010010000111111111100001"
    "10000100010010000000000000100001"
    "10000111110011111111111110100001"
    "10000000000000000000000000100001"
    "10000000000000000000000000100001"
    "10001111111111111111111110100001"
    "10001000000000000000000000100001"
    "10001000000000000000000000100001"
    "10001000000000000000000000100001"
    "11111111111111111111111111111111";

// input keys
char forwardKey = 'w';
char leftKey = 'a';
char backKey = 's';
char rightKey = 'd';
char fireKey = ' ';
char menuKey = 'm';

int hardness = 5;
float playerX = 2.0f, playerY = 2.0f;
float playerA = 0.0f;

struct Enemy {
    float xCords;
    float yCords;
    int health;
};

struct Enemy spawnEnemy() {
    struct Enemy e;
    e.xCords = rand() % 32;
    e.yCords = rand() % 16;
    e.health = 10;
    return e;
}

void fireWeapon() {}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[H\033[J");
#endif
}

// cross platfor replacemnts for windows.h features
#ifndef _WIN32
int _kbhit(void) {
    struct termios oldt, newt;
    int ch, oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char _getch(void) {
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

int main() {
//styart the program and input if ready to luanch

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    char startLetter;
    printf("Are you ready to start? Y/n - ");
    scanf(" %c", &startLetter);

    wchar_t *screen = malloc(sizeof(wchar_t) * SCREEN_WIDTH * SCREEN_HEIGHT);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        screen[i] = L' ';

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = { SCREEN_WIDTH, SCREEN_HEIGHT };
    SetConsoleScreenBufferSize(hConsole, bufferSize);
    SetConsoleActiveScreenBuffer(hConsole);
#endif

    if (startLetter == 'Y' || startLetter == 'y') {
        //start raycasting
        while (1) {
            if (_kbhit()) {
                char key = _getch();
                if (key == leftKey) playerA -= 0.1f;
                if (key == rightKey) playerA += 0.1f;
                if (key == forwardKey) {
                    playerX += sinf(playerA) * 0.5f;
                    playerY += cosf(playerA) * 0.5f;
                    if (map[(int)playerY * MAP_WIDTH + (int)playerX] == '1') {
                        playerX -= sinf(playerA) * 0.5f;
                        playerY -= cosf(playerA) * 0.5f;
                    }
                }
                if (key == backKey) {
                    playerX -= sinf(playerA) * 0.5f;
                    playerY -= cosf(playerA) * 0.5f;
                    if (map[(int)playerY * MAP_WIDTH + (int)playerX] == '1') {
                        playerX += sinf(playerA) * 0.5f;
                        playerY += cosf(playerA) * 0.5f;
                    }
                }
            }

            for (int x = 0; x < SCREEN_WIDTH; x++) {
                float rayAngle = (playerA - FOV / 2.0f) + ((float)x / (float)SCREEN_WIDTH) * FOV;
                float distanceToWall = 0;
                int hitWall = 0;
                float eyeX = sinf(rayAngle);
                float eyeY = cosf(rayAngle);

                while (!hitWall && distanceToWall < DEPTH) {
                    distanceToWall += 0.1f;
                    int testX = (int)(playerX + eyeX * distanceToWall);
                    int testY = (int)(playerY + eyeY * distanceToWall);

                    if (testX < 0 || testX >= MAP_WIDTH || testY < 0 || testY >= MAP_HEIGHT) {
                        hitWall = 1;
                        distanceToWall = DEPTH;
                    } else if (map[testY * MAP_WIDTH + testX] == '1') {
                        hitWall = 1;
                    }
                }

                int ceiling = (float)(SCREEN_HEIGHT / 2.0f) - SCREEN_HEIGHT / ((float)distanceToWall);
                int floor = SCREEN_HEIGHT - ceiling;

                for (int y = 0; y < SCREEN_HEIGHT; y++) {
                    //walls 
                    int idx = y * SCREEN_WIDTH + x;
                    if (y < ceiling) screen[idx] = L' ';
                    else if (y >= ceiling && y <= floor) {
                        //distance = diffrent color
                        if (distanceToWall <= DEPTH / 4.0f)       screen[idx] = 0x2588;
                        else if (distanceToWall < DEPTH / 3.0f)  screen[idx] = 0x2593;
                        else if (distanceToWall < DEPTH / 2.0f)  screen[idx] = 0x2592;
                        else if (distanceToWall < DEPTH)         screen[idx] = 0x2591;
                        else                                     screen[idx] = L' ';        // to far nothing gets rendered
                    } else {
                        //floor
                        float b = 1.0f - (((float)y - SCREEN_HEIGHT / 2.0f) / (SCREEN_HEIGHT / 2.0f));
                        if (b < 0.25)      screen[idx] = '#';
                        else if (b < 0.5)  screen[idx] = 'x';
                        else if (b < 0.75) screen[idx] = '.';
                        else if (b < 0.9)  screen[idx] = '-';
                        else               screen[idx] = ' ';
                    }
                }
            }

            screen[(SCREEN_HEIGHT / 2) * SCREEN_WIDTH + (SCREEN_WIDTH / 2)] = L'+';

#ifdef _WIN32           //write the frame to the console
            DWORD dwBytesWritten = 0;
            WriteConsoleOutputCharacterW(hConsole, screen, SCREEN_WIDTH * SCREEN_HEIGHT, (COORD){0,0}, &dwBytesWritten);
#else
            clearScreen();
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    putwchar(screen[y * SCREEN_WIDTH + x]);
                }
                putwchar(L'\n');
            }
#endif


// sleep 30 ms inbetween frames to make it smoother locked at 33 fps
#ifdef _WIN32
            Sleep(30);
#else
            usleep(30000);
#endif
        }
    }

    free(screen);
    return 0;
}
