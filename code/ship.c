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

    if (ship->isExploded)
    {
        ship->explosionTimer -= game.frameTime;
        if (game.messageTimer < EPSILON)
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

    // Rotate (mouse)
    bool mouseMoved = (Vector2Length(GetMouseDelta()) != 0);
    if (!game.touchMode && mouseMoved)
    {
        RotateShipToMouse(ship);
    }

    // Rotate (touch analog stick)
    if (ui.stick.isActive)
    {
        Vector2 stickDirection = Vector2Subtract(ui.stick.stickPos, ui.stick.centerPos);
        ship->angle = (float)atan2(stickDirection.y, stickDirection.x)*RAD2DEG + 90;
    }

    // Rotate (keys)
    if (IsInputActionDown(INPUT_ACTION_LEFT))
        ship->angle -= SHIP_TURN_SPEED*game.frameTime;
    if (IsInputActionDown(INPUT_ACTION_RIGHT))
        ship->angle += SHIP_TURN_SPEED*game.frameTime;

    // Thrust jet forward
    bool inputActionThrust = IsInputActionDown(INPUT_ACTION_THRUST);
    bool keyInputThrust = !IsInputActionMouseDown(INPUT_ACTION_THRUST);
    bool mouseInputThrust = (!game.touchMode && !keyInputThrust);
    bool touchInputThrust = (game.touchMode && ui.fly.clicked);
    if ((inputActionThrust && game.resumeInputCooldown == false) &&
        (keyInputThrust || mouseInputThrust || touchInputThrust))
    {
        if (mouseInputThrust)
            RotateShipToMouse(ship);

        Vector2 thrust = (Vector2){ 0, -SHIP_THRUST_SPEED };
        thrust = Vector2Rotate(thrust, ship->angle*DEG2RAD);
        thrust = Vector2Scale(thrust, game.frameTime);
        ship->velocity = Vector2Add(ship->velocity, thrust);
        ship->velocity = Vector2ClampValue(ship->velocity, 0, SHIP_MAX_SPEED);
        ship->isThrusting = true;
    }
    else if (ship->isThrusting)
        ship->isThrusting = false;

    // Shoot missile
    bool inputActionShoot = IsInputActionDown(INPUT_ACTION_SHOOT);
    bool keyInputShoot = !IsInputActionMouseDown(INPUT_ACTION_SHOOT);
    bool mouseInputShoot = (!game.touchMode && !keyInputShoot);
    bool touchInputShoot = (game.touchMode && ui.shoot.clicked);
    if ((inputActionShoot && game.resumeInputCooldown == false) &&
        (keyInputShoot || mouseInputShoot || touchInputShoot))
    {
        if (mouseInputShoot)
            RotateShipToMouse(ship);

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
        if (!rock->isExploded && CheckCollisionAsteroidShip(i, &game.ship))
        {
            ship->isExploded = true;
            ship->explosionTimer = EXPLOSION_TIME;
            rock->isExploded = true;
            SplitAsteroid(i);
            game.eliminatedCount++;
            PlaySound(game.ship.soundExplode);
        }
    }
    if (game.ship.isExploded)
    {
        game.lives--;
        if (game.lives > 0) game.messageTimer = 3.0f;
        ui.textFade = 1.0f; // for respawn message
    }
}

