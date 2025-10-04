// EXPLANATION:
// Helps handle game input

#include "input.h"
#include "raymath.h"
#include "config.h"
#include "game.h"

// Global struct to track input key mappings
InputState gameInput = { 0 };

// Input Actions
// ----------------------------------------------------------------------------
void InitDefaultInputControls(void)
{
    InputState defaultControls = {
        // Global across program
        .keyMaps[INPUT_ACTION_FULLSCREEN] = {
            KEY_LEFT_ALT, KEY_ENTER,
            KEY_RIGHT_ALT, KEY_ENTER,
            KEY_LEFT_SHIFT, KEY_F,
            KEY_RIGHT_SHIFT, KEY_F,
            KEY_F11,
        },

        // Menu and Game
        .keyMaps[INPUT_ACTION_CONFIRM] =   { KEY_ENTER, KEY_SPACE },
        .keyMaps[INPUT_ACTION_CANCEL] =    { KEY_ESCAPE, KEY_BACKSPACE, },
        .keyMaps[INPUT_ACTION_MENU_UP] =   { KEY_W, KEY_UP },
        .keyMaps[INPUT_ACTION_MENU_DOWN] = { KEY_S, KEY_DOWN },
        .keyMaps[INPUT_ACTION_PAUSE] =     { KEY_P },

        // Player 1 controls
        .keyMaps[INPUT_ACTION_LEFT] =      { KEY_A, KEY_LEFT, },
        .keyMaps[INPUT_ACTION_RIGHT] =     { KEY_D, KEY_RIGHT, },
        .keyMaps[INPUT_ACTION_THRUST] =    { KEY_W, KEY_UP, },
        .mouseMaps[INPUT_ACTION_THRUST] =  { MOUSE_RIGHT_BUTTON },
        .keyMaps[INPUT_ACTION_SHOOT] =     { KEY_SPACE },
        .mouseMaps[INPUT_ACTION_SHOOT] =   { INPUT_MOUSE_LEFT_BUTTON },
    };

    // Set touch point button ids (bad code. only used for touch screen analog stick, needs to be rewritten)
    for (unsigned int i = 0; i < INPUT_MAX_TOUCH_POINTS; ++i)
        gameInput.touchPoints[i].currentButton = -1;

    gameInput = defaultControls;
}

bool IsInputKeyModifier(KeyboardKey key)
{
    if (key == KEY_LEFT_ALT || key == KEY_RIGHT_ALT ||
        key == KEY_LEFT_SHIFT || key == KEY_RIGHT_SHIFT ||
        key == KEY_LEFT_CONTROL || key == KEY_RIGHT_CONTROL)
        return true;
    return false;
}

bool IsInputActionPressed(InputAction action)
{
    // Check touch screen button
    if (gameInput.touchButtonPressed[action] == true)
        return true;

    // Check potential key combinations
    KeyboardKey* keys = gameInput.keyMaps[action];
    for (unsigned int i = 0; i < INPUT_MAX_MAPS && keys[i] != 0; i++)
    {
        KeyboardKey key = keys[i];

        // Check modifier plus next key (only 1 modifier for now)
        if (IsInputKeyModifier(key))
        {
            if ((i + 1 < INPUT_MAX_MAPS) && (keys[i + 1] != 0) &&
                (!IsInputKeyModifier(keys[i + 1])))
            {
                if (IsKeyDown(key) && IsKeyPressed(keys[i + 1]))
                    return true;
                i++; // Skip the next key
            }
            // Check just the modifier by itself
            else if (IsKeyPressed(key))
                return true;
        }

        // Check a single key
        else if (IsKeyPressed(key))
            return true;
    }

    // Check mouse buttons
    if (IsInputActionMousePressed(action))
        return true;

    // Check controller buttons
    // TODO

    return false;
}

bool IsInputActionMousePressed(InputAction action)
{
    MouseButton* mb = gameInput.mouseMaps[action];
    for (unsigned int i = 0; i < INPUT_MAX_MAPS && mb[i] != 0; i++)
    {
        MouseButton button = mb[i];
        if (button == 0) button = INPUT_MOUSE_NULL;
        if (button == INPUT_MOUSE_LEFT_BUTTON)
            button = MOUSE_LEFT_BUTTON;
        if (IsMouseButtonPressed(button))
            return true;
    }

    return false;
}

