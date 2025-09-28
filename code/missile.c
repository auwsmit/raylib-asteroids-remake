#include "missile.h"
#include "raymath.h"
#include "game.h"

void UpdateMissile(Missile *shot)
{
    if (shot->isExploded)
    {
        shot->explosionTimer -= game.frameTime;
        return;
    }

    // Update position
    Vector2 currentVelocity = (Vector2){ 0, shot->speed*game.frameTime };
    currentVelocity = Vector2Rotate(currentVelocity, shot->angle*DEG2RAD);
    currentVelocity = Vector2Add(currentVelocity, Vector2Scale(game.ship.velocity, game.frameTime));
    shot->position = Vector2Add(shot->position, currentVelocity);
    shot->isAtScreenEdge = IsCircleOnEdge(shot->position, shot->radius);
    WrapPastEdge(&shot->position);

    // Update despawn timer
    shot->despawnTimer -= game.frameTime;
    if (shot->despawnTimer <= 0)
    {
        shot->isExploded = true;
        shot->explosionTimer = 0.0f;
    }
}

void DrawMissile(Missile *shot)
{
    if ((shot->explosionTimer > EPSILON) && shot->isExploded)
        DrawCircleV(shot->position, shot->radius*5, Fade(MAROON, 0.5f));
    if (shot->isExploded) return;

    Color missileColor = RAYWHITE;

    if (shot->overheated)
    {
        missileColor = ColorTint(missileColor, RED);
        missileColor = ColorBrightness(missileColor, 0.7f);
    }

    DrawCircleV((Vector2){ shot->position.x, shot->position.y }, shot->radius, missileColor);

    // Clones at opposite side of screen
    if (shot->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneAsteroid = Vector2Add(shot->position, game.wrapOffsets[i]);
            DrawCircleV((Vector2){ cloneAsteroid.x, cloneAsteroid.y }, shot->radius, missileColor);
        }
    }
}
