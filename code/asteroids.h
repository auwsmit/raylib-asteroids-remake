// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef ASTEROIDS_GAME_HEADER_GUARD
#define ASTEROIDS_GAME_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

#define SHIP_WIDTH 100.0f
#define SHIP_LENGTH 150.0f
#define SHIP_TURN_SPEED 180.0f // turn X degrees per second
#define SHIP_THRUST_SPEED 5.0f
#define SHIP_MAX_SPEED 10.0f
#define SPACE_FRICTION 5.0f

#define ASTEROID_AMOUNT 8
#define ASTEROID_SPEED 300.0f

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

typedef enum GameBeep
{
    BEEP_MENU,
} GameBeep;

typedef struct SpaceShip
{
    Vector2 position;
    Vector2 velocity;
    float rotation; // in degrees, 0 is pointing up, 90 is right
    float width;
    float length;
    bool isAtScreenEdge;
} SpaceShip;

typedef struct Asteroid
{
    Vector2 position;
    Vector2 velocity;
    float angle;
    float speed;
    float size;
    unsigned int life;
    bool isAtScreenEdge;
} Asteroid;

typedef struct GameState
{
    Sound beeps[1];
    SpaceShip ship;
    Asteroid rocks[ASTEROID_AMOUNT];
    float winTimer;   // countdown after player wins
    float scoreTimer; // countdown after a score
    GameMode currentMode;
    ScreenState currentScreen;
    int scoreL;
    int scoreR;
    bool isPaused;
    bool gameShouldExit;
} GameState;

extern GameState game; // global declaration

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(void); // Initialize game data and allocate memory for beeps
Sound GenBeep(float freq, float lengthSec); // Generate and allocate memory a sine wave buffer for a beep
void FreeBeeps(void);

// Collision
bool IsShipOnEdge(SpaceShip *ship);

// Update / User Input
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void UpdateShipPastEdge(SpaceShip *ship);
void UpdateShip(SpaceShip *ship);
void UpdateAsteroid(Asteroid *rock);

// Draw
void DrawGameFrame(void); // Draws all the game's objects for the current frame
void DrawSpaceShip(SpaceShip *ship);
void DrawAsteroid(Asteroid *rock);

// Game functions
void ResetShip(SpaceShip *ship);

#endif // ASTEROIDS_GAME_HEADER_GUARD
