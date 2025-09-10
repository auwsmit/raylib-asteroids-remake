// EXPLANATION:
// All the game logic, including how/when to draw to screen
// See asteroids.h for more documentation/descriptions

#include "asteroids.h"

#include <limits.h> // for SHRT_MAX for beep sound math
#include "raymath.h" // needed for vector math

#include "config.h"
#include "input.h"
#include "ui.h"

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

void InitGameState(void)
{
    GameState defaultState = {
        // Game boots to raylib logo animation
        .currentScreen = SCREEN_LOGO,

        // Center camera
        .camera.target = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 },

        .ship = {
            .position = {
                VIRTUAL_WIDTH/2,
                VIRTUAL_HEIGHT/2,
            },
            .width = SHIP_WIDTH,
            .length = SHIP_LENGTH,
            .rotation = 90.0f, // pointing right
            .respawnTimer = SHIP_RESPAWN_TIME,
        },

        .shipTri = {
            (Vector2){ 0, -SHIP_LENGTH/2 },
            (Vector2){ -SHIP_WIDTH/2, SHIP_WIDTH/2 },
            (Vector2){  SHIP_WIDTH/2, SHIP_WIDTH/2 },
        },


        .lives = 3,

    };

    // Missiles / Shots
    for (unsigned int i = 0; i < MAX_SHOTS; i++)
    {
        Missile *shot = &defaultState.ship.shots[i];
        shot->speed = SHOT_SPEED;
        shot->radius = SHOT_RADIUS;
    }

    // Random rocks
    for (unsigned int i = 0; i < MAX_ASTEROIDS; i++)
    {
        Asteroid *rock = &defaultState.rocks[i];
        float rockPosX = (float)GetRandomValue(0, VIRTUAL_WIDTH);
        float rockPosY = (float)GetRandomValue(0, VIRTUAL_HEIGHT);

        rock->position = (Vector2){ rockPosX, rockPosY };
        rock->angle = (float)GetRandomValue(0, 360);
        rock->radius = (float)GetRandomValue(ASTEROID_MIN_RADIUS,ASTEROID_MAX_RADIUS);
        rock->life = 3;

        // Speed proportional to size
        float radiusRange = ASTEROID_MAX_RADIUS - ASTEROID_MIN_RADIUS;
        float scaledSpeed;
        if (radiusRange < EPSILON)
            scaledSpeed = ASTEROID_SPEED;
        else
            scaledSpeed = ASTEROID_SPEED*(ASTEROID_MAX_RADIUS - rock->radius)/radiusRange;

        if (scaledSpeed < ASTEROID_SPEED/8) // minimum speed
            scaledSpeed = ASTEROID_SPEED/8;

        rock->speed = scaledSpeed;
    }

    // Allocate memory for beep sine waves
    defaultState.beeps[BEEP_MENU] = GenBeep(300.0f, 0.03f);

    game = defaultState;
}

Sound GenBeep(float freq, float lengthSec)
{
    unsigned int sampleRate = 44100;
    unsigned int samples = (int)(lengthSec*sampleRate);
    short *data = MemAlloc(samples*sizeof(short));

    // fade length in samples
    // (This prevents an unpleasant "pop" noise when the sound starts or stops)
    unsigned int fadeSamples = (unsigned int)(0.005f*sampleRate); // 5 ms

    // Generate wave data
    for (unsigned int i = 0; i < samples; i++)
    {
        float timeInSeconds = (float)i/sampleRate;
        float sample = sinf(2.0f*PI*freq*timeInSeconds);

        // Apply fade in/out
        float amplitude = 1.0f;
        if (i < fadeSamples)
        {
            amplitude = (float)i/fadeSamples; // fade in
        }
        else if (i > samples - fadeSamples)
        {
            amplitude = (float)(samples - i)/fadeSamples; // fade out
        }

        data[i] = (short)(sample*amplitude*SHRT_MAX*0.25f);
    }

    Wave beepSoundWave = {
        .frameCount = samples,
        .sampleRate = sampleRate,
        .sampleSize = 16,
        .channels = 1,
        .data = data
    };

    Sound beep = LoadSoundFromWave(beepSoundWave);
    UnloadWave(beepSoundWave); // frees data
    return beep;
}

void FreeBeeps(void)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(game.beeps); i++)
        UnloadSound(game.beeps[i]);
}

