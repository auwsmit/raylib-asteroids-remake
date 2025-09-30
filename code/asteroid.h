#ifndef ASTEROIDS_ASTEROID_HEADER_GUARD
#define ASTEROIDS_ASTEROID_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

#define ASTEROID_RADIUS_BIG 80
#define ASTEROID_RADIUS_MEDIUM 40
#define ASTEROID_RADIUS_SMALL 20
#define ASTEROID_SPEED 300.0f

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum SizeOfAsteroid {
    ASTEROID_SIZE_SMALL,
    ASTEROID_SIZE_MEDIUM,
    ASTEROID_SIZE_BIG,
} SizeOfAsteroid;

typedef struct Asteroid {
    Sound *soundExplode;
    Texture *sprite;
    Color color;
    Vector2 position;
    Vector2 velocity;
    float angle;
    float spriteAngle;
    float speed;
    float radius;
    SizeOfAsteroid size;
    bool isAtScreenEdge;
    bool rotateLeft;
    bool isExploded;
} Asteroid;

// Prototypes
// ----------------------------------------------------------------------------

unsigned int CreateAsteroid(SizeOfAsteroid size, Vector2 position, float angle, Color color);
unsigned int CreateAsteroidRandom(SizeOfAsteroid size);
Color ColorBrightnessVariation(Color color);
void SplitAsteroid(unsigned int rockIdx);
void UpdateAsteroid(unsigned int rockIdx);
void DrawAsteroid(unsigned int rockIdx);

#endif // ASTEROIDS_ASTEROID_HEADER_GUARD


