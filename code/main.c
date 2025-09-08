// EXPLANATION:
// The main entry point for the game/program
// - See header files for more explanations/documentation

#include "raylib.h"

#include "config.h" // Program config, e.g. window title/size, fps, vsync
#include "input.h"  // Input controls / key mappings
#include "logo.h"   // Raylib logo animation
#include "ui.h"     // User interface (menus and buttons)
#include "asteroids.h"

#if defined(PLATFORM_WEB) // for compiling to wasm (web assembly)
    #include <emscripten/emscripten.h>
#endif

typedef struct Viewport {
    int width, height, x, y;
} Viewport;

// Globals
// ----------------------------------------------------------------------------
GameState game; // game data
UiState   ui;   // user interface data
Viewport  view; // for rendering within aspect ratio

// Local Functions Declaration
// ----------------------------------------------------------------------------
void CreateNewWindow(void); // Creates a new window with the proper initial settings
void RunGameLoop(void); // Runs the game loop depending on platform
void UpdateCameraViewport(void);

void UpdateDrawFrame(void); // Update and Draw the current frame
                            // Most of the game loop's code is found in here

// Main entry point
// ----------------------------------------------------------------------------
int main(void)
{
    // Initialization
    // ----------------------------------------------------------------------------
    CreateNewWindow();
    InitAudioDevice();
    InitDefaultInputControls();
    InitRaylibLogo();
    InitUiState();   // also allocates memory for menu buttons
    InitGameState(); // also allocates memory for beep sound effects

    // No exit key (use alt+F4 or in-game exit option)
    SetExitKey(KEY_NULL);

    // Debug:
    SetExitKey(KEY_Q);

    // Start the game loop
    // (See UpdateDrawFrame() for the full game loop)
    RunGameLoop();

    // De-Initialization
    // ----------------------------------------------------------------------------
    FreeBeeps();
    FreeUiMenuButtons();
    CloseAudioDevice();
    CloseWindow(); // Close window and OpenGL context

    return 0;
}

void CreateNewWindow(void)
{
#if defined(PLATFORM_WEB)
    SetConfigFlags(0); // no vsync or window resize for web
#else
    unsigned int windowFlags = FLAG_WINDOW_RESIZABLE;
    if (VSYNC_ENABLED) windowFlags |= FLAG_VSYNC_HINT;
    SetConfigFlags(windowFlags);
#endif
    InitWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, WINDOW_TITLE);
    SetWindowMinSize(320, 240);
}

void RunGameLoop(void)
{
#if defined(PLATFORM_WEB)
    const int emscriptenFPS = 0; // Let emscripten handle the framerate because setting a specific one is kinda janky
                                 // Generally, it will use whatever the monitor's refresh rate is
    emscripten_set_main_loop(UpdateDrawFrame, emscriptenFPS, 1);
#else
    if (MAX_FRAMERATE > 0)
        SetTargetFPS(MAX_FRAMERATE);
    // ----------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose() && !game.gameShouldExit) // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif
}

void UpdateCameraViewport(void)
{
    int winWidth = GetScreenWidth();
    int winHeight = GetScreenHeight();
    float windowAspect = (float)winWidth/(float)winHeight;

    if (windowAspect > ASPECT_RATIO)
    {
        // Window too wide → pillarbox
        view.height = winHeight;
        view.width = (winHeight*ASPECT_RATIO);
        view.x = (winWidth - view.width)/2;
        view.y = 0;
    }
    else
    {
        // Window too tall → letterbox
        view.width = winWidth;
        view.height = (int)(winWidth/ASPECT_RATIO);
        view.x = 0;
        view.y = (winHeight - view.height)/2;
    }

    game.camera.offset = (Vector2){ view.x + view.width/2.0f, view.y + view.height/2.0f };
    game.camera.zoom   = (float)view.width/VIRTUAL_WIDTH;
}

// Update data and draw elements to the screen for the current frame
void UpdateDrawFrame(void)
{
    // Update
    // ----------------------------------------------------------------------------
    HandleToggleFullscreen();
    UpdateCameraViewport();

    switch(game.currentScreen)
    {
        case SCREEN_LOGO:     UpdateRaylibLogo();
                              break;
        case SCREEN_TITLE:    UpdateUiFrame();
                              break;
        case SCREEN_GAMEPLAY: UpdateGameFrame();
                              break;
        default: break;
    }

    // Draw
    // ----------------------------------------------------------------------------

    BeginDrawing(); // Draw to screen
        ClearBackground(BLACK); // Clear previous frame

        // Draw within aspect ratio
        BeginScissorMode(view.x, view.y, view.width, view.height);
            BeginMode2D(game.camera); // Scale to camera view


            switch(game.currentScreen)
            {
                case SCREEN_LOGO:     DrawRaylibLogo();
                                      break;
                case SCREEN_TITLE:    DrawUiFrame();
                                      break;
                case SCREEN_GAMEPLAY: DrawGameFrame();
                                      break;
                default: break;
            }

            EndMode2D();
        EndScissorMode();
    // DrawFPS(0, 0);
    EndDrawing();
}
