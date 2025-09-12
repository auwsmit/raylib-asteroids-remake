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
    game = (GameState){
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

        // Define shape of ship + jet
        .shipTriangle = {
            (Vector2){ 0, -SHIP_LENGTH/2 },
            (Vector2){ -SHIP_WIDTH/2, SHIP_WIDTH/2 },
            (Vector2){  SHIP_WIDTH/2, SHIP_WIDTH/2 },
        },
        .jetTriangle = {
            (Vector2){  0, -SHIP_LENGTH*4/5 },
            (Vector2){ -SHIP_WIDTH/6, -SHIP_WIDTH/2 },
            (Vector2){  SHIP_WIDTH/6, -SHIP_WIDTH/2 },
        },

        .wrapOffsets = {
            { VIRTUAL_WIDTH, 0 },   // right
            { -VIRTUAL_WIDTH, 0 },  // left
            { 0, -VIRTUAL_HEIGHT }, // up
            { 0, VIRTUAL_HEIGHT },  // down
            { VIRTUAL_WIDTH, -VIRTUAL_HEIGHT },  // top-right
            { -VIRTUAL_WIDTH, -VIRTUAL_HEIGHT }, // top-left
            { VIRTUAL_WIDTH, VIRTUAL_HEIGHT },   // bottom-right
            { -VIRTUAL_WIDTH, VIRTUAL_HEIGHT }   // bottom-left
        },

        // .lives = 3,
    };

    // Generate random stars
    for (unsigned int i = 0; i < STAR_AMOUNT; i++)
    {
        game.stars[i].x = (float)GetRandomValue(0, VIRTUAL_WIDTH);
        game.stars[i].y = (float)GetRandomValue(0, VIRTUAL_HEIGHT);
    }

    // Missiles / Shots
    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        Missile *shot = &game.ship.missiles[i];
        shot->speed = MISSILE_SPEED;
        shot->radius = MISSILE_RADIUS;
        shot->exploded = true; // aka non-existant
    }

    // Create asteroids
    for (unsigned int i = 0; i < ASTEROID_COUNT; i++)
    {
        CreateAsteroidRandom(ASTEROID_SIZE_BIG);
    }

    // Allocate memory for beep sine waves
    game.beeps[BEEP_MENU] = GenBeep(300.0f, 0.03f);
    game.beeps[BEEP_SHOOT] = GenBeep(400.0f, 0.05f);
    game.beeps[BEEP_EXPLODE] = GenBeep(150.0f, EXPLOSION_TIME);
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

void FreeGameState(void)
{
    MemFree(game.rocks); // asteroids
    for (unsigned int i = 0; i < ARRAY_SIZE(game.beeps); i++)
        UnloadSound(game.beeps[i]); // beeps
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
    Vector2 spawnPos = { 0, ship->length/2 + shot->radius*3 };
    spawnPos = Vector2Rotate(spawnPos, shot->angle*DEG2RAD);
    spawnPos = Vector2Add(spawnPos, ship->position);
    shot->position = spawnPos;
    shot->despawnTimer = 0.8f;

    ship->shotCount++;
    PlaySound(game.beeps[BEEP_SHOOT]);
}

Color ColorBrightnessVariation(Color color)
{
    float brightness = -0.25f*GetRandomValue(0, 2); // 3 main shades
    brightness *= 0.01f*GetRandomValue(1, 100); // 10 sub-shades
    color = ColorBrightness(color, brightness);
    return color;
}

Asteroid *CreateAsteroid(SizeOfAsteroid size, Vector2 position, float angle, Color color)
{
    game.rockCount++;
    game.rocks = MemRealloc(game.rocks, game.rockCount*sizeof(Asteroid));
    Asteroid *rock = &game.rocks[game.rockCount - 1];
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

    return rock;
}

void CreateAsteroidRandom(SizeOfAsteroid size)
{
    float rockPosX = (float)GetRandomValue(0, VIRTUAL_WIDTH);
    float rockPosY = (float)GetRandomValue(0, VIRTUAL_HEIGHT);
    float angle = (float)GetRandomValue(0, 360);
    Color colorVariation = ColorBrightnessVariation(BROWN);

    Asteroid *rock = CreateAsteroid(size, (Vector2){ rockPosX, rockPosY }, angle, colorVariation);

    float safeZoneRadius = game.ship.length*3;
    rock->radius += safeZoneRadius;
    if (CheckCollisionAsteroidShip(rock, &game.ship))
    {
        rock->position.x += ((GetRandomValue(0, 1)*2) - 1)*rock->radius*2;
        rock->position.y += ((GetRandomValue(0, 1)*2) - 1)*rock->radius*2;
    }
    rock->radius -= safeZoneRadius;
}

