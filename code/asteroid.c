#include "asteroid.h"
#include "raymath.h" // needed for vector math
#include "config.h"
#include "game.h"

unsigned int CreateAsteroid(SizeOfAsteroid size, Vector2 position, float angle, Color color)
{
    game.rockCount++;
    game.rocks = MemRealloc(game.rocks, game.rockCount*sizeof(Asteroid));
    unsigned int rockIdx = game.rockCount - 1;
    Asteroid *rock = &game.rocks[rockIdx];
    *rock = (Asteroid){ 0 };
    rock->exploded = false;
    rock->color = color;

    // radius
    rock->size = size;
    if (size == ASTEROID_SIZE_BIG)
        rock->radius = ASTEROID_RADIUS_BIG;
    else if (size == ASTEROID_SIZE_MEDIUM)
        rock->radius = ASTEROID_RADIUS_MEDIUM;
    else if (size == ASTEROID_SIZE_SMALL)
        rock->radius = ASTEROID_RADIUS_SMALL;
    unsigned int newRockAdd = 1;
    for (int i = rock->size; i >= 0; i--)
        newRockAdd *= 2;
    game.rockLimit += newRockAdd;

    // position & angle
    rock->position = position;
    rock->angle = angle;

    // Speed proportional to size
    float radiusRange = ASTEROID_RADIUS_BIG - ASTEROID_RADIUS_SMALL;
    float scaledSpeed;
    scaledSpeed = ASTEROID_SPEED*(ASTEROID_RADIUS_BIG - rock->radius)/radiusRange;
    if (scaledSpeed < ASTEROID_SPEED/8) // minimum speed
        scaledSpeed = ASTEROID_SPEED/8;

    rock->speed = scaledSpeed;

    return rockIdx;
}

unsigned int CreateAsteroidRandom(SizeOfAsteroid size)
{
    float rockPosX = (float)GetRandomValue(0, VIRTUAL_WIDTH);
    float rockPosY = (float)GetRandomValue(0, VIRTUAL_HEIGHT);
    float angle = (float)GetRandomValue(0, 360);
    Color colorVariation = ColorBrightnessVariation(BROWN);

    unsigned int rockIdx = CreateAsteroid(size, (Vector2){ rockPosX, rockPosY }, angle, colorVariation);
    Asteroid *rock = &game.rocks[rockIdx];

    float safeZoneRadius = game.ship.length*3;
    rock->radius += safeZoneRadius;
    if (CheckCollisionAsteroidShip(rockIdx, &game.ship))
    {
        rock->position.x += ((GetRandomValue(0, 1)*2) - 1)*rock->radius*2;
        rock->position.y += ((GetRandomValue(0, 1)*2) - 1)*rock->radius*2;
    }
    rock->radius -= safeZoneRadius;

    return rockIdx;
}

Color ColorBrightnessVariation(Color color)
{
    float brightness = -0.25f*GetRandomValue(0, 2); // 3 main shades
    brightness += 0.01f*GetRandomValue(1, 10); // sub-shades
    color = ColorBrightness(color, brightness);
    return color;
}

void SplitAsteroid(unsigned int rockIdx)
{
    Asteroid *rock = &game.rocks[rockIdx];
    float angle = (float)GetRandomValue(0, 180);
    Vector2 spawnPosA = { 0, rock->radius/2 };
    spawnPosA = Vector2Rotate(spawnPosA, angle*DEG2RAD);
    Vector2 spawnPosB = Vector2Negate(spawnPosA);
    spawnPosA = Vector2Add(spawnPosA, rock->position);
    spawnPosB = Vector2Add(spawnPosB, rock->position);

    if (rock->size > ASTEROID_SIZE_SMALL)
    {
        SizeOfAsteroid splitSize = rock->size - 1;
        Color splitColor = rock->color;
        CreateAsteroid(splitSize, spawnPosA, angle, splitColor);
        CreateAsteroid(splitSize, spawnPosB, angle + 180, splitColor);
        unsigned int splitRocksAdd = 1;
        for (int i = splitSize + 1; i >= 0; i--)
            splitRocksAdd *= 2;
        game.rockLimit -= splitRocksAdd;
    }
}

void UpdateAsteroid(unsigned int rockIdx)
{
    Asteroid *rock = &game.rocks[rockIdx];
    if (rock->exploded) return;

    // Update position
    Vector2 currentVelocity = (Vector2){ 0, rock->speed*game.frameTime };
    currentVelocity = Vector2Rotate(currentVelocity, rock->angle*DEG2RAD);
    rock->position = Vector2Add(rock->position, currentVelocity);
    rock->isAtScreenEdge = IsCircleOnEdge(rock->position, rock->radius);
    WrapPastEdge(&rock->position);

    // Check collision with missiles
    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        Missile *shot = &game.ship.missiles[i];
        if (!shot->exploded && CheckCollisionCircles(rock->position, rock->radius,
                                                     shot->position, shot->radius))
        {
            rock->exploded = true;
            shot->exploded = true;
        }

        if (rock->isAtScreenEdge)
        {
            for (unsigned int o = 0; o < 8; o++)
            {
                Vector2 cloneRockPos = Vector2Add(rock->position, game.wrapOffsets[o]);
                if (!shot->exploded && CheckCollisionCircles(cloneRockPos, rock->radius,
                                                             shot->position, shot->radius))
                {
                    rock->exploded = true;
                    shot->exploded = true;
                }
            }
        }
    }

    if (rock->exploded)
    {
        game.eliminatedCount++;
        SplitAsteroid(rockIdx);
        Sound explosion = game.sounds[SOUND_EXPLODE_SMALL];
        if (rock->size == ASTEROID_SIZE_MEDIUM)
            explosion = game.sounds[SOUND_EXPLODE_MEDIUM];
        else if (rock->size == ASTEROID_SIZE_BIG)
            explosion = game.sounds[SOUND_EXPLODE_BIG];
        PlaySound(explosion);
    }
}

void DrawAsteroid(unsigned int rockIdx)
{
    Asteroid *rock = &game.rocks[rockIdx];
    DrawCircleV((Vector2){ rock->position.x, rock->position.y }, rock->radius, rock->color);

    // Clones at opposite side of screen
    if (rock->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneAsteroid = Vector2Add(rock->position, game.wrapOffsets[i]);
            DrawCircleV((Vector2){ cloneAsteroid.x, cloneAsteroid.y }, rock->radius, rock->color);
        }
    }
}
