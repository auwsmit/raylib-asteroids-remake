// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef ASTEROIDS_GAME_HEADER_GUARD
#define ASTEROIDS_GAME_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

#define SHIP_WIDTH 100
#define SHIP_LENGTH 150
#define TURN_SPEED 180
#define THRUST_SPEED 10
#define SLOWDOWN_RATE 3

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum ScreenState
{
    SCREEN_LOGO, SCREEN_TITLE, SCREEN_GAMEPLAY, SCREEN_ENDING
} ScreenState;

typedef enum GameMode
{
    MODE_1PLAYER, MODE_2PLAYER, MODE_DEMO
} GameMode;

typedef enum GameDifficulty // Multiplier for CPU paddle speed
{
    DIFFICULTY_EASY, DIFFICULTY_MEDIUM, DIFFICULTY_HARD
} GameDifficulty;

typedef enum PongBeep
{
    BEEP_MENU, BEEP_PADDLE, BEEP_EDGE, BEEP_SCORE
} PongBeep;

typedef struct SpaceShip
{
    Vector2 position;
    Vector2 velocity;
    float velocityRot;
    float rotation; // in degrees, 0 is pointing up, 90 is right
    float width;
    float length;
} SpaceShip;

typedef struct GameState
{
    Sound beeps[4];
    SpaceShip ship;
    float winTimer;   // countdown after player wins
    float scoreTimer; // countdown after a score
    GameMode currentMode;
    ScreenState currentScreen;
    int scoreL;
    int scoreR;
    bool isPaused;
    bool gameShouldExit;
} GameState;

extern GameState asteroidGame; // global declaration

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(void); // Initialize game data and allocate memory for beeps
Sound GenBeep(float freq, float lengthSec); // Generate and allocate memory a sine wave buffer for a beep
void FreeBeeps(void);

// Collision

// Update / User Input
void UpdatePongFrame(void); // Updates all the game's data and objects for the current frame
void UpdateShip(SpaceShip *ship);

// Draw
void DrawPongFrame(void); // Draws all the game's objects for the current frame
void DrawSpaceShip(SpaceShip *ship);

// Game functions
void ResetShip(SpaceShip *ship);

#endif // ASTEROIDS_GAME_HEADER_GUARD
