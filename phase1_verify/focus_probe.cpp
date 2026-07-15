// Phase-1 Step-4 focus-loss probe (windowed, real SDL video subsystem, NOT dummy).
// Goal: exercise the production path Input.cpp uses — SDL_GetKeyboardState(nullptr)
// sampled every frame — and confirm it survives a real focus-loss/minimize cycle.
// This is the closest honest proxy to the alt-tab bug class we can run headlessly.
#include <SDL3/SDL.h>
#include <cstdio>

int main() {
    printf("FOCUS_PROBE_START\n");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init FAIL: %s\n", SDL_GetError());
        return 2;
    }
    SDL_Window* w = SDL_CreateWindow("focusprobe", 640, 480, 0);
    if (!w) {
        printf("CreateWindow FAIL: %s\n", SDL_GetError());
        SDL_Quit();
        return 3;
    }
    printf("WINDOW_CREATED ok\n");

    // Pump a few frames so SDL settles.
    for (int i = 0; i < 5; ++i) { SDL_Event e; while (SDL_PollEvent(&e)) {} SDL_Delay(16); }

    const bool* kb = SDL_GetKeyboardState(nullptr);
    printf("GETKEYBOARDSTATE %s\n", kb ? "NONNULL" : "NULL");
    if (kb) printf("SCANCODE_COUNT=%d W_state=%d\n", SDL_SCANCODE_COUNT, (int)kb[SDL_SCANCODE_W]);

    // Simulate the focus-loss bug class: minimize (loses input focus).
    SDL_MinimizeWindow(w);
    SDL_Delay(120);
    bool minimized = (SDL_GetWindowFlags(w) & SDL_WINDOW_MINIMIZED) != 0;
    printf("AFTER_MINIMIZE minimized=%d\n", minimized);

    const bool* kb2 = SDL_GetKeyboardState(nullptr);
    printf("KB_DURING_FOCUSLOSS %s\n", kb2 ? "NONNULL" : "NULL");
    if (kb2) printf("KB_DURING_FOCUSLOSS_W=%d\n", (int)kb2[SDL_SCANCODE_W]);

    SDL_RestoreWindow(w);
    SDL_Delay(120);
    printf("AFTER_RESTORE minimized=%d\n", (SDL_GetWindowFlags(w) & SDL_WINDOW_MINIMIZED) != 0);

    const bool* kb3 = SDL_GetKeyboardState(nullptr);
    printf("KB_AFTER_RESTORE %s W=%d\n", kb3 ? "NONNULL" : "NULL", kb3 ? (int)kb3[SDL_SCANCODE_W] : -1);

    SDL_DestroyWindow(w);
    SDL_Quit();
    printf("FOCUS_PROBE_DONE\n");
    return 0;
}
