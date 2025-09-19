#include "ship.h"
#include "raymath.h"
#include "config.h"
#include "input.h"
#include "ui.h"
#include "game.h"

void UpdateShip(SpaceShip *ship)
{
    // Update timers
    // ----------------------------------------------------------------------------

    if (ship->exploded)
    {
        ship->explosionTimer -= game.frameTime;
        if (game.delayTimer < EPSILON)
            ship->respawnTimer -= game.frameTime;

        // Respawn
        if ((ship->respawnTimer <= EPSILON) && (game.lives > 0))
            RespawnShip(ship);

        // do not update, ship has exploded
        return;
    }
    if (ship->safeRespawnTimer > 0)
        ship->safeRespawnTimer -= game.frameTime;

    // Player Input
    // ----------------------------------------------------------------------------

    // check if mouse is over an interactive UI element
    bool isMouseLeftValid = (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !ui.mouseInUse);
    bool isInputActionValid = isMouseLeftValid || !IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    // Rotate (mouse)
    if ((Vector2Length(GetMouseDelta()) != 0) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON) || isMouseLeftValid)
    {
        Vector2 mousePos = GetScaledMousePosition();
        Vector2 mouseDirection = Vector2Subtract(mousePos, ship->position);
        float distanceToMouse = Vector2Length(mouseDirection);
        if ((IsInputActionDown(INPUT_ACTION_FORWARD) && distanceToMouse > ship->length) ||
            !IsInputActionDown(INPUT_ACTION_FORWARD) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
            ship->rotation = (float)atan2(mouseDirection.y, mouseDirection.x)*RAD2DEG + 90;
    }
    // Rotate (keys)
    if (IsInputActionDown(INPUT_ACTION_LEFT))
        ship->rotation -= SHIP_TURN_SPEED*game.frameTime;
    if (IsInputActionDown(INPUT_ACTION_RIGHT))
        ship->rotation += SHIP_TURN_SPEED*game.frameTime;

    // Calculate thrust amount
    if (IsInputActionDown(INPUT_ACTION_FORWARD))
    {
        Vector2 thrust = (Vector2){ 0, -SHIP_THRUST_SPEED };
        thrust = Vector2Rotate(thrust, ship->rotation*DEG2RAD);
        thrust = Vector2Scale(thrust, game.frameTime);
        ship->velocity = Vector2Add(ship->velocity, thrust);
        ship->velocity = Vector2ClampValue(ship->velocity, 0, SHIP_MAX_SPEED);
        ship->isThrusting = true;
    }
    else if (ship->isThrusting)
        ship->isThrusting = false;

    // Shoot missile
    if (IsInputActionDown(INPUT_ACTION_SHOOT) && isInputActionValid)
    {
        if (ship->autoFireTimer == 0)
        {
            ShootMissile(ship);
            ship->autoFireTimer += game.frameTime;
        }
        else if (ship->autoFireTimer > SHIP_AUTO_FIRE_RATE)
            ship->autoFireTimer = 0;
        else
            ship->autoFireTimer += game.frameTime;
    }
    else if (ship->autoFireTimer != 0)
        ship->autoFireTimer = 0;

    // Calculate motion
    // ----------------------------------------------------------------------------

    // Apply friction (smooth exponential decay)
    float slowdown = expf(-SHIP_SPACE_FRICTION/10*game.frameTime);
    ship->velocity = Vector2Scale(ship->velocity, slowdown);

    // Update position
    Vector2 scaledVelocity = Vector2Scale(ship->velocity, game.frameTime);
    ship->position = Vector2Add(ship->position, scaledVelocity);
    UpdateShipTriangles(ship);

    // Screen edge wrap
    ship->isAtScreenEdge = IsShipOnEdge(ship);
    WrapPastEdge(&ship->position);

    // Collision
    // ----------------------------------------------------------------------------

    // Check collision with asteroids
    if (ship->safeRespawnTimer > 0) return;
    for (unsigned int i = 0; i < game.rockCount; i++)
    {
        Asteroid *rock = &game.rocks[i];
        if (!rock->exploded && CheckCollisionAsteroidShip(i, &game.ship))
        {
            ship->exploded = true;
            ship->explosionTimer = EXPLOSION_TIME;
            rock->exploded = true;
            SplitAsteroid(i);
            game.eliminatedCount++;
            PlaySound(game.beeps[BEEP_EXPLODE]);
        }
    }
    if (game.ship.exploded)
    {
        game.lives--;
        if (game.lives > 0)
            game.delayTimer = 3.0f;
        ui.textFade = 1.0f; // for respawn message
    }
}