void ShootMissile(SpaceShip *ship)
{
    // spawn bullet
    if (ship->shotCount == MAX_SHOTS)
        ship->shotCount = 0;

    Missile *shot = &ship->shots[ship->shotCount];

    shot->exists = true;
    shot->angle = ship->rotation + 180;
    Vector2 spawnPos = { 0, ship->length/2 + shot->radius*3 };
    spawnPos = Vector2Rotate(spawnPos, shot->angle*DEG2RAD);
    spawnPos = Vector2Add(spawnPos, ship->position);
    shot->position = spawnPos;
    shot->despawnTimer = 0.8f;

    ship->shotCount++;
}

bool IsShipOnEdge(SpaceShip *ship)
{
    // Check each point
    for (unsigned int i = 0; i < 3; i++)
    {
        Vector2 shipPoint = Vector2Rotate(game.shipTri[i], ship->rotation*DEG2RAD);
        shipPoint = Vector2Add(shipPoint, ship->position);
        if ((shipPoint.x < 0) || (shipPoint.x > VIRTUAL_WIDTH) ||
            (shipPoint.y < 0) || (shipPoint.y > VIRTUAL_HEIGHT))
                return true; // At least one point is past the edge
    }

    // Ship is not past edge
    return false;
}

bool IsCircleOnEdge(Vector2 position, float radius)
{
    if ((position.x - radius < 0) ||
        (position.x + radius > VIRTUAL_WIDTH) ||
        (position.y - radius < 0) ||
        (position.y + radius > VIRTUAL_HEIGHT))
        return true; // Circular object is past the edge

    // Circular object is not past edge
    return false;
}

bool CheckCollisionAsteroidShip(Asteroid *rock, SpaceShip *ship)
{
    // Check each point
    for (unsigned int i = 0; i < 3; i++)
    {
        Vector2 shipPoint = Vector2Rotate(game.shipTri[i], ship->rotation*DEG2RAD);
        shipPoint = Vector2Add(shipPoint, ship->position);
        float distanceFromAsteroid = Vector2Length(Vector2Subtract(shipPoint, rock->position));
        bool isShipWithinAsteroid = distanceFromAsteroid < rock->radius;
        if (isShipWithinAsteroid)
            return true;
    }

    return false;
}

void UpdateGameFrame(void)
{
    // Debug: reset ship
    if (IsKeyPressed(KEY_R))
        ResetShip(&game.ship);

    if (IsInputActionPressed(INPUT_ACTION_BACK))
    {
        ChangeUiMenu(UI_MENU_TITLE);
        PlaySound(game.beeps[BEEP_MENU]);
        return; // back to main game loop: UpdateDrawFrame()
    }

    // Input to pause
    if (IsInputActionPressed(INPUT_ACTION_PAUSE))
    {
        game.isPaused = !game.isPaused;
        if (game.isPaused)
            ChangeUiMenu(UI_MENU_PAUSE);
        else
            ui.currentMenu = UI_MENU_GAMEPLAY;
        PlaySound(game.beeps[BEEP_MENU]);
    }

    if (!game.isPaused)
    {
        // Update rocks
        for (unsigned int i = 0; i < MAX_ASTEROIDS; i++)
        {
            UpdateAsteroid(&game.rocks[i]);
            if (CheckCollisionAsteroidShip(&game.rocks[i], &game.ship))
            {
                game.ship.exploded = true;
            }
        }

        // Update bullets
        for (unsigned int i = 0; i < MAX_SHOTS; i++)
        {
            UpdateMissile(&game.ship.shots[i]);
        }

        // Update ship
        if (!game.ship.exploded)
            UpdateShip(&game.ship);
        else
            game.ship.respawnTimer -= GetFrameTime();

        if (game.ship.respawnTimer <= 0)
        {
            game.ship.exploded = false;
            game.ship.position = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };
            game.ship.velocity = (Vector2){ 0, 0 };
            game.ship.respawnTimer = SHIP_RESPAWN_TIME;
        }
    }

    // Update user interface elements and logic
    UpdateUiFrame();
}

