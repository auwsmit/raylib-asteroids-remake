// EXPLANATION:
// TODO

#ifndef ASTEROIDS_SHIP_HEADER_GUARD
#define ASTEROIDS_SHIP_HEADER_GUARD

#include "raylib.h"
#include "missile.h"

// Macros
// ----------------------------------------------------------------------------

#define SHIP_WIDTH 40.0f
#define SHIP_LENGTH 60.0f
#define SHIP_TURN_SPEED 190.0f // turn X degrees per second
#define SHIP_THRUST_SPEED 400.0f
#define SHIP_MAX_SPEED 1000.0f
#define SHIP_RESPAWN_TIME 1.5f
#define SHIP_SAFE_TIME 3.0f
#define SHIP_AUTO_FIRE_RATE 0.3f
#define SHIP_SPACE_FRICTION 2.0f // how quickly the player slows to 0

// Types and Structures
// ----------------------------------------------------------------------------

typedef struct SpaceShip {
    Missile missiles[MISSILE_MAX];
    Vector2 position;
    Vector2 shipPoints[3];
    Vector2 jetPoints[3];
    Vector2 velocity;
    float rotation; // in degrees, 0 is pointing up, 90 is right
    float width;
    float length;
    float autoFireTimer;
    float respawnTimer;
    float safeRespawnTimer;
    unsigned int shotCount;
    bool isThrusting;
    bool isAtScreenEdge;
    bool exploded;
} SpaceShip;


// Prototypes
// ----------------------------------------------------------------------------

void UpdateShip(SpaceShip *ship);
void UpdateShipTriangles(SpaceShip *ship);
void DrawShip(SpaceShip *ship);

void RespawnShip(SpaceShip *ship);
void ShootMissile(SpaceShip *ship);

#endif // ASTEROIDS_SHIP_HEADER_GUARD