bool IsInputActionDown(InputAction action)
{
    if (gameInput.touchButtonPressed[action] == true)
        return true;

    // Check potential key combinations
    KeyboardKey* keys = gameInput.keyMaps[action];
    for (unsigned int i = 0; i < INPUT_MAX_MAPS && keys[i] != 0; i++)
    {
        KeyboardKey key = keys[i];

        // Check modifier plus next key (only 1 modifier for now)
        if (IsInputKeyModifier(key))
        {
            if ((i + 1 < INPUT_MAX_MAPS) && (keys[i + 1] != 0) &&
                (!IsInputKeyModifier(keys[i + 1])))
            {
                if (IsKeyDown(key) && IsKeyDown(keys[i + 1]))
                    return true;
                i++;
            }
            // Check just the modifier by itself
            else if (IsKeyDown(key))
                return true;
        }

        else if (IsKeyDown(key))
            return true;
    }

    // Check mouse buttons
    if (IsInputActionMouseDown(action))
        return true;

    // Check controller buttons
    // TODO

    return false;
}

bool IsInputActionMouseDown(InputAction action)
{
    MouseButton* mb = gameInput.mouseMaps[action];
    for (unsigned int i = 0; i < INPUT_MAX_MAPS && mb[i] != 0; i++)
    {
        MouseButton button = mb[i];
        if (button == 0) button = INPUT_MOUSE_NULL;
        if (button == INPUT_MOUSE_LEFT_BUTTON)
            button = MOUSE_LEFT_BUTTON;
        if (IsMouseButtonDown(button))
            return true;
    }

    return false;
}

// Touch / Virtual Input
// ----------------------------------------------------------------------------
void SetTouchInput(InputAction action, bool isButtonPressed)
{
    gameInput.touchButtonPressed[action] = isButtonPressed;
}

void SetTouchPointButton(int index, int buttonIdx)
{
    gameInput.touchPoints[index].currentButton = buttonIdx;
}

bool IsTouchTapped(int index)
{
    return (gameInput.touchPoints[index].pressedCurrentFrame &&
            !gameInput.touchPoints[index].pressedPreviousFrame);
}

bool IsTouchPressingButton(int index)
{
    return gameInput.touchPoints[index].currentButton != -1;
}

int CheckCollisionTouchCircle(Vector2 center, float radius)
{
    for (int i = 0; i < game.touchCount; ++i)
    {
        if (CheckCollisionPointCircle(gameInput.touchPoints[i].position, center, radius))
            return i;
    }

    return -1;
}

int CheckCollisionTouchRec(Rectangle rec)
{
    for (int i = 0; i < game.touchCount; ++i)
    {
        if (CheckCollisionPointRec(gameInput.touchPoints[i].position, rec))
            return i;
    }

    return -1;
}

// Helpers
// ----------------------------------------------------------------------------

// int CheckAvailableGamepads(void)
// {
//     // TODO
// }

int UpdateInputTouchPoints(void)
{
    int tCount = GetTouchPointCount();
    game.touchCount = tCount;

    if (tCount > INPUT_MAX_TOUCH_POINTS)
        tCount = INPUT_MAX_TOUCH_POINTS;
    for (int i = 0; i < tCount; i++)
    {
        Vector2 touchPosition = GetScreenToWorld2D(GetTouchPosition(i), game.camera);
        gameInput.touchPoints[i].position = touchPosition;
        gameInput.touchPoints[i].pressedPreviousFrame = gameInput.touchPoints[i].pressedCurrentFrame;
        gameInput.touchPoints[i].pressedCurrentFrame = (GetTouchPointId(i) < tCount);
    }

    for (int i = 0; i < INPUT_MAX_TOUCH_POINTS; i++)
        gameInput.touchPoints[i].currentButton = -1;

    return tCount;
}

#define MIN(a, b) ((a)<(b)? (a) : (b))
Vector2 GetScaledMousePosition(void)
{
    return GetScreenToWorld2D(GetMousePosition(), game.camera);
}
