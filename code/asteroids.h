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
#define SPACE_FRICTION 2.0f
#define SHIP_RESPAWN_TIME 2.0f

#define MAX_SHOTS 4
#define SHOT_RADIUS 5.0f
#define SHOT_SPEED 1100.0f

#define ASTEROID_COUNT 3
#define ASTEROID_RADIUS_BIG 80
#define ASTEROID_RADIUS_MEDIUM 40
#define ASTEROID_RADIUS_SMALL 20
#define ASTEROID_SPEED 300.0f

#define EXPLOSION_TIME 0.4f

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum ScreenState {
    SCREEN_LOGO, SCREEN_TITLE, SCREEN_GAMEPLAY, SCREEN_ENDING
} ScreenState;

typedef enum GameMode {
    MODE_1PLAYER, MODE_2PLAYER, MODE_DEMO
} GameMode;

typedef enum GameBeep {
    BEEP_MENU, BEEP_SHOOT, BEEP_EXPLODE
} GameBeep;

typedef enum SizeOfAsteroid {
    ASTEROID_SIZE_SMALL = 1,
    ASTEROID_SIZE_MEDIUM = 2,
    ASTEROID_SIZE_BIG = 3,
} SizeOfAsteroid;

typedef struct Asteroid {
    Vector2 position;
    Vector2 velocity;
    float angle;
    float speed;
    float radius;
    SizeOfAsteroid size;
    bool isAtScreenEdge;
    bool exploded;
} Asteroid;

typedef struct Missile {
    Vector2 position;
    Vector2 velocity;
    float angle;
    float speed;
    float radius;
    float despawnTimer;
    float explosionTimer;
    bool isAtScreenEdge;
    bool exploded;
} Missile;

typedef struct SpaceShip {
    Missile missiles[MAX_SHOTS];
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
    Sound beeps[3];
    Camera2D camera;
    SpaceShip ship;
    Asteroid *rocks;
    GameMode currentMode;
    Vector2 shipTriangle[3];
    ScreenState currentScreen;
    unsigned int rockCount;
    unsigned int eliminatedCount;
    // unsigned int scoreL;
    // unsigned int scoreR;
    // unsigned int lives;
    bool isPaused;
    bool gameShouldExit;
} GameState;

extern GameState game; // global declaration

// Prototypes
// ----------------------------------------------------------------------------

// Initialization
void InitGameState(void); // Initialize game data and allocate memory for beeps
Sound GenBeep(float freq, float lengthSec); // Generate and allocate memory a sine wave buffer for a beep
void FreeGameState(void); // Free any allocated memory within game state

// Create/Destroy Entities
void ShootMissile(SpaceShip *ship);
Asteroid *CreateAsteroid(SizeOfAsteroid size, Vector2 position, float angle);
void CreateAsteroidRandom(SizeOfAsteroid size);
void SplitAsteroid(Asteroid *rock);

// Collision
bool IsShipOnEdge(SpaceShip *ship);
bool IsCircleOnEdge(Vector2 position, float radius);
bool CheckCollisionAsteroidShip(Asteroid *rock, SpaceShip *ship);

// Update & User Input
void UpdateGameFrame(void); // Updates all the game's data and objects for the current frame
void WrapPastEdge(Vector2 *position);
void UpdateAsteroid(Asteroid *rock);
void UpdateMissile(Missile *shot);
void UpdateShip(SpaceShip *ship);
void ResetShip(SpaceShip *ship);

// Draw
void DrawGameFrame(void); // Draws all the game's objects for the current frame
void DrawAsteroid(Asteroid *rock);
void DrawMissile(Missile *shot);
void DrawShip(SpaceShip *ship);

// Game functions

#endif // ASTEROIDS_GAME_HEADER_GUARD
