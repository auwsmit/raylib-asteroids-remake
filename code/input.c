// EXPLANATION:
// Helps handle game input

#include "input.h"

// Global struct to track input key mappings
InputMappings gameInput = { 0 };

void InitDefaultInputControls(void)
{
    InputMappings defaultControls =
    {
        // Global across program
        .keyMaps[INPUT_ACTION_FULLSCREEN] =
        {
            KEY_LEFT_ALT, KEY_ENTER,
            KEY_RIGHT_ALT, KEY_ENTER,
            KEY_LEFT_SHIFT, KEY_F,
            KEY_RIGHT_SHIFT, KEY_F,
            KEY_F11,
        },

        // Menu and Game
        .keyMaps[INPUT_ACTION_CONFIRM] = { KEY_ENTER, KEY_SPACE },
        .keyMaps[INPUT_ACTION_BACK] = { KEY_ESCAPE, KEY_BACKSPACE, },
        .mouseMaps[INPUT_ACTION_BACK] = { MOUSE_RIGHT_BUTTON },
        .keyMaps[INPUT_ACTION_MENU_UP] = { KEY_W, KEY_UP },
        .keyMaps[INPUT_ACTION_MENU_DOWN] = { KEY_S, KEY_UP },
        .keyMaps[INPUT_ACTION_PAUSE] = { KEY_P },

        // Player 1 controls
        .keyMaps[INPUT_ACTION_LEFT] = { KEY_A, KEY_LEFT, },
        .keyMaps[INPUT_ACTION_RIGHT] = { KEY_D, KEY_RIGHT, },
        .keyMaps[INPUT_ACTION_FORWARD] = { KEY_W, KEY_UP, },
        .mouseMaps[INPUT_ACTION_FORWARD] = { INPUT_MOUSE_LEFT_BUTTON },
    };

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
    KeyboardKey* keys = gameInput.keyMaps[action];

    // Check potential key combinations
    for (int i = 0; i < INPUT_MAX_MAPS && keys[i] != 0; i++)
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
    MouseButton* mb = gameInput.mouseMaps[action];
    for (int i = 0; i < INPUT_MAX_MAPS && mb[i] != 0; i++)
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
    KeyboardKey* keys = gameInput.keyMaps[action];

    for (int i = 0; i < INPUT_MAX_MAPS && keys[i] != 0; i++)
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
    MouseButton* mb = gameInput.mouseMaps[action];
    for (int i = 0; i < INPUT_MAX_MAPS && mb[i] != 0; i++)
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

void HandleToggleFullscreen(void)
{
    // No fullscreen input for web because it's buggy
    // For now just use emscripten's fullscreen button
#if !defined(PLATFORM_WEB)
    // Input for fullscreen
    if (IsInputActionPressed(INPUT_ACTION_FULLSCREEN))
    {
        // Borderless Windowed is generally nicer to use on desktop
        ToggleBorderlessWindowed();
        PollInputEvents(); // Skip to the next frame's input
    }
#endif
}
