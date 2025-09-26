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

// Initialization
// ----------------------------------------------------------------------------

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
            (Vector2){  0, -SHIP_LENGTH/2 },
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

        .currentLevel = 1,
        .lives = STARTING_LIVES,
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

    // Get sounds
    game.sounds[SOUND_MENU] =  LoadSound("assets/menu_beep.wav");
    game.sounds[SOUND_SHOOT] = LoadSound("assets/shoot.wav");
    game.sounds[SOUND_EXPLODE_SMALL] =  LoadSound("assets/explode_small.wav");
    game.sounds[SOUND_EXPLODE_MEDIUM] = LoadSound("assets/explode_medium.wav");
    game.sounds[SOUND_EXPLODE_BIG] =    LoadSound("assets/explode_big.wav");
}

void InitNewLevel(unsigned int newLevel)
{
    game.currentLevel = newLevel;
    game.rockCount = 0;
    game.eliminatedCount = 0;
    game.levelFinished = false;
    game.newLevelTimer = NEW_LEVEL_TIMER;
    if (newLevel == 1)
    {
        game.lives = STARTING_LIVES;
        game.rockCountStartOfLevel = LVL1_ASTEROID_AMOUNT;
        game.ship.position = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 };
        UpdateShipTriangles(&game.ship);
    }
    else
    {
        unsigned int rocks = LVL1_ASTEROID_AMOUNT;
        for (unsigned int i = 2; i <= game.currentLevel; i++)
        {
            rocks += (i % 2)? 1 : 2;
        }
        game.rockCountStartOfLevel = rocks;
        game.ship.safeRespawnTimer = SHIP_SAFE_TIME;
        game.ship.velocity = (Vector2){ 0, 0 };
    }

    game.rockLimit = 0;
    for (unsigned int i = 0; i < game.rockCountStartOfLevel; i++)
    {
        unsigned int rockIdx = CreateAsteroidRandom(ASTEROID_SIZE_BIG);
        Asteroid *newRock = &game.rocks[rockIdx];
        newRock->isAtScreenEdge = IsCircleOnEdge(newRock->position, newRock->radius);
    }
    game.rockLimit -= game.rockCountStartOfLevel;

    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        game.ship.missiles[i].exploded = true;
        game.ship.missiles[i].explosionTimer = 0;
    }
    ui.textFade = 1.0f;
}

void FreeGameState(void)
{
    MemFree(game.rocks); // asteroids
    for (unsigned int i = 0; i < ARRAY_SIZE(game.sounds); i++)
        UnloadSound(game.sounds[i]); // sounds
}

// Update & Draw
// ----------------------------------------------------------------------------

void UpdateGameFrame(void)
{
    // Update virtual input buttons
    if (game.touchMode)
    {
        UpdateUiVirtualInput(&ui.shoot);
        UpdateUiVirtualInput(&ui.fly);
        UpdateUiAnalogStick(&ui.stick);
    }

    // Detect win state and go to next level
    if (!game.levelFinished && (game.lives > 0) &&
        game.rockLimit == game.eliminatedCount)
    {
        game.levelFinished = true;
        game.delayTimer = 3.0f;
        ui.textFade = 1.0f;
    }
    if (game.levelFinished && (game.delayTimer < EPSILON))
        InitNewLevel(game.currentLevel + 1);

    // Pause
    if (IsInputActionPressed(INPUT_ACTION_PAUSE) ||
        IsInputActionPressed(INPUT_ACTION_CANCEL))
    {
        static float previousTextFade = 0.0f;
        game.isPaused = !game.isPaused;
        if (game.isPaused)
        {
            ChangeUiMenu(UI_MENU_PAUSE);
            previousTextFade = ui.textFade;
            ui.textFade = 1.0f;
        }
        else
        {
            ui.currentMenu = UI_MENU_GAMEPLAY;
            ui.textFade = previousTextFade;
        }
        PlaySound(game.sounds[SOUND_MENU]);
    }

    // Update timers
    if (!game.isPaused && game.delayTimer > EPSILON)
        game.delayTimer -= game.frameTime;
    if (!game.isPaused && game.newLevelTimer > EPSILON)
        game.newLevelTimer -= game.frameTime;

    bool noMessageDisplayed = (game.newLevelTimer < EPSILON);
    if (!game.isPaused && (noMessageDisplayed || game.delayTimer > EPSILON))
    {
        // Game Over
        bool inputCooldownFinished = (SHIP_RESPAWN_TIME - game.ship.respawnTimer >= GAMEOVER_INPUT_COOLDOWN);
        if (game.lives == 0 && inputCooldownFinished)
        {
            if (IsInputActionPressed(INPUT_ACTION_CONFIRM) || IsGestureDetected(GESTURE_TAP))
            {
                PollInputEvents(); // Skip input this frame
                InitNewLevel(1);
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
    // Prevent input after resuming pause
    if (IsMouseButtonUp(MOUSE_LEFT_BUTTON) && game.resumeInputCooldown)
        game.resumeInputCooldown = false;

    // Update user interface elements and logic
    UpdateUiFrame();
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
        DrawMissile(shot);
    }

    DrawShip(&game.ship);

    // Draw user interface elements
    DrawUiFrame();
}

// Collision
// ----------------------------------------------------------------------------

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
