// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef ASTEROIDS_GAME_HEADER_GUARD
#define ASTEROIDS_GAME_HEADER_GUARD

#include "raylib.h"
#include "asteroid.h"
#include "ship.h"
#include "input.h"

// Macros
// ----------------------------------------------------------------------------
#define STARTING_LIVES 3
#define STAR_AMOUNT 800
#define EXPLOSION_TIME 0.4f
#define NEW_LEVEL_TIMER 1.5f
#define GAMEOVER_INPUT_COOLDOWN 1.0f
#define LVL1_ASTEROID_AMOUNT 2

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum ScreenState {
    SCREEN_LOGO, SCREEN_TITLE, SCREEN_GAMEPLAY
} ScreenState;

typedef struct GameSounds {
    Sound menu;
    Sound explodeSmall;
    Sound explodeMedium;
    Sound explodeBig;
} GameSounds;

typedef struct GameTextures {
    Texture ship;
    Texture asteroidA;
    Texture asteroidB;
    Texture asteroidC;
} GameTextures;

typedef struct GameState {
    GameSounds sounds;
    GameTextures textures;
    Camera2D camera;
    SpaceShip ship;
    Asteroid *rocks;
    Vector2 stars[STAR_AMOUNT];
    Vector2 shipTriangle[3];
    Vector2 jetTriangle[3];
    Vector2 wrapOffsets[8];
    ScreenState currentScreen;
    int touchCount;
    unsigned int currentLevel;
    unsigned int lives;
    unsigned int rockCountStartOfLevel;
    unsigned int rockCount;
    unsigned int rockLimit;
    unsigned int eliminatedCount;
    float frameTime;
    float messageTimer;
    float newLevelTimer;
    bool anyKeyPressed;
    bool touchMode;
    bool isPaused;
    bool levelFinished;
    bool resumeInputCooldown;
    bool gameShouldExit;
} GameState;

extern GameState game; // global declaration

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(ScreenState screen); // Initialize game data and allocate memory for sounds
void InitNewLevel(unsigned int newLevel);
void FreeGameState(void); // Free any allocated memory within game state

// Update & Draw
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void DrawGameFrame(void); // Draws all the game's objects for the current frame

// Collision
bool IsShipOnEdge(SpaceShip *ship);
bool IsCircleOnEdge(Vector2 position, float radius);
bool CheckCollisionAsteroidShip(unsigned int rockIdx, SpaceShip *ship);
void WrapPastEdge(Vector2 *position);

#endif // ASTEROIDS_GAME_HEADER_GUARD
