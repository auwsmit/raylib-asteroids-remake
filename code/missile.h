#ifndef ASTEROIDS_MISSILE_HEADER_GUARD
#define ASTEROIDS_MISSILE_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

#define MISSILE_MAX 5
#define MISSILE_RADIUS 5.0f
#define MISSILE_SPEED 700.0f
#define MISSILE_DESPAWN_TIME 1.25f

// Types and Structures
// ----------------------------------------------------------------------------

typedef struct Missile {
    Vector2 position;
    Vector2 velocity;
    float angle;
    float speed;
    float radius;
    float despawnTimer;
    float explosionTimer;
    bool isAtScreenEdge;
    bool isExploded;
    bool overheated;
} Missile;

// Prototypes
// ----------------------------------------------------------------------------
void UpdateMissile(Missile *shot);
void DrawMissile(Missile *shot);

#endif // ASTEROIDS_MISSILE_HEADER_GUARD