void DrawShip(SpaceShip *ship)
{
    // Draw explosion
    if ((ship->explosionTimer > EPSILON) && ship->isExploded)
        DrawCircleV(ship->position, ship->length, Fade(RED, 0.5f));
    if (ship->isExploded) return;

    Color shipColor = GRAY;
    Color jetColor = Fade(ORANGE, 0.5f);

    // Draw jet triangle
    if (ship->isThrusting)
        DrawTriangle(ship->jetPoints[0], ship->jetPoints[1], ship->jetPoints[2], jetColor);

    // Draw ship sprite
    Texture sprite = game.textures.ship;
    float spriteScaleX = (ship->width*1.5f)/sprite.width;
    float spriteScaleY = (ship->length*1.5f)/sprite.height;
    Rectangle spriteSrc = { 0.0f, 0.0f, (float)sprite.width, (float)sprite.height };
    Rectangle spriteDest = {
        ship->position.x, ship->position.y,
        sprite.width*spriteScaleX, sprite.height*spriteScaleY
    };
    Vector2 spriteOrigin = {
        sprite.width/2*spriteScaleX,
        sprite.height/2*spriteScaleY };
    DrawTexturePro(sprite, spriteSrc, spriteDest, spriteOrigin, ship->angle, shipColor);

    // Draw respawn shield
    if (game.ship.safeRespawnTimer > 0)
        DrawCircleV(ship->position, ship->length, Fade(SKYBLUE, 0.15f));

    // Clones at opposite side of screen
    if (ship->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 spriteClonePos = Vector2Add(ship->position, game.wrapOffsets[i]);
            Rectangle spriteCloneDest = {
                spriteClonePos.x, spriteClonePos.y,
                sprite.width*spriteScaleX,
                sprite.height*spriteScaleY
            };
            Vector2 cloneJet[3];
            cloneJet[0] = Vector2Add(ship->jetPoints[0], game.wrapOffsets[i]);
            cloneJet[1] = Vector2Add(ship->jetPoints[1], game.wrapOffsets[i]);
            cloneJet[2] = Vector2Add(ship->jetPoints[2], game.wrapOffsets[i]);

            // DrawTriangle(cloneShip[0], cloneShip[1], cloneShip[2], shipColor);
            DrawTexturePro(sprite, spriteSrc, spriteCloneDest, spriteOrigin, ship->angle, shipColor);
            if (IsInputActionDown(INPUT_ACTION_THRUST))
                DrawTriangle(cloneJet[0], cloneJet[1], cloneJet[2], jetColor);
        }
    }
}

void UpdateShipTriangles(SpaceShip *ship)
{
    // Calculate new triangle points for drawing, collision, & screen wrap
    for (unsigned int i = 0; i < 3; i++)
    {
        ship->shipPoints[i] = Vector2Rotate(game.shipTriangle[i], ship->angle*DEG2RAD);
        ship->shipPoints[i] = Vector2Add(ship->shipPoints[i], ship->position);
        ship->jetPoints[i] = Vector2Rotate(game.jetTriangle[i], (ship->angle+180)*DEG2RAD);
        ship->jetPoints[i] = Vector2Add(ship->jetPoints[i], ship->position);
    }
}

void RotateShipToMouse(SpaceShip *ship)
{
    Vector2 mousePos = GetScaledMousePosition();
    Vector2 mouseDirection = Vector2Subtract(mousePos, ship->position);
    ship->angle = (float)atan2(mouseDirection.y, mouseDirection.x)*RAD2DEG + 90;
}

void RespawnShip(SpaceShip *ship)
{
    ship->isExploded = false;
    ship->isThrusting = false;
    ship->position = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };
    ship->velocity = (Vector2){ 0, 0 };
    ship->angle = 90;
    ship->respawnTimer = SHIP_RESPAWN_TIME;
    ship->safeRespawnTimer = SHIP_SAFE_TIME;

    UpdateShipTriangles(ship);
}

void ShootMissile(SpaceShip *ship)
{
    if (ship->shotCount == MISSILE_MAX) ship->shotCount = 0;

    Missile *shot = &ship->missiles[ship->shotCount];

    shot->isExploded = false;
    shot->explosionTimer = EXPLOSION_TIME;
    shot->angle = ship->angle + 180;
    Vector2 spawnPos = { 0, ship->length*0.6f + shot->radius };
    spawnPos = Vector2Rotate(spawnPos, shot->angle*DEG2RAD);
    spawnPos = Vector2Add(spawnPos, ship->position);
    shot->position = spawnPos;
    shot->despawnTimer = MISSILE_DESPAWN_TIME;

    ship->shotCount++;
    PlaySound(ship->soundShoot);
}