void WrapPastEdge(Vector2 *position)
{
    if (position->x < 0)            // past left edge
        position->x += VIRTUAL_WIDTH;
    if (position->x > VIRTUAL_WIDTH) // past right edge
        position->x -= VIRTUAL_WIDTH;
    if (position->y < 0)            // past top edge
        position->y += VIRTUAL_HEIGHT;
    if (position->y > VIRTUAL_HEIGHT) // past bottom edge
        position->y -= VIRTUAL_HEIGHT;
}

void UpdateShip(SpaceShip *ship)
{
    ship->isAtScreenEdge = IsShipOnEdge(ship);

    // Player Input
    // Rotate (mouse)
    if ((Vector2Length(GetMouseDelta()) != 0) ||
        IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        Vector2 mousePos = GetScaledMousePosition();
        Vector2 mouseDirection = Vector2Subtract(mousePos, ship->position);
        float distanceToMouse = Vector2Length(mouseDirection);
        if ((IsInputActionDown(INPUT_ACTION_FORWARD) && distanceToMouse > ship->length) ||
            !IsInputActionDown(INPUT_ACTION_FORWARD))
            ship->rotation = (float)atan2(mouseDirection.y, mouseDirection.x)*RAD2DEG + 90;
    }
    // Rotate (keys)
    if (IsInputActionDown(INPUT_ACTION_LEFT))
    {
        ship->rotation -= SHIP_TURN_SPEED*GetFrameTime();
    }
    if (IsInputActionDown(INPUT_ACTION_RIGHT))
    {
        ship->rotation += SHIP_TURN_SPEED*GetFrameTime();
    }

    // Thrust
    if (IsInputActionDown(INPUT_ACTION_FORWARD))
    {
        Vector2 thrust = (Vector2){ 0, -SHIP_THRUST_SPEED };
        thrust = Vector2Rotate(thrust, ship->rotation*DEG2RAD);
        thrust = Vector2Scale(thrust, GetFrameTime());
        ship->velocity = Vector2Add(ship->velocity, thrust);
        ship->velocity = Vector2ClampValue(ship->velocity, 0, SHIP_MAX_SPEED);
    }

    // Shoot missile
    if (IsInputActionPressed(INPUT_ACTION_SHOOT))
    {
        ShootMissile(ship);
    }

    // Apply friction (smooth exponential decay)
    float slowdown = expf(-SPACE_FRICTION/10*GetFrameTime());
    ship->velocity = Vector2Scale(ship->velocity, slowdown);

    // Update position
    Vector2 scaledVelocity = Vector2Scale(ship->velocity, GetFrameTime());
    ship->position = Vector2Add(ship->position, scaledVelocity);
    WrapPastEdge(&ship->position);
}

void UpdateAsteroid(Asteroid *rock)
{
    rock->isAtScreenEdge = IsCircleOnEdge(rock->position, rock->radius);

    Vector2 currentVelocity = (Vector2){ 0, rock->speed*GetFrameTime() };
    currentVelocity = Vector2Rotate(currentVelocity, rock->angle*DEG2RAD);
    rock->position = Vector2Add(rock->position, currentVelocity);
    WrapPastEdge(&rock->position);
}

void UpdateMissile(Missile *shot)
{
    shot->isAtScreenEdge = IsCircleOnEdge(shot->position, shot->radius);

    // Update position
    Vector2 currentVelocity = (Vector2){ 0, shot->speed*GetFrameTime() };
    currentVelocity = Vector2Rotate(currentVelocity, shot->angle*DEG2RAD);
    shot->position = Vector2Add(shot->position, currentVelocity);
    WrapPastEdge(&shot->position);

    // Update despawn timer
    shot->despawnTimer -= GetFrameTime();
    if (shot->despawnTimer <= 0)
        shot->exists = false;
}

void DrawGameFrame(void)
{
    // Draw rocks
    for (unsigned int i = 0; i < MAX_ASTEROIDS; i++)
    {
        DrawAsteroid(&game.rocks[i]);
    }

    // Draw missiles
    for (unsigned int i = 0; i < MAX_SHOTS; i++)
    {
        DrawMissile(&game.ship.shots[i]);
    }

    // Draw ship
    if (!game.ship.exploded)
        DrawShip(&game.ship);
    else if ((SHIP_RESPAWN_TIME - game.ship.respawnTimer) < 0.5)
        DrawCircleV(game.ship.position, game.ship.length, RED);

    // Draw user interface elements
    DrawUiFrame();
}