void SplitAsteroid(Asteroid *rock)
{
    float angle = (float)GetRandomValue(0, 180);
    Vector2 spawnPosA = { 0, rock->radius/2 };
    spawnPosA = Vector2Rotate(spawnPosA, angle*DEG2RAD);
    Vector2 spawnPosB = Vector2Invert(spawnPosA);
    spawnPosA = Vector2Add(spawnPosA, rock->position);
    spawnPosB = Vector2Add(spawnPosB, rock->position);

    if (rock->size > ASTEROID_SIZE_SMALL)
    {
        CreateAsteroid(rock->size - 1, spawnPosA, angle, rock->color);
        CreateAsteroid(rock->size - 1, spawnPosB, angle + 180, rock->color);
    }
}

bool IsShipOnEdge(SpaceShip *ship)
{
    // Check ship
    for (unsigned int i = 0; i < 3; i++)
    {
        Vector2 shipPoint = ship->shipPoints[i];
        Vector2 jetPoint = ship->jetPoints[i];
        if ((shipPoint.x < 0) || (shipPoint.x > VIRTUAL_WIDTH) ||
            (shipPoint.y < 0) || (shipPoint.y > VIRTUAL_HEIGHT)||
            (jetPoint.x < 0) || (jetPoint.x > VIRTUAL_WIDTH) ||
            (jetPoint.y < 0) || (jetPoint.y > VIRTUAL_HEIGHT))
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
        Vector2 shipPoint = Vector2Rotate(game.shipTriangle[i], ship->rotation*DEG2RAD);
        shipPoint = Vector2Add(shipPoint, ship->position);
        if (CheckCollisionPointCircle(shipPoint, rock->position, rock->radius))
            return true;
    }

    if (rock->isAtScreenEdge)
    {
        for (unsigned int o = 0; o < 8; o++)
        {
            Vector2 cloneRockPos = Vector2Add(rock->position, game.wrapOffsets[o]);
            for (unsigned int i = 0; i < 3; i++)
            {
                Vector2 shipPoint = Vector2Rotate(game.shipTriangle[i], ship->rotation*DEG2RAD);
                shipPoint = Vector2Add(shipPoint, ship->position);
                if (CheckCollisionPointCircle(shipPoint, cloneRockPos, rock->radius))
                    return true;
            }
        }
    }

    return false;
}

void UpdateGameFrame(void)
{
    if (IsInputActionPressed(INPUT_ACTION_BACK))
    {
        ChangeUiMenu(UI_MENU_TITLE);
        PlaySound(game.beeps[BEEP_MENU]);
        return; // back to main game loop: UpdateDrawFrame()
    }

    // Detect win state and reset asteroids
    if (game.rockCount == game.eliminatedCount)
    {
        game.rockCount = 0;
        game.eliminatedCount = 0;
        for (unsigned int i = 0; i < ASTEROID_COUNT; i++)
        {
            CreateAsteroidRandom(ASTEROID_SIZE_BIG);
        }
    }

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
        for (unsigned int i = 0; i < game.rockCount; i++)
        {
            UpdateAsteroid(&game.rocks[i]);
        }

        // Update bullets
        for (unsigned int i = 0; i < MISSILE_MAX; i++)
        {
            UpdateMissile(&game.ship.missiles[i]);
        }

        // Update ship
        UpdateShip(&game.ship);
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
    if (ship->exploded)
    {
        game.ship.respawnTimer -= GetFrameTime();

        if (game.ship.respawnTimer <= EPSILON)
        {
            game.ship.exploded = false;
            game.ship.position = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };
            game.ship.velocity = (Vector2){ 0, 0 };
            game.ship.respawnTimer = SHIP_RESPAWN_TIME;
        }

        // do not update, ship has exploded
        return;
    }

    // Player Input
    // Rotate (mouse)
    if ((Vector2Length(GetMouseDelta()) != 0) ||
        IsMouseButtonDown(MOUSE_LEFT_BUTTON) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
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
    {
        ship->rotation -= SHIP_TURN_SPEED*GetFrameTime();
    }
    if (IsInputActionDown(INPUT_ACTION_RIGHT))
    {
        ship->rotation += SHIP_TURN_SPEED*GetFrameTime();
    }

    // Calculate thrust amount
    if (IsInputActionDown(INPUT_ACTION_FORWARD))
    {
        Vector2 thrust = (Vector2){ 0, -SHIP_THRUST_SPEED };
        thrust = Vector2Rotate(thrust, ship->rotation*DEG2RAD);
        thrust = Vector2Scale(thrust, GetFrameTime());
        ship->velocity = Vector2Add(ship->velocity, thrust);
        ship->velocity = Vector2ClampValue(ship->velocity, 0, SHIP_MAX_SPEED);
    }

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

    // Calculate new triangle points for collision & screen wrap
    for (unsigned int i = 0; i < 3; i++)
    {
        ship->shipPoints[i] = Vector2Rotate(game.shipTriangle[i], ship->rotation*DEG2RAD);
        ship->shipPoints[i] = Vector2Add(ship->shipPoints[i], ship->position);
        ship->jetPoints[i] = Vector2Rotate(game.jetTriangle[i], (ship->rotation+180)*DEG2RAD);
        ship->jetPoints[i] = Vector2Add(ship->jetPoints[i], ship->position);
    }

    ship->isAtScreenEdge = IsShipOnEdge(ship);
    WrapPastEdge(&ship->position);


    // Check collision with asteroids
    for (unsigned int i = 0; i < game.rockCount; i++)
    {
        Asteroid *rock = &game.rocks[i];
        if (!rock->exploded && CheckCollisionAsteroidShip(rock, &game.ship))
        {
            game.ship.exploded = true;
            rock->exploded = true;
            SplitAsteroid(rock);
            game.eliminatedCount++;
            PlaySound(game.beeps[BEEP_EXPLODE]);
        }
    }
}

