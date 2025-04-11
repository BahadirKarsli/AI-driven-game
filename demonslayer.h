#ifndef DEMONSLAYER_H
#define DEMONSLAYER_H

#include <SDL2/SDL.h>
#include <pthread.h>

#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define MAX_PLAYERS 10
#define MAX_DEMONS 20
#define CELL_SIZE 30
#define MIN_GAME_TIME 30 // Minimum game time in seconds before allowing win

typedef struct cell {
    int x;
    int y;
} Cell;

typedef enum strategy {
    RANDOM_WALK,
    DEMON_SEEKING,
    STRENGTH_CONTROL
} Strategy;

typedef struct player {
    char name[15];
    int score;
    Cell location;
    Strategy strategy;
    pthread_t thread;
    int active;
} Player;

typedef struct demon {
    char name[15];
    int score;
    Cell location;
    pthread_t thread;
    int active;
} Demon;

// View functions
void init_view();
void update_view(Player **players, int player_count, Demon **demons, int demon_count);
void close_view();
void update_scoreboard(Player **players, int player_count);

// Thread functions
void *player(void *properties);
void *demon(void *properties);
void *player_generator(void *arg);
void *demon_generator(void *arg);

// Utility functions
Cell generate_random_location();
int calculate_strength(int base);
void remove_player(Player *player, Player **players, int *player_count);
void remove_demon(Demon *demon, Demon **demons, int *demon_count);

#endif