void UpdateShipTriangles(SpaceShip *ship)
{
    // Calculate new triangle points for drawing, collision, & screen wrap
    for (unsigned int i = 0; i < 3; i++)
    {
        ship->shipPoints[i] = Vector2Rotate(game.shipTriangle[i], ship->rotation*DEG2RAD);
        ship->shipPoints[i] = Vector2Add(ship->shipPoints[i], ship->position);
        ship->jetPoints[i] = Vector2Rotate(game.jetTriangle[i], (ship->rotation+180)*DEG2RAD);
        ship->jetPoints[i] = Vector2Add(ship->jetPoints[i], ship->position);
    }
}

void DrawShip(SpaceShip *ship)
{
    // Transparent ship during respawn invincibility
    Color shipColor = GRAY;
    Color jetColor = Fade(ORANGE, 0.5f);
    if (ship->safeRespawnTimer > 0)
    {
        shipColor = Fade(shipColor, 0.5);
        jetColor = Fade(jetColor, 0.25);
    }

    // Get and transform ship triangle + jet triangle
    DrawTriangle(ship->shipPoints[0], ship->shipPoints[1], ship->shipPoints[2], shipColor);
    if (ship->isThrusting)
        DrawTriangle(ship->jetPoints[0], ship->jetPoints[1], ship->jetPoints[2], jetColor);

    // Clones at opposite side of screen
    if (ship->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneShip[3];
            Vector2 cloneJet[3];
            cloneShip[0] = Vector2Add(ship->shipPoints[0], game.wrapOffsets[i]);
            cloneShip[1] = Vector2Add(ship->shipPoints[1], game.wrapOffsets[i]);
            cloneShip[2] = Vector2Add(ship->shipPoints[2], game.wrapOffsets[i]);
            cloneJet[0] = Vector2Add(ship->jetPoints[0], game.wrapOffsets[i]);
            cloneJet[1] = Vector2Add(ship->jetPoints[1], game.wrapOffsets[i]);
            cloneJet[2] = Vector2Add(ship->jetPoints[2], game.wrapOffsets[i]);

            DrawTriangle(cloneShip[0], cloneShip[1], cloneShip[2], shipColor);
            if (IsInputActionDown(INPUT_ACTION_FORWARD))
                DrawTriangle(cloneJet[0], cloneJet[1], cloneJet[2], jetColor);
        }
    }
}

void RespawnShip(SpaceShip *ship)
{
    ship->exploded = false;
    ship->isThrusting = false;
    ship->position = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };
    ship->velocity = (Vector2){ 0, 0 };
    ship->rotation = 90;
    ship->respawnTimer = SHIP_RESPAWN_TIME;
    if (game.lives != STARTING_LIVES)
        ship->safeRespawnTimer = SHIP_SAFE_TIME;

    UpdateShipTriangles(ship);
}

void ShootMissile(SpaceShip *ship)
{
    // spawn bullet
    if (ship->shotCount == MISSILE_MAX)
        ship->shotCount = 0;

    Missile *shot = &ship->missiles[ship->shotCount];

    shot->exploded = false;
    shot->explosionTimer = EXPLOSION_TIME;
    shot->angle = ship->rotation + 180;
    Vector2 spawnPos = { 0, ship->length*0.6f + shot->radius };
    spawnPos = Vector2Rotate(spawnPos, shot->angle*DEG2RAD);
    spawnPos = Vector2Add(spawnPos, ship->position);
    shot->position = spawnPos;
    shot->despawnTimer = MISSILE_DESPAWN_TIME;

    ship->shotCount++;
    PlaySound(game.beeps[BEEP_SHOOT]);
}
