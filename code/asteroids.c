// EXPLANATION:
// All the game logic, including how/when to draw to screen
// See asteroids.h for more documentation/descriptions

#include "asteroids.h"

#include <limits.h> // for SHRT_MAX for beep sound math
#include "raymath.h" // needed for vector math

#include "config.h"
#include "input.h"
#include "ui.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void InitGameState(void)
{
    float shipAngle = (float)GetRandomValue(0, 360);

    GameState defaultState =
    {
        // Game boots to raylib logo animation
        .currentScreen = SCREEN_LOGO,
        .camera.target = (Vector2){ VIRTUAL_WIDTH / 2, VIRTUAL_HEIGHT / 2 },

        .ship = {
            .position = {
                VIRTUAL_WIDTH / 2,
                VIRTUAL_HEIGHT / 2,
            },
            .width = SHIP_WIDTH,
            .length = SHIP_LENGTH,
            .rotation = shipAngle,
        },

    };

    // random rocks
    for (int i = 0; i < ASTEROID_AMOUNT; i++)
    {
        float rockPosX = (float)GetRandomValue(0, VIRTUAL_WIDTH);
        float rockPosY = (float)GetRandomValue(0, VIRTUAL_HEIGHT);
        defaultState.rocks[i].position = (Vector2){ rockPosX, rockPosY };
        defaultState.rocks[i].angle = (float)GetRandomValue(0, 360);
        defaultState.rocks[i].size = (float)GetRandomValue(10,100);
        defaultState.rocks[i].speed = ASTEROID_SPEED;
    }

    // Allocate memory for beep sine waves
    defaultState.beeps[BEEP_MENU] = GenBeep(300.0f, 0.03f);

    game = defaultState;
}

