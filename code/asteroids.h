// EXPLANATION:
// All the game logic, including how/when to draw to screen

#ifndef ASTEROIDS_GAME_HEADER_GUARD
#define ASTEROIDS_GAME_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

#define SHIP_WIDTH 40.0f
#define SHIP_LENGTH 60.0f
#define SHIP_TURN_SPEED 190.0f // turn X degrees per second
#define SHIP_THRUST_SPEED 400.0f
#define SHIP_MAX_SPEED 1000.0f
#define SPACE_FRICTION 5.0f
#define SHIP_RESPAWN_TIME 2.0f

#define MAX_SHOTS 4
#define SHOT_RADIUS 5.0f
#define SHOT_SPEED 1100.0f

#if 0 // performance test
#define MAX_ASTEROIDS 10000
#define ASTEROID_MIN_RADIUS 1.0f
#define ASTEROID_MAX_RADIUS 5.0f
#else
#define MAX_ASTEROIDS 10
#define ASTEROID_MIN_RADIUS 10
#define ASTEROID_MAX_RADIUS 80
#endif
#define ASTEROID_SPEED 300.0f

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum ScreenState {
    SCREEN_LOGO, SCREEN_TITLE, SCREEN_GAMEPLAY, SCREEN_ENDING
} ScreenState;

typedef enum GameMode {
    MODE_1PLAYER, MODE_2PLAYER, MODE_DEMO
} GameMode;

typedef enum GameBeep {
    BEEP_MENU,
} GameBeep;

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    float angle;
    float speed;
    float radius;
    unsigned int life;
    bool isAtScreenEdge;
} Asteroid;

typedef struct Missile {
    Vector2 position;
    Vector2 velocity;
    float angle;
    float speed;
    float radius;
    float despawnTimer;
    bool isAtScreenEdge;
    bool exists;
} Missile;

typedef struct SpaceShip {
    Missile shots[MAX_SHOTS];
    Vector2 position;
    Vector2 velocity;
    float rotation; // in degrees, 0 is pointing up, 90 is right
    float width;
    float length;
    float respawnTimer;
    unsigned int shotCount;
    bool isAtScreenEdge;
    bool exploded;
} SpaceShip;

typedef struct GameState {
    Sound beeps[1];
    Camera2D camera;
    SpaceShip ship;
    Asteroid rocks[MAX_ASTEROIDS];
    GameMode currentMode;
    Vector2 shipTri[3];
    ScreenState currentScreen;
    unsigned int scoreL;
    unsigned int scoreR;
    unsigned int lives;
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

// Spawn
void ShootMissile(SpaceShip *ship);
// void SpawnAsteroid(void);

// Collision
bool IsShipOnEdge(SpaceShip *ship);
bool IsCircleOnEdge(Vector2 position, float radius);
bool CheckCollisionAsteroidShip(Asteroid *rock, SpaceShip *ship);
// bool CheckCollisionAsteroidMissile(Asteroid *rock, Missile *shot);

// Update / User Input
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void WrapPastEdge(Vector2 *position);
void UpdateAsteroid(Asteroid *rock);
void UpdateMissile(Missile *shot);
void UpdateShip(SpaceShip *ship);

// Draw
void DrawGameFrame(void); // Draws all the game's objects for the current frame
void DrawAsteroid(Asteroid *rock);
void DrawMissile(Missile *shot);
void DrawShip(SpaceShip *ship);

// Game functions
void ResetShip(SpaceShip *ship);

#endif // ASTEROIDS_GAME_HEADER_GUARD
