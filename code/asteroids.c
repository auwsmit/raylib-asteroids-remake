// EXPLANATION:
// All the game logic, including how/when to draw to screen
// See asteroids.h for more documentation/descriptions

#include "asteroids.h"

#include <limits.h> // for SHRT_MAX for beep sound math
#include "raymath.h" // needed for vector math
#include "rlgl.h" // needed to transform ship coords

#include "config.h"
#include "input.h"
#include "ui.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void InitGameState(void)
{
    GameState defaultState =
    {
        // Game boots to raylib logo animation
        .currentScreen = SCREEN_LOGO,

        .ship = {
            .position = {
                RENDER_WIDTH / 2,
                RENDER_HEIGHT / 2,
            },

            .width = SHIP_WIDTH,
            .length = SHIP_LENGTH,
            .rotation = 45,
        },
    };

    // Allocate memory for beep sine waves
    defaultState.beeps[BEEP_MENU] = GenBeep(300.0f, 0.03f);

    asteroidGame = defaultState;
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
    for (unsigned int i = 0; i < ARRAY_SIZE(asteroidGame.beeps); i++)
        UnloadSound(asteroidGame.beeps[i]);
}

void UpdatePongFrame(void)
{

    // Debug: reset ship
    if (IsKeyPressed(KEY_R))
        ResetShip(&asteroidGame.ship);

    if (IsInputActionPressed(INPUT_ACTION_BACK) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        ChangeUiMenu(UI_MENU_TITLE);
        return; // back to main game loop: UpdateDrawFrame()
    }

    // Input to pause
    if (IsInputActionPressed(INPUT_ACTION_PAUSE))
    {
        asteroidGame.isPaused = !asteroidGame.isPaused;
        if (asteroidGame.isPaused)
            ChangeUiMenu(UI_MENU_PAUSE);
        else
            asteroidUi.currentMenu = UI_MENU_GAMEPLAY;
        PlaySound(asteroidGame.beeps[BEEP_MENU]);
    }

    if (!asteroidGame.isPaused)
    {
        // Update ship
        UpdateShip(&asteroidGame.ship);
    }


    // Update user interface elements and logic
    UpdateUiFrame();
}

void UpdateShip(SpaceShip *ship)
{
    if (IsInputActionDown(INPUT_ACTION_LEFT))
    {
        ship->rotation -= TURN_SPEED * GetFrameTime();
    }
    if (IsInputActionDown(INPUT_ACTION_RIGHT))
    {
        ship->rotation += TURN_SPEED * GetFrameTime();
    }

    if (IsInputActionDown(INPUT_ACTION_FORWARD))
    {
        Vector2 newVelocity = (Vector2){ 0, THRUST_SPEED * GetFrameTime() };
        newVelocity = Vector2Rotate(newVelocity, ship->rotation * DEG2RAD);
        ship->velocity = Vector2Add(ship->velocity, newVelocity);
    }

    // Update ship's position for velocity
    ship->position = Vector2Subtract(ship->position, ship->velocity);

    // Slow down ship
    if (Vector2Length(ship->velocity) > 0)
    {
        float slowdownAmount = SLOWDOWN_RATE * GetFrameTime();

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
}

void DrawPongFrame(void)
{
    // Draw ship
    DrawSpaceShip(&asteroidGame.ship);

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

    // Draw rotated ship via transform matrix
    rlPushMatrix();
    {
        rlTranslatef(ship->position.x, ship->position.y, 0);
        rlRotatef(ship->rotation, 0, 0, 1);
        DrawTriangle(shipTriangle[0], shipTriangle[1], shipTriangle[2], RAYWHITE);
    }
    rlPopMatrix();

    /* Manually transform each point
    // Rotate points
    for (unsigned int i = 0; i < 3; i++)
    {
        shipTriangle[i] = Vector2Rotate(shipTriangle[i], ship->rotation * DEG2RAD);
        shipTriangle[i] = Vector2Add(shipTriangle[i], ship->position);
    }
    DrawTriangle(shipTriangle[0], shipTriangle[1], shipTriangle[2], RAYWHITE);
    */
}

void ResetShip(SpaceShip *ship)
{
    ship->position.x = RENDER_WIDTH / 2;
    ship->position.y = RENDER_HEIGHT / 2;
}
