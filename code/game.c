// EXPLANATION:
// All the game logic, including how/when to draw to screen
// See game.h for more documentation/descriptions

#include "game.h"

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
            {  VIRTUAL_WIDTH, 0 },  // right
            { -VIRTUAL_WIDTH, 0 },  // left
            { 0, -VIRTUAL_HEIGHT }, // up
            { 0,  VIRTUAL_HEIGHT }, // down
            {  VIRTUAL_WIDTH, -VIRTUAL_HEIGHT }, // top-right
            { -VIRTUAL_WIDTH, -VIRTUAL_HEIGHT }, // top-left
            {  VIRTUAL_WIDTH,  VIRTUAL_HEIGHT }, // bottom-right
            { -VIRTUAL_WIDTH,  VIRTUAL_HEIGHT }  // bottom-left
        },

        .level = 1,
        .lives = STARTING_LIVES,
        .rockCountStartOfLevel = ASTEROID_AMOUNT_LVL1,
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
    SetSoundVolume(beep, 0.3f);

    UnloadWave(beepSoundWave); // frees data
    return beep;
}

void FreeGameState(void)
{
    MemFree(game.rocks); // asteroids
    for (unsigned int i = 0; i < ARRAY_SIZE(game.beeps); i++)
        UnloadSound(game.beeps[i]); // beeps
}

void InitLevel(unsigned int currentLevel)
{
    game.lives = STARTING_LIVES;
    game.level = currentLevel;
    game.rockCount = 0;
    game.eliminatedCount = 0;
    if (currentLevel == 1)
    {
        game.newLevelTimer = 1.0f;
        game.rockCountStartOfLevel = ASTEROID_AMOUNT_LVL1;
        UpdateShipTriangles(&game.ship);
    }
    else
    {
        game.newLevelTimer = 0;
        game.rockCountStartOfLevel += (currentLevel % 2)? 1 : 2;
        game.ship.safeRespawnTimer = SHIP_SAFE_TIME;
        game.ship.velocity = (Vector2){ 0, 0 };
    }

    for (unsigned int i = 0; i < game.rockCountStartOfLevel; i++)
    {
        unsigned int rockIdx = CreateAsteroidRandom(ASTEROID_SIZE_BIG);
        Asteroid *newRock = &game.rocks[rockIdx];
        newRock->isAtScreenEdge = IsCircleOnEdge(newRock->position, newRock->radius);
    }

    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        game.ship.missiles[i].exploded = true;
        game.ship.missiles[i].explosionTimer = 0;
    }
    ui.textFade = 1.0f;
}

Color ColorBrightnessVariation(Color color)
{
    float brightness = -0.25f*GetRandomValue(0, 2); // 3 main shades
    brightness += 0.01f*GetRandomValue(1, 10); // sub-shades
    color = ColorBrightness(color, brightness);
    return color;
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

bool CheckCollisionAsteroidShip(unsigned int rockIdx, SpaceShip *ship)
{
    Asteroid *rock = &game.rocks[rockIdx];

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
    // Detect win state and go to next level
    if (game.rockCount == game.eliminatedCount)
        InitLevel(game.level + 1);

    // Pause
    if (IsInputActionPressed(INPUT_ACTION_PAUSE) ||
        IsInputActionPressed(INPUT_ACTION_CANCEL))
    {
        game.isPaused = !game.isPaused;
        if (game.isPaused)
            ChangeUiMenu(UI_MENU_PAUSE);
        else
            ui.currentMenu = UI_MENU_GAMEPLAY;
        PlaySound(game.beeps[BEEP_MENU]);
    }

    if (!game.isPaused && game.newLevelTimer < NEW_LEVEL_TIMER)
        game.newLevelTimer += game.frameTime;

    // Only update while unpaused
    if (!game.isPaused && game.newLevelTimer > NEW_LEVEL_TIMER)
    {
        // Game Over
        bool inputCooldownFinished = (SHIP_RESPAWN_TIME - game.ship.respawnTimer >= GAMEOVER_INPUT_COOLDOWN);
        if (game.lives == 0 && inputCooldownFinished)
        {
            if (IsInputActionPressed(INPUT_ACTION_CONFIRM) || IsGestureDetected(GESTURE_TAP))
            {
                PollInputEvents(); // Skip input this frame
                InitLevel(1);
            }
        }

        // Update rocks
        for (unsigned int i = 0; i < game.rockCount; i++)
            UpdateAsteroid(i);

        // Update bullets
        for (unsigned int i = 0; i < MISSILE_MAX; i++)
            UpdateMissile(&game.ship.missiles[i]);

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
            DrawAsteroid(i);
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
