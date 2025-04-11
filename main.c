#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "demonslayer.h"

// Global game state
Player **players;
Demon **demons;
int demon_count = 0;
int player_count = 0;
int game_running = 1;
time_t start_time;

Cell generate_random_location() {
    Cell loc;
    loc.x = rand() % GRID_WIDTH;
    loc.y = rand() % GRID_HEIGHT;
    return loc;
}

int calculate_strength(int base) {
    // Adjusted for balance: slower growth
    return base + (rand() % 3);
}

void remove_player(Player *player, Player **players, int *player_count) {
    player->active = 0;
    for (int i = 0; i < *player_count; i++) {
        if (players[i] == player) {
            for (int j = i; j < *player_count - 1; j++) {
                players[j] = players[j + 1];
            }
            (*player_count)--;
            printf("Removed player %s. Players left: %d\n", player->name, *player_count);
            break;
        }
    }
}

void remove_demon(Demon *demon, Demon **demons, int *demon_count) {
    demon->active = 0;
    for (int i = 0; i < *demon_count; i++) {
        if (demons[i] == demon) {
            for (int j = i; j < *demon_count - 1; j++) {
                demons[j] = demons[j + 1];
            }
            (*demon_count)--;
            printf("Removed demon %s. Demons left: %d\n", demon->name, *demon_count);
            break;
        }
    }
}

void *player(void *properties) {
    Player *p = (Player *)properties;
    while (p->active && game_running) {
        // Movement strategy
        Cell new_loc = p->location;
        switch (p->strategy) {
            case RANDOM_WALK:
                new_loc.x += (rand() % 3) - 1;
                new_loc.y += (rand() % 3) - 1;
                break;
            case DEMON_SEEKING:
                int min_dist = GRID_WIDTH * GRID_HEIGHT;
                Cell target = p->location;
                for (int i = 0; i < demon_count; i++) {
                    if (!demons[i]->active) continue;
                    int dx = demons[i]->location.x - p->location.x;
                    int dy = demons[i]->location.y - p->location.y;
                    int dist = dx * dx + dy * dy;
                    if (dist < min_dist && dist > 0) {
                        min_dist = dist;
                        target = demons[i]->location;
                    }
                }
                if (target.x > p->location.x) new_loc.x++;
                else if (target.x < p->location.x) new_loc.x--;
                if (target.y > p->location.y) new_loc.y++;
                else if (target.y < p->location.y) new_loc.y--;
                break;
            case STRENGTH_CONTROL:
                int max_score = 0;
                Cell best_loc = p->location;
                for (int dx = -2; dx <= 2; dx++) {
                    for (int dy = -2; dy <= 2; dy++) {
                        int tx = p->location.x + dx;
                        int ty = p->location.y + dy;
                        if (tx < 0 || tx >= GRID_WIDTH || ty < 0 || ty >= GRID_HEIGHT) continue;
                        int score = 0;
                        for (int i = 0; i < demon_count; i++) {
                            if (!demons[i]->active) continue;
                            if (demons[i]->location.x == tx && demons[i]->location.y == ty) {
                                score += demons[i]->score;
                            }
                        }
                        if (score > max_score) {
                            max_score = score;
                            best_loc.x = tx;
                            best_loc.y = ty;
                        }
                    }
                }
                new_loc = best_loc;
                break;
        }

        // Ensure new location is within bounds
        new_loc.x = new_loc.x < 0 ? 0 : (new_loc.x >= GRID_WIDTH ? GRID_WIDTH - 1 : new_loc.x);
        new_loc.y = new_loc.y < 0 ? 0 : (new_loc.y >= GRID_HEIGHT ? GRID_HEIGHT - 1 : new_loc.y);
        p->location = new_loc;

        // Combat resolution
        for (int i = 0; i < demon_count; i++) {
            if (!demons[i]->active) continue;
            if (demons[i]->location.x == p->location.x && demons[i]->location.y == p->location.y) {
                if (p->score >= demons[i]->score) { // Changed to >= to favor players slightly
                    p->score += demons[i]->score;
                    printf("%s defeated %s! New score: %d\n", p->name, demons[i]->name, p->score);
                    remove_demon(demons[i], demons, &demon_count);
                } else {
                    demons[i]->score += p->score;
                    printf("%s was defeated by %s!\n", p->name, demons[i]->name);
                    remove_player(p, players, &player_count);
                    return NULL;
                }
            }
        }

        usleep(200000); // Slower movement for balance
    }
    return NULL;
}

void *demon(void *properties) {
    Demon *d = (Demon *)properties;
    int time_alive = 0;
    while (d->active && game_running) {
        // Strength grows over time
        time_alive++;
        d->score = calculate_strength(8 + time_alive / 5); // Start stronger, grow faster

        // Random movement
        Cell new_loc = d->location;
        new_loc.x += (rand() % 3) - 1;
        new_loc.y += (rand() % 3) - 1;
        new_loc.x = new_loc.x < 0 ? 0 : (new_loc.x >= GRID_WIDTH ? GRID_WIDTH - 1 : new_loc.x);
        new_loc.y = new_loc.y < 0 ? 0 : (new_loc.y >= GRID_HEIGHT ? GRID_HEIGHT - 1 : new_loc.y);
        d->location = new_loc;

        // Combat resolution
        for (int i = 0; i < player_count; i++) {
            if (!players[i]->active) continue;
            if (players[i]->location.x == d->location.x && players[i]->location.y == d->location.y) {
                if (d->score > players[i]->score) {
                    d->score += players[i]->score;
                    printf("%s defeated %s! New score: %d\n", d->name, players[i]->name, d->score);
                    remove_player(players[i], players, &player_count);
                } else {
                    players[i]->score += d->score;
                    printf("%s was defeated by %s!\n", d->name, players[i]->name);
                    remove_demon(d, demons, &demon_count);
                    return NULL;
                }
            }
        }

        usleep(300000); // Slower than players
    }
    return NULL;
}