Sound GenBeep(float freq, float lengthSec)
{
    int sampleRate = 44100;
    int samples = (int)(lengthSec * sampleRate);
    short *data = MemAlloc(samples * sizeof(short));

    // fade length in samples
    // (This prevents an unpleasant "pop" noise when the sound starts or stops)
    int fadeSamples = (int)(0.005f * sampleRate); // 5 ms

    // Generate wave data
    for (int i = 0; i < samples; i++)
    {
        float timeInSeconds = (float)i / sampleRate;
        float sample = sinf(2.0f * PI * freq * timeInSeconds);

        // Apply fade in/out
        float amplitude = 1.0f;
        if (i < fadeSamples) {
            amplitude = (float)i / fadeSamples; // fade in
        } else if (i > samples - fadeSamples) {
            amplitude = (float)(samples - i) / fadeSamples; // fade out
        }

        data[i] = (short)(sample * amplitude * SHRT_MAX * 0.25f);
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

bool IsShipOnEdge(SpaceShip *ship)
{
    Vector2 shipTriangle[3] =
    {
        (Vector2){ 0, -ship->length / 2 },
        (Vector2){ -ship->width / 2, ship->width / 2 },
        (Vector2){  ship->width / 2, ship->width / 2 },
    };

    // Check each point
    for (unsigned int i = 0; i < 3; i++)
    {
        shipTriangle[i] = Vector2Rotate(shipTriangle[i], ship->rotation * DEG2RAD);
        shipTriangle[i] = Vector2Add(shipTriangle[i], ship->position);
        if ((shipTriangle[i].x < 0) || (shipTriangle[i].x > VIRTUAL_WIDTH) ||
            (shipTriangle[i].y < 0) || (shipTriangle[i].y > VIRTUAL_HEIGHT))
                return true; // At least one point is past the edge
    }

    // Ship is not past edge
    return false;
}

bool IsRockOnEdge(Asteroid *rock)
{
    if ((rock->position.x - rock->size < 0) ||
        (rock->position.x + rock->size > VIRTUAL_WIDTH) ||
        (rock->position.y - rock->size < 0) ||
        (rock->position.y + rock->size > VIRTUAL_HEIGHT))
        return true; // Rock is past the edge

    // Rock is not past edge
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
        // Update ship
        UpdateShip(&game.ship);

        // Update rocks
        for (unsigned int i = 0; i < ASTEROID_AMOUNT; i++)
        {
            UpdateRock(&game.rocks[i]);
        }
    }

    // Update user interface elements and logic
    UpdateUiFrame();
}

void UpdateShip(SpaceShip *ship)
{
    ship->isAtScreenEdge = IsShipOnEdge(ship);

    // Player Input
    if ((Vector2Length(GetMouseDelta()) != 0) ||
        IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        float scale = MIN((float)GetScreenWidth()/VIRTUAL_WIDTH, (float)GetScreenHeight()/VIRTUAL_HEIGHT);
        Vector2 mousePos = { 0 };
        mousePos.x = (mouse.x - (GetScreenWidth() - (VIRTUAL_WIDTH*scale))*0.5f)/scale;
        mousePos.y = (mouse.y - (GetScreenHeight() - (VIRTUAL_HEIGHT*scale))*0.5f)/scale;
        mousePos = Vector2Clamp(mousePos, (Vector2){ 0, 0 }, (Vector2){ (float)VIRTUAL_WIDTH, (float)VIRTUAL_HEIGHT });
        Vector2 mouseDirection = Vector2Subtract(mousePos, ship->position);
        ship->rotation = atan2(mouseDirection.y, mouseDirection.x) * RAD2DEG + 90;
    }
    if (IsInputActionDown(INPUT_ACTION_LEFT))
    {
        ship->rotation -= SHIP_TURN_SPEED * GetFrameTime();
    }
    if (IsInputActionDown(INPUT_ACTION_RIGHT))
    {
        ship->rotation += SHIP_TURN_SPEED * GetFrameTime();
    }

    if (IsInputActionDown(INPUT_ACTION_FORWARD))
    {
        Vector2 newVelocity = (Vector2){ 0, SHIP_THRUST_SPEED * GetFrameTime() };
        newVelocity = Vector2Rotate(newVelocity, ship->rotation * DEG2RAD);
        ship->velocity = Vector2Add(ship->velocity, newVelocity);
        ship->velocity = Vector2ClampValue(ship->velocity, 0, SHIP_MAX_SPEED);
    }

    // Update ship's position for velocity
    ship->position = Vector2Subtract(ship->position, ship->velocity);

    // Slow down ship
    if (Vector2Length(ship->velocity) > 0)
    {
        float slowdownAmount = SPACE_FRICTION / 10 * GetFrameTime();

        if (Vector2Length(ship->velocity) > slowdownAmount)
        {
            Vector2 slowdownVector = Vector2Scale(Vector2Normalize(ship->velocity), -slowdownAmount);
            ship->velocity = Vector2Add(ship->velocity, slowdownVector);
        }
        else
            ship->velocity = (Vector2){ 0, 0 };
    }
    else
    {
        ship->velocity = (Vector2){ 0, 0 };
    }

    // Loop ship past screen edge
    if (ship->position.x < 0)            // past left edge
        ship->position.x += VIRTUAL_WIDTH;
    if (ship->position.x > VIRTUAL_WIDTH) // past right edge
        ship->position.x -= VIRTUAL_WIDTH;
    if (ship->position.y < 0)            // past top edge
        ship->position.y += VIRTUAL_HEIGHT;
    if (ship->position.y > VIRTUAL_HEIGHT) // past bottom edge
        ship->position.y -= VIRTUAL_HEIGHT;
}

void UpdateRock(Asteroid *rock)
{
    rock->isAtScreenEdge = IsRockOnEdge(rock);

    Vector2 currentVelocity = (Vector2){ 0, ASTEROID_SPEED * GetFrameTime() };
    currentVelocity = Vector2Rotate(currentVelocity, rock->angle);
    rock->position = Vector2Add(rock->position, currentVelocity);

    // Loop ship past screen edge
    if (rock->position.x < 0)             // past left edge
        rock->position.x += VIRTUAL_WIDTH;
    if (rock->position.x > VIRTUAL_WIDTH)  // past right edge
        rock->position.x -= VIRTUAL_WIDTH;
    if (rock->position.y < 0)             // past top edge
        rock->position.y += VIRTUAL_HEIGHT;
    if (rock->position.y > VIRTUAL_HEIGHT) // past bottom edge
        rock->position.y -= VIRTUAL_HEIGHT;
}

void DrawGameFrame(void)
{
    // Draw ship
    DrawSpaceShip(&game.ship);

    // Draw rocks
    for (unsigned int i = 0; i < ASTEROID_AMOUNT; i++)
    {
        DrawAsteroid(&game.rocks[i]);
    }

    // Draw user interface elements
    DrawUiFrame();
}

void DrawSpaceShip(SpaceShip *ship)
{
    Vector2 shipTriangle[3] =
    {
        (Vector2){ 0, -ship->length / 2 },
        (Vector2){ -ship->width / 2, ship->width / 2 },
        (Vector2){  ship->width / 2, ship->width / 2 },
    };

    // Transform ship triangle
    for (unsigned int i = 0; i < 3; i++)
    {
        shipTriangle[i] = Vector2Rotate(shipTriangle[i], ship->rotation * DEG2RAD);
        shipTriangle[i] = Vector2Add(shipTriangle[i], ship->position);
    }
    DrawTriangle(shipTriangle[0], shipTriangle[1], shipTriangle[2], RAYWHITE);

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

        for (int i = 0; i < 8; i++)
        {
            Vector2 cloneShip[3];
            cloneShip[0] = Vector2Add(shipTriangle[0], offsets[i]);
            cloneShip[1] = Vector2Add(shipTriangle[1], offsets[i]);
            cloneShip[2] = Vector2Add(shipTriangle[2], offsets[i]);

            DrawTriangle(cloneShip[0], cloneShip[1], cloneShip[2], RAYWHITE);
        }
    }
}

void DrawAsteroid(Asteroid *rock)
{
    DrawCircle(rock->position.x, rock->position.y, rock->size, RAYWHITE);
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

    if (rock->isAtScreenEdge)
    {
        for (int i = 0; i < 8; i++)
        {
            DrawCircle(rock->position.x + offsets[i].x,
                       rock->position.y + offsets[i].y,
                       rock->size, RAYWHITE);
        }
    }
}

void ResetShip(SpaceShip *ship)
{
    ship->position.x = VIRTUAL_WIDTH / 2;
    ship->position.y = VIRTUAL_HEIGHT / 2;
    ship->rotation = (float)GetRandomValue(0, 360);
}