void UpdateAsteroid(Asteroid *rock)
{
    if (rock->exploded) return;

    // Update position
    Vector2 currentVelocity = (Vector2){ 0, rock->speed*GetFrameTime() };
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
        SplitAsteroid(rock);
        PlaySound(game.beeps[BEEP_EXPLODE]);
    }
}

void UpdateMissile(Missile *shot)
{
    if (shot->exploded)
    {
        shot->explosionTimer -= GetFrameTime();
        return;
    }

    // Update position
    Vector2 currentVelocity = (Vector2){ 0, shot->speed*GetFrameTime() };
    currentVelocity = Vector2Rotate(currentVelocity, shot->angle*DEG2RAD);
    shot->position = Vector2Add(shot->position, currentVelocity);
    shot->isAtScreenEdge = IsCircleOnEdge(shot->position, shot->radius);
    WrapPastEdge(&shot->position);

    // Update despawn timer
    shot->despawnTimer -= GetFrameTime();
    if (shot->despawnTimer <= 0)
    {
        shot->exploded = true;
        shot->explosionTimer = 0.0f;
    }
}

void DrawGameFrame(void)
{
    // Draw stars
    for (unsigned int i = 0; i < STAR_AMOUNT; i++)
        DrawCircleV(game.stars[i], 1.0f, WHITE);

    // Draw rocks
    for (unsigned int i = 0; i < game.rockCount; i++)
    {
        Asteroid *rock = &game.rocks[i];
        if (!rock->exploded)
            DrawAsteroid(rock);
    }

    // Draw missiles
    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        Missile *shot = &game.ship.missiles[i];
        if (!shot->exploded)
            DrawMissile(shot);
        else if (shot->explosionTimer > EPSILON)
            DrawCircleV(shot->position, shot->radius*5, Fade(RED, 0.5f));
    }

    // Draw ship
    if (!game.ship.exploded)
        DrawShip(&game.ship);
    else if ((SHIP_RESPAWN_TIME - game.ship.respawnTimer) < EXPLOSION_TIME)
        DrawCircleV(game.ship.position, game.ship.length, Fade(RED, 0.5f));

    // Draw user interface elements
    DrawUiFrame();
}

void DrawShip(SpaceShip *ship)
{
    // Get and transform ship triangle + jet triangle
    DrawTriangle(ship->shipPoints[0], ship->shipPoints[1], ship->shipPoints[2], GRAY);
    if (IsInputActionDown(INPUT_ACTION_FORWARD))
        DrawTriangle(ship->jetPoints[0], ship->jetPoints[1], ship->jetPoints[2], ORANGE);

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

            DrawTriangle(cloneShip[0], cloneShip[1], cloneShip[2], GRAY);
            if (IsInputActionDown(INPUT_ACTION_FORWARD))
                DrawTriangle(cloneJet[0], cloneJet[1], cloneJet[2], ORANGE);
        }
    }
}

void DrawAsteroid(Asteroid *rock)
{
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

void DrawMissile(Missile *shot)
{
    if (shot->exploded) return;

    DrawCircleV((Vector2){ shot->position.x, shot->position.y }, shot->radius, RAYWHITE);

    // Clones at opposite side of screen
    if (shot->isAtScreenEdge)
    {
        for (unsigned int i = 0; i < 8; i++)
        {
            Vector2 cloneAsteroid = Vector2Add(shot->position, game.wrapOffsets[i]);
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