void DrawShip(SpaceShip *ship)
{
    // Transform ship triangle
    Vector2 shipPoints[3] = {
        game.shipTri[1],
        game.shipTri[2],
        game.shipTri[3],
    };
    for (unsigned int i = 0; i < 3; i++)
    {
        // shipPoints[i] = game.shipTri[i];
        shipPoints[i] = Vector2Rotate(game.shipTri[i], ship->rotation*DEG2RAD);
        shipPoints[i] = Vector2Add(shipPoints[i], ship->position);
    }
    DrawTriangle(shipPoints[0], shipPoints[1], shipPoints[2], RAYWHITE);

    // Clones at opposite side of screen
    if (ship->isAtScreenEdge)
    {
        Vector2 offsets[8] = {
            { VIRTUAL_WIDTH, 0 },   // right
            { -VIRTUAL_WIDTH, 0 },  // left
            { 0, -VIRTUAL_HEIGHT }, // up
            { 0, VIRTUAL_HEIGHT },  // down
            { VIRTUAL_WIDTH, -VIRTUAL_HEIGHT },  // top-right
            { -VIRTUAL_WIDTH, -VIRTUAL_HEIGHT }, // top-left
            { VIRTUAL_WIDTH, VIRTUAL_HEIGHT },   // bottom-right
            { -VIRTUAL_WIDTH, VIRTUAL_HEIGHT }   // bottom-left
        };

        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneShip[3];
            cloneShip[0] = Vector2Add(game.shipTri[0], offsets[i]);
            cloneShip[1] = Vector2Add(game.shipTri[1], offsets[i]);
            cloneShip[2] = Vector2Add(game.shipTri[2], offsets[i]);

            DrawTriangle(cloneShip[0], cloneShip[1], cloneShip[2], RAYWHITE);
        }
    }
}

void DrawAsteroid(Asteroid *rock)
{
    DrawCircleV((Vector2){ rock->position.x, rock->position.y }, rock->radius, RAYWHITE);
    Vector2 offsets[8] = {
        { VIRTUAL_WIDTH, 0 },   // right
        { -VIRTUAL_WIDTH, 0 },  // left
        { 0, -VIRTUAL_HEIGHT }, // up
        { 0, VIRTUAL_HEIGHT },  // down
        { VIRTUAL_WIDTH, -VIRTUAL_HEIGHT },  // top-right
        { -VIRTUAL_WIDTH, -VIRTUAL_HEIGHT }, // top-left
        { VIRTUAL_WIDTH, VIRTUAL_HEIGHT },   // bottom-right
        { -VIRTUAL_WIDTH, VIRTUAL_HEIGHT }   // bottom-left
    };

    // clones at opposite side of screen
    if (rock->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneAsteroid = Vector2Add(rock->position, offsets[i]);
            DrawCircleV((Vector2){ cloneAsteroid.x, cloneAsteroid.y }, rock->radius, RAYWHITE);
        }
    }
}

void DrawMissile(Missile *shot)
{
    if (!shot->exists) return;

    DrawCircleV((Vector2){ shot->position.x, shot->position.y }, shot->radius, RAYWHITE);
    Vector2 offsets[8] = {
        { VIRTUAL_WIDTH, 0 },   // right
        { -VIRTUAL_WIDTH, 0 },  // left
        { 0, -VIRTUAL_HEIGHT }, // up
        { 0, VIRTUAL_HEIGHT },  // down
        { VIRTUAL_WIDTH, -VIRTUAL_HEIGHT },  // top-right
        { -VIRTUAL_WIDTH, -VIRTUAL_HEIGHT }, // top-left
        { VIRTUAL_WIDTH, VIRTUAL_HEIGHT },   // bottom-right
        { -VIRTUAL_WIDTH, VIRTUAL_HEIGHT }   // bottom-left
    };

    // clones at opposite side of screen
    if (shot->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneAsteroid = Vector2Add(shot->position, offsets[i]);
            DrawCircleV((Vector2){ cloneAsteroid.x, cloneAsteroid.y }, shot->radius, RAYWHITE);
        }
    }
}

void ResetShip(SpaceShip *ship)
{
    ship->position.x = VIRTUAL_WIDTH/2;
    ship->position.y = VIRTUAL_HEIGHT/2;
    ship->rotation = (float)GetRandomValue(0, 360);
}
