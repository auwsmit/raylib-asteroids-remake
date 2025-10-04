// EXPLANATION:
// For configuring aspects of the program outside of game logic

#ifndef ASTEROIDS_CONFIG_HEADER_GUARD
#define ASTEROIDS_CONFIG_HEADER_GUARD

// Macros
// ----------------------------------------------------------------------------

#define WINDOW_TITLE "Asteroids Remake with raylib"

#define ASPECT_RATIO (16.0f/9.0f)
#define VIRTUAL_HEIGHT 1080 // The size of the game world
#define VIRTUAL_WIDTH (int)(VIRTUAL_HEIGHT*ASPECT_RATIO)

#define INITIAL_HEIGHT 720 // Default size of the game window
#define INITIAL_WIDTH (int)(INITIAL_HEIGHT*ASPECT_RATIO)

// there may be small bugs with very high FPS (uncapped + no vsync), but should work fine overall
#define MAX_FRAMERATE 120 // Set to 0 for uncapped framerate
#define VSYNC_ENABLED true

#endif // ASTEROIDS_CONFIG_HEADER_GUARD