void *player_generator(void *arg) {
    int player_id = 0;
    // Spawn initial players
    for (int i = 0; i < 3 && game_running; i++) { // Start with 3 players
        if (player_count < MAX_PLAYERS) {
            Player *p = malloc(sizeof(Player));
            snprintf(p->name, 15, "Player%d", player_id++);
            p->score = calculate_strength(10);
            p->location = generate_random_location();
            p->strategy = (Strategy)(rand() % 3);
            p->active = 1;
            players[player_count] = p;
            pthread_create(&p->thread, NULL, player, p);
            player_count++;
            printf("Spawned %s. Players: %d\n", p->name, player_count);
        }
        usleep(500000); // Small delay between initial spawns
    }
    // Continue spawning
    while (game_running) {
        if (player_count < MAX_PLAYERS) {
            Player *p = malloc(sizeof(Player));
            snprintf(p->name, 15, "Player%d", player_id++);
            p->score = calculate_strength(10);
            p->location = generate_random_location();
            p->strategy = (Strategy)(rand() % 3);
            p->active = 1;
            players[player_count] = p;
            pthread_create(&p->thread, NULL, player, p);
            player_count++;
            printf("Spawned %s. Players: %d\n", p->name, player_count);
        }
        sleep(3); // Spawn every 3 seconds
    }
    return NULL;
}

void *demon_generator(void *arg) {
    int demon_id = 0;
    // Spawn initial demons
    for (int i = 0; i < 5 && game_running; i++) { // Start with 5 demons
        if (demon_count < MAX_DEMONS) {
            Demon *d = malloc(sizeof(Demon));
            snprintf(d->name, 15, "Demon%d", demon_id++);
            d->score = calculate_strength(8);
            d->location = generate_random_location();
            d->active = 1;
            demons[demon_count] = d;
            pthread_create(&d->thread, NULL, demon, d);
            demon_count++;
            printf("Spawned %s. Demons: %d\n", d->name, demon_count);
        }
        usleep(500000); // Small delay between initial spawns
    }
    // Continue spawning
    while (game_running) {
        if (demon_count < MAX_DEMONS) {
            Demon *d = malloc(sizeof(Demon));
            snprintf(d->name, 15, "Demon%d", demon_id++);
            d->score = calculate_strength(8);
            d->location = generate_random_location();
            d->active = 1;
            demons[demon_count] = d;
            pthread_create(&d->thread, NULL, demon, d);
            demon_count++;
            printf("Spawned %s. Demons: %d\n", d->name, demon_count);
        }
        sleep(2); // Spawn every 2 seconds (faster)
    }
    return NULL;
}

void update_scoreboard(Player **players, int player_count) {
    printf("\n=== Scoreboard ===\n");
    for (int i = 0; i < player_count; i++) {
        if (players[i]->active) {
            printf("%s: %d\n", players[i]->name, players[i]->score);
        }
    }
    printf("Players: %d, Demons: %d\n", player_count, demon_count);
    printf("==================\n");
}

int main() {
    srand(time(NULL));
    start_time = time(NULL);

    // Initialize game state
    players = malloc(sizeof(Player *) * MAX_PLAYERS);
    demons = malloc(sizeof(Demon *) * MAX_DEMONS);
    for (int i = 0; i < MAX_PLAYERS; i++) players[i] = NULL;
    for (int i = 0; i < MAX_DEMONS; i++) demons[i] = NULL;

    init_view();

    // Create generator threads
    pthread_t player_gen, demon_gen;
    pthread_create(&player_gen, NULL, player_generator, NULL);
    pthread_create(&demon_gen, NULL, demon_generator, NULL);

    // Main game loop
    while (game_running) {
        update_view(players, player_count, demons, demon_count);
        update_scoreboard(players, player_count);

        // Check for SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                game_running = 0;
            }
        }

        // Game termination conditions
        time_t current_time = time(NULL);
        if (current_time - start_time >= MIN_GAME_TIME) {
            if (player_count == 0 && demon_count > 0) {
                printf("All players defeated! Demons win!\n");
                game_running = 0;
            } else if (demon_count == 0 && player_count > 0) {
                printf("All demons defeated! Players win!\n");
                game_running = 0;
            }
        }

        usleep(16666); // ~60 FPS
    }

    // Cleanup
    for (int i = 0; i < player_count; i++) {
        if (players[i]) {
            players[i]->active = 0;
            pthread_join(players[i]->thread, NULL);
            free(players[i]);
        }
    }
    for (int i = 0; i < demon_count; i++) {
        if (demons[i]) {
            demons[i]->active = 0;
            pthread_join(demons[i]->thread, NULL);
            free(demons[i]);
        }
    }
    free(players);
    free(demons);
    close_view();
    return 0;
}
