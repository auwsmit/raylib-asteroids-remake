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

void InitGameState(ScreenState screen)
{
    static bool allocated = false;

    GameState defaults = {
        // Center camera
        .camera.target = (Vector2){ VIRTUAL_WIDTH/2, VIRTUAL_HEIGHT/2 },

        .ship = {
            .position = {
                VIRTUAL_WIDTH/2,
                VIRTUAL_HEIGHT/2,
            },
            .width = SHIP_WIDTH,
            .length = SHIP_LENGTH,
            .angle = 90.0f, // pointing right
            .respawnTimer = SHIP_RESPAWN_TIME,
        },

        // Define shape of ship + jet
        .shipTriangle = {
            (Vector2){  0, -SHIP_LENGTH/2 },
            (Vector2){ -SHIP_WIDTH/2, SHIP_WIDTH/2 },
            (Vector2){  SHIP_WIDTH/2, SHIP_WIDTH/2 },
        },
        .jetTriangle = {
            (Vector2){  0, -SHIP_LENGTH*0.8f },
            (Vector2){ -SHIP_WIDTH/6, -SHIP_WIDTH/3 },
            (Vector2){  SHIP_WIDTH/6, -SHIP_WIDTH/3 },
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

        .currentScreen = screen,
        .currentLevel = 1,
        .lives = STARTING_LIVES,
    };

    // Generate random stars
    for (unsigned int i = 0; i < STAR_AMOUNT; i++)
    {
        defaults.stars[i].x = (float)GetRandomValue(0, VIRTUAL_WIDTH);
        defaults.stars[i].y = (float)GetRandomValue(0, VIRTUAL_HEIGHT);
    }

    // Missiles / Shots
    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        Missile *shot = &defaults.ship.missiles[i];
        shot->speed = MISSILE_SPEED;
        shot->radius = MISSILE_RADIUS;
        shot->isExploded = true; // aka non-existant
    }

    // Load sound and texture assets
    if (!allocated)
    {
        defaults.sounds.menu =  LoadSound("assets/menu_beep.wav");
        defaults.sounds.explodeSmall = LoadSound("assets/explode_small.wav");
        defaults.sounds.explodeMedium = LoadSound("assets/explode_medium.wav");
        defaults.sounds.explodeBig = LoadSound("assets/explode_big.wav");
        defaults.ship.soundShoot = LoadSound("assets/shoot.wav");
        defaults.ship.soundExplode = LoadSound("assets/explode_medium.wav");

        defaults.textures.ship = LoadTexture("assets/ship.png");
        defaults.textures.asteroidA = LoadTexture("assets/asteroid_a.png");
        defaults.textures.asteroidB = LoadTexture("assets/asteroid_b.png");
        defaults.textures.asteroidC = LoadTexture("assets/asteroid_c.png");

        allocated = true;
    }
    else // reuse already allocated memory
    {
        defaults.rocks = game.rocks;
        defaults.sounds = game.sounds;

        defaults.ship.soundExplode = game.ship.soundExplode;
        defaults.ship.soundShoot = game.ship.soundShoot;
        defaults.textures = game.textures;
    }

    game = defaults;
}

void InitNewLevel(unsigned int newLevel)
{
    game.currentLevel = newLevel;
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

    // Create new asteroids
    game.rockLimit = 0;
    game.rockCount = 0;
    for (unsigned int i = 0; i < game.rockCountStartOfLevel; i++)
    {
        unsigned int rockIdx = CreateAsteroidRandom(ASTEROID_SIZE_BIG);
        Asteroid *newRock = &game.rocks[rockIdx];
        newRock->isAtScreenEdge = IsCircleOnEdge(newRock->position, newRock->radius);
    }
    game.rockLimit -= game.rockCountStartOfLevel;

    for (unsigned int i = 0; i < MISSILE_MAX; i++)
    {
        game.ship.missiles[i].isExploded = true;
        game.ship.missiles[i].explosionTimer = 0;
    }
    ui.textFade = 1.0f;
}

void FreeGameState(void)
{
    MemFree(game.rocks);
    UnloadSound(game.sounds.menu);
    UnloadSound(game.sounds.explodeSmall);
    UnloadSound(game.sounds.explodeMedium);
    UnloadSound(game.sounds.explodeBig);
    UnloadSound(game.ship.soundExplode);
    UnloadSound(game.ship.soundShoot);
    UnloadTexture(game.textures.ship);
    UnloadTexture(game.textures.asteroidA);
    UnloadTexture(game.textures.asteroidB);
    UnloadTexture(game.textures.asteroidC);
}

// Update & Draw
// ----------------------------------------------------------------------------

void UpdateGameFrame(void)
{
    // Update virtual input buttons
    if (game.touchMode)
    {
        UpdateUiTouchInput(&ui.shoot);
        UpdateUiTouchInput(&ui.fly);
        UpdateUiAnalogStick(&ui.stick);
    }

    // Detect win state and go to next level
    if (!game.levelFinished && (game.lives > 0) &&
        game.rockLimit == game.eliminatedCount)
    {
        game.levelFinished = true;
        game.messageTimer = 3.0f;
        ui.textFade = 1.0f;
    }
    if (game.levelFinished && (game.messageTimer < EPSILON))
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
        PlaySound(game.sounds.menu);
    }

    // Update timers
    if (!game.isPaused && game.messageTimer > EPSILON)
        game.messageTimer -= game.frameTime;
    if (!game.isPaused && game.newLevelTimer > EPSILON)
        game.newLevelTimer -= game.frameTime;

    bool noMessageDisplayed = (game.newLevelTimer < EPSILON);
    if (!game.isPaused && (noMessageDisplayed || game.messageTimer > EPSILON))
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
        if (!rock->isExploded)
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
        Vector2 shipPoint = Vector2Rotate(game.shipTriangle[i], ship->angle*DEG2RAD);
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
                Vector2 shipPoint = Vector2Rotate(game.shipTriangle[i], ship->angle*DEG2RAD);
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
