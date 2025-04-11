#include "demonslayer.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void init_view() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("Demon Slayer",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE,
                              SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                      SDL_RENDERER_PRESENTVSYNC);
}

void draw_circle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if (w * w + h * h <= radius * radius) {
                SDL_RenderDrawPoint(renderer, x + w, y + h);
            }
        }
    }
}

void update_view(Player **players, int player_count, Demon **demons, int demon_count) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    for (int x = 0; x <= GRID_WIDTH * CELL_SIZE; x += CELL_SIZE) {
        SDL_RenderDrawLine(renderer, x, 0, x, GRID_HEIGHT * CELL_SIZE);
    }
    for (int y = 0; y <= GRID_HEIGHT * CELL_SIZE; y += CELL_SIZE) {
        SDL_RenderDrawLine(renderer, 0, y, GRID_WIDTH * CELL_SIZE, y);
    }

    for (int i = 0; i < demon_count; i++) {
        if (!demons[i]->active) continue;
        int radius = 5 + demons[i]->score / 5;
        int intensity = 100 + demons[i]->score * 5;
        intensity = intensity > 255 ? 255 : intensity;
        SDL_SetRenderDrawColor(renderer, intensity, 0, 0, 255);
        draw_circle(renderer,
                    demons[i]->location.x * CELL_SIZE + CELL_SIZE / 2,
                    demons[i]->location.y * CELL_SIZE + CELL_SIZE / 2,
                    radius);
    }

    for (int i = 0; i < player_count; i++) {
        if (!players[i]->active) continue;
        int size = 10 + players[i]->score / 5;
        int intensity = 100 + players[i]->score * 5;
        intensity = intensity > 255 ? 255 : intensity;
        SDL_SetRenderDrawColor(renderer, 0, 0, intensity, 255);
        SDL_Rect rect = {
            players[i]->location.x * CELL_SIZE + (CELL_SIZE - size) / 2,
            players[i]->location.y * CELL_SIZE + (CELL_SIZE - size) / 2,
            size,
            size
        };
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

void close_view() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